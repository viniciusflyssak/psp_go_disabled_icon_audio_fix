/*
 * PSP Go AudioFix - kernel PRX para PRO-C / PSPDEV atual.
 *
 * Contorna a politica de roteamento: NAO altera eletricamente HP_DET e NAO
 * promete corrigir o icone do XMB. Sem offsets de firmware, hooks em codigo,
 * buffers PCM, novos canais de audio ou alteracao da memoria dos jogos.
 *
 * O volume e o controle mestre do impose (0..30), nao multiplicacao de PCM.
 * sceAudioSetVolume nao e uma API global publica do PSPSDK. Alterar os oito
 * canais com sceAudioChangeChannelVolume nao cobriria SRC/Output2 e jogos
 * poderiam sobrescrever o valor a cada buffer.
 */
#include <pspkernel.h>
#include <pspctrl.h>
#include <pspimpose_driver.h>
#include <pspsysmem_kernel.h>
#include <pspinit.h>
#include "config.h"
#include "osd.h"

PSP_MODULE_INFO("GoAudioFix", PSP_MODULE_KERNEL, 1, 2);

extern unsigned int sctrlHENFindFunction(char *module, char *library,
                                       unsigned int nid);
typedef int (*SetRouteFn)(int);
typedef int (*GetRouteFn)(void);
typedef int (*GetParamFn)(int);
typedef int (*SetParamFn)(int, int);

static SetRouteFn set_route;
static GetRouteFn get_route;
static GetParamFn get_param;
static SetParamFn set_param;
static volatile int running;
static SceUID worker_id = -1;
static int target_volume = AUDIOFIX_BOOT_VOLUME;
static int old_volume, old_mute, old_route;
static int state_saved;
static int last_log_result = -1;
static const char *selected_log_path;
static unsigned int mute_writes, volume_writes, route_writes;

/* Log apenas na inicializacao, falha e parada. Sem stdio/newlib ou escrita
 * por frame. Os resultados das APIs sao hexadecimais de 32 bits. */
static void log_result(const char *label, int result)
{
    char line[112];
    static const char hex[] = "0123456789ABCDEF";
    unsigned int value = (unsigned int)result;
    unsigned int n = 0, i;
    SceUID fd;
    int written;
    unsigned int path_index;
    static const char *const paths[] = {
        AUDIOFIX_LOG_PATH, "ef0:/go_audiofix.log",
        "ms0:/seplugins/go_audiofix.log", "ms0:/go_audiofix.log"
    };
    while (*label && n < sizeof(line) - 12)
        line[n++] = *label++;
    line[n++] = ' ';
    line[n++] = '0';
    line[n++] = 'x';
    for (i = 0; i < 8; ++i)
        line[n++] = hex[(value >> (28 - i * 4)) & 15];
    line[n++] = '\n';
    fd = -1;
    if (selected_log_path)
        fd = sceIoOpen(selected_log_path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND,
                       0777);
    if (fd < 0) {
        for (path_index = 0; path_index < sizeof(paths) / sizeof(paths[0]); ++path_index) {
            fd = sceIoOpen(paths[path_index], PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND,
                           0777);
            if (fd >= 0) {
                selected_log_path = paths[path_index];
                break;
            }
        }
    }
    last_log_result = fd < 0 ? fd : 0;
    if (fd < 0)
        return;
    i = 0;
    while (i < n) {
        written = sceIoWrite(fd, line + i, n - i);
        if (written <= 0) {
            last_log_result = written < 0 ? written : -1;
            break;
        }
        i += (unsigned int)written;
    }
    written = sceIoClose(fd);
    if (written < 0 && !last_log_result)
        last_log_result = written;
}

/* O PRO traduz os NIDs antigos do SDK. Alguns resolvedores nao incluem a
 * biblioteca AudioRouting; nesse caso tentamos tambem o NID observado em
 * impose de firmware recente. Ausencia de export => ponteiro NULL, nunca
 * uma chamada a um endereco presumido. Ver fontes no README. */
static int resolve_audio(void)
{
    unsigned int address;
    if (!set_route) {
        address = sctrlHENFindFunction("sceImpose_Driver",
                                      "sceAudioRouting_driver", 0x36FD8AA9);
        if (!address)
            address = sctrlHENFindFunction("sceImpose_Driver",
                                          "sceAudioRouting_driver", 0x18B6F449);
        set_route = (SetRouteFn)address;
    }
    if (!get_route) {
        address = sctrlHENFindFunction("sceImpose_Driver",
                                      "sceAudioRouting_driver", 0x39240E7D);
        if (!address)
            address = sctrlHENFindFunction("sceImpose_Driver",
                                          "sceAudioRouting_driver", 0x931ABEF5);
        get_route = (GetRouteFn)address;
    }
    if (!get_param)
        get_param = (GetParamFn)sctrlHENFindFunction("sceImpose_Driver",
                                          "sceImpose_driver", 0x531C9778);
    if (!set_param)
        set_param = (SetParamFn)sctrlHENFindFunction("sceImpose_Driver",
                                          "sceImpose_driver", 0x810FB7FB);
    return set_route && get_param && set_param;
}

/* Read first: even writing an identical impose value can refresh the native
 * volume popup. Polling must not generate UI events while state is stable.
 * A route getter is optional: without it, keep the startup route rather than
 * repeatedly writing an unverifiable value. */
static int enforce_audio(void)
{
    int result, first_error = 0;
    result = get_param(PSP_IMPOSE_MUTE);
    if (result > 0) {
        ++mute_writes;
        result = set_param(PSP_IMPOSE_MUTE, 0);
    }
    if (result < 0)
        first_error = result;
    result = get_param(PSP_IMPOSE_MAIN_VOLUME);
    if (result >= 0 && result != target_volume) {
        ++volume_writes;
        result = set_param(PSP_IMPOSE_MAIN_VOLUME, target_volume);
    }
    if (result < 0 && !first_error)
        first_error = result;
    if (get_route) {
        result = get_route();
        if (result >= 0 && result != AUDIOFIX_ROUTE_MODE) {
            ++route_writes;
            result = set_route(AUDIOFIX_ROUTE_MODE);
        }
        if (result < 0 && !first_error)
            first_error = result;
    }
    return first_error;
}

/* VOL+ e VOL- juntos e HOLD nao alteram volume. Nao suprimimos os botoes
 * de volume; o impose tambem pode processa-los. Peek evita bloquear ou
 * consumir o buffer que o jogo pode ler com sceCtrlReadBufferPositive. */
static int direction(unsigned int buttons)
{
    int up, down;
    if (buttons & PSP_CTRL_HOLD)
        return 0;
    up = (buttons & PSP_CTRL_VOLUP) != 0;
    down = (buttons & PSP_CTRL_VOLDOWN) != 0;
    return up == down ? 0 : (up ? 1 : -1);
}

static int due(unsigned int now, unsigned int deadline)
{
    /* Comparacao modular: tolera o wrap do relogio baixo (~71 minutos). */
    return (int)(now - deadline) >= 0;
}

static int audio_thread(SceSize args, void *argp)
{
    SceCtrlData pad;
    unsigned int now, deadline, next_enforce, next_repeat = 0, next_log_retry;
    unsigned int summary_at;
    int dir, previous_dir = 0, result, error_logged = 0, summary_logged = 0;
    (void)args;
    (void)argp;

    log_result("worker iniciado; firmware", (int)sceKernelDevkitVersion());
    log_result("context: 100=VSH 200=GAME", sceKernelApplicationType());
    deadline = sceKernelGetSystemTimeLow() + AUDIOFIX_READY_TIMEOUT_US;
    next_log_retry = sceKernelGetSystemTimeLow() + 1000000u;
    /* A thread do module_start retorna imediatamente; esperamos os modulos
     * e o estado do impose ficarem prontos fora do caminho de boot. */
    while (running) {
        audiofix_osd_tick(last_log_result);
        now = sceKernelGetSystemTimeLow();
        if (last_log_result < 0 && due(now, next_log_retry)) {
            log_result("worker retry; firmware", (int)sceKernelDevkitVersion());
            next_log_retry = now + 1000000u;
        }
        if (resolve_audio()) {
            old_volume = get_param(PSP_IMPOSE_MAIN_VOLUME);
            old_mute = get_param(PSP_IMPOSE_MUTE);
            if (old_volume >= 0 && old_volume <= 30 &&
                old_mute >= 0 && old_mute <= 1)
                break;
        }
        if (due(sceKernelGetSystemTimeLow(), deadline)) {
            log_result("impose/export indisponivel; plugin inativo", -1);
            log_result("export set_route", (int)set_route);
            log_result("export get_param", (int)get_param);
            log_result("export set_param", (int)set_param);
            log_result("ultimo volume lido", old_volume);
            log_result("ultimo mute lido", old_mute);
            audiofix_osd_show("ERROR AUDIO NOT READY", -1);
            goto diagnostic_only;
        }
        sceKernelDelayThread(100000);
    }
    if (!running)
        return 0;

    /* SetMode retorna o modo ANTERIOR, nao necessariamente zero. */
    old_route = set_route(AUDIOFIX_ROUTE_MODE);
    log_result("set_route: modo anterior ou erro", old_route);
    if (old_route < 0) {
        audiofix_osd_show("ERROR ROUTING REJECTED", old_route);
        goto diagnostic_only;
    }
    state_saved = 1;
    ++route_writes;
    log_result("route getter available", get_route != NULL);
    next_enforce = sceKernelGetSystemTimeLow();
    summary_at = next_enforce + 10000000u;

    while (running) {
        now = sceKernelGetSystemTimeLow();
        dir = 0;
        if (sceCtrlPeekBufferPositive(&pad, 1) > 0)
            dir = direction(pad.Buttons);
        if (dir && (dir != previous_dir || due(now, next_repeat))) {
            target_volume += dir;
            if (target_volume < 0)
                target_volume = 0;
            if (target_volume > AUDIOFIX_MAX_VOLUME)
                target_volume = AUDIOFIX_MAX_VOLUME;
            next_repeat = now + (dir != previous_dir ?
                         AUDIOFIX_FIRST_REPEAT_US : AUDIOFIX_NEXT_REPEAT_US);
            next_enforce = now;
        }
        previous_dir = dir;
        if (due(now, next_enforce)) {
            result = enforce_audio();
            /* Uma unica linha de sucesso/falha inicial e a primeira falha
             * posterior: evita escrever continuamente em ef0:. */
            if (!error_logged) {
                log_result("apply: zero=APIs aceitas, nao prova som", result);
                audiofix_osd_show(result < 0 ? "AUDIO API ERROR" : "ACTIVE",
                                  result);
                if (result < 0)
                    error_logged = 1;
                else
                    error_logged = 2;
            } else if (result < 0 && error_logged == 2) {
                log_result("apply: primeira falha posterior", result);
                audiofix_osd_show("ERROR AUDIO API", result);
                error_logged = 1;
            }
            next_enforce = now + AUDIOFIX_ENFORCE_US;
        }
        if (last_log_result < 0 && due(now, next_log_retry)) {
            log_result("log retry; firmware", (int)sceKernelDevkitVersion());
            next_log_retry = now + 1000000u;
        }
        if (!summary_logged && due(now, summary_at)) {
            log_result("10s: mute write attempts", (int)mute_writes);
            log_result("10s: volume write attempts", (int)volume_writes);
            log_result("10s: route write attempts", (int)route_writes);
            log_result("10s: volume readback", get_param(PSP_IMPOSE_MAIN_VOLUME));
            log_result("10s: mute readback", get_param(PSP_IMPOSE_MUTE));
            if (get_route)
                log_result("10s: route readback", get_route());
            summary_logged = 1;
        }
        /* While the short toast is visible, VBlank is the pacing source;
         * adding another 20 ms sleep would skip frames and cause flicker. */
        if (!audiofix_osd_tick(last_log_result))
            sceKernelDelayThread(AUDIOFIX_POLL_US);
    }
    return 0;

diagnostic_only:
    /* Keep an error visible even if audio init failed; do not call bad APIs.
     * Retry logging at most once per second if storage was not ready. */
    while (running) {
        now = sceKernelGetSystemTimeLow();
        if (last_log_result < 0 && due(now, next_log_retry)) {
            log_result("audio inativo; firmware", (int)sceKernelDevkitVersion());
            next_log_retry = now + 1000000u;
        }
        if (!audiofix_osd_tick(last_log_result))
            sceKernelDelayThread(AUDIOFIX_POLL_US);
    }
    return 0;
}

int module_start(SceSize args, void *argp)
{
    int result;
    (void)args;
    (void)argp;
    log_result("GoAudioFix 1.2 module_start", 0);
    log_result("model", sceKernelGetModel());
    /* 05g/PSP Go = 4 na API. ef0: e armazenamento, nao particao RAM. */
    if (sceKernelGetModel() != 4) {
        log_result("modelo nao suportado", -1);
        return -1;
    }
    running = 1;
    worker_id = sceKernelCreateThread("GoAudioFixWorker", audio_thread,
                                     0x18, 0x2000, 0, NULL);
    if (worker_id < 0) {
        log_result("sceKernelCreateThread erro", worker_id);
        running = 0;
        return worker_id;
    }
    result = sceKernelStartThread(worker_id, 0, NULL);
    if (result < 0) {
        log_result("sceKernelStartThread erro", result);
        running = 0;
        sceKernelDeleteThread(worker_id);
        worker_id = -1;
        return result;
    }
    return 0;
}

int module_stop(SceSize args, void *argp)
{
    SceUInt timeout = 2000000;
    int result;
    (void)args;
    (void)argp;
    running = 0;
    if (worker_id >= 0) {
        result = sceKernelWaitThreadEnd(worker_id, &timeout);
        if (result < 0)
            return result; /* Recusa descarregar codigo ainda em execucao. */
        result = sceKernelDeleteThread(worker_id);
        if (result < 0)
            return result;
        worker_id = -1;
    }
    if (state_saved) {
        set_param(PSP_IMPOSE_MAIN_VOLUME, old_volume);
        set_param(PSP_IMPOSE_MUTE, old_mute);
        set_route(old_route);
        state_saved = 0;
    }
    return 0;
}
