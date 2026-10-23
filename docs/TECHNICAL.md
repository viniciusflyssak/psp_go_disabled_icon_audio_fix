# Build and troubleshooting / Compilação e diagnóstico

[README](../README.md)

## English

### 1.4 Bluetooth candidate

Source/root build: 1.4. Main download: hardware-tested 1.3 (Bluetooth switching is a known issue). The 1.4 candidate is in `dist/v1.4/`.

Local analysis of the official PSP Go 6.61 firmware identified read-only impose parameter `0x1000`: it returns the same Boolean selection used by native Bluetooth routing and volume handling. This interpretation still needs confirmation on hardware. The plugin queries it through the existing resolved impose API, with no module-memory reads, patches, extra Bluetooth imports or pairing changes. Only firmware `0x06060110` enables this undocumented query; other versions retain the previous speaker workaround and log that the query is unsupported.

Status 1 selects automatic route 0; status 0 selects configured speaker route 2. Errors or unexpected values preserve the last selection (initially local). Selection is checked at startup, each audio enforcement and each resume attempt. Resume never forces a speaker transition when the selection is Bluetooth. Native master-volume handling and the volume overlay are unchanged. State transitions are logged, without per-frame storage writes.

Hardware checks: connect/disconnect audio in XMB and a fresh game; start with a headset already connected; resume a newly created Pause Game session with and without Bluetooth; keep only the radio or a controller connected. Verify audio, volume buttons and overlay, and that speakers return on headset disconnection. SDK compilation and host state/timing tests cannot establish audible output.

### 1.3 resume recovery and volume indicator

The console owner confirmed version 1.3, including Pause Game recovery and the volume indicator, on PSP Go with PRO-C firmware 6.61. Other setups remain unvalidated.
The module registers a system-event handler that only sets a flag. Audio calls,
logging and OSD work stay in the worker. A separate RTC check every 250 ms can
spot a restored snapshot whose plugin/system-time state was saved earlier.
Long worker gaps also request recovery. RTC adjustments or long scheduling
stalls can trigger the same limited recovery sequence.

Each request schedules up to three auto -> speaker routing refreshes, starting
after 350 ms and spaced 750 ms apart. Related requests share the active window;
there is no catch-up burst after a missed deadline. A fresh route transition
can refresh physical output even if the cached route getter still returns 2.
The log records the trigger (event ID, 1 for worker gap, 2 for RTC gap) and each
attempt result. A successful API result is not proof of audible sound.

`logic.c` / `logic.h` contain the platform-independent timing/formatting logic.
Host tests can be run with a native C compiler from the project root:

```sh
cc -std=c99 -Wall -Wextra -Werror -I. logic.c tests/logic_test.c -o /tmp/audiofix_logic_test
/tmp/audiofix_logic_test
```

`AUDIOFIX_VOLUME_OSD_US` controls the volume indicator lifetime (1.5 seconds).
It displays the target level after accepted audio calls, including at 0/30.
The archived 1.3 source behavior described below predates the 1.4 Bluetooth candidate. The main download at `dist/go_audiofix.prx` and versioned copy at
`dist/v1.3/go_audiofix.prx` are the tested 1.3 binary; 1.2 is archived in `dist/v1.2/`.
Create a fresh Pause Game snapshot after updating; an older snapshot may restore
an older plugin/state and cannot be assumed to include the new code.

### Status and behavior

Version **1.2 was tested successfully on a PSP Go running firmware 6.61**, as reported by the console owner: speaker audio and VOL+/VOL- work in XMB and games; the smaller notification displays correctly; the native volume icon no longer remains visible at startup. It also builds with PSPDEV using `-Wall -Wextra -Werror`. This confirms operation on the tested device, not every firmware, game or hardware revision.

- PSP Go only (05g, model value 4), in VSH and GAME.
- Requests speaker routing, clears system mute, and sets master volume.
- Uses **VOL+ / VOL- only**, without triggers or D-pad combinations.
- Starts at **12/30**, clamps to 0..30, and repeats after 400 ms then every 120 ms when held. Both volume buttons together or HOLD make no adjustment through the listener.
- Uses nonblocking controller Peek every 20 ms; checks audio state every 250 ms and after adjustments. Mute, volume and routing are written only when readback differs from the desired state. The short notification temporarily uses VBlank for pacing.
- Uses the system master-volume parameter, not per-channel gain, a PCM multiplier or amplification above the original maximum.
- No user RAM expansion, reserved audio channels, PCM buffers or flash0 writes. `ef0:` is internal storage, not a RAM partition.

This is a software workaround, not a physical repair. It does not change the disabled-volume icon itself. Firmware can override audio settings between updates, causing cuts. If firmware also handles the volume buttons, duplicate steps are possible. The listener requires `sceCtrl` to expose the physical button states. POPS, Bluetooth and video-out are not validated.

### On-screen notification (1.2)

A single small line appears at the **bottom-right for about 2.5 seconds**:

```text
AUDIOFIX 1.2 ACTIVE
```

The font is 6x9 pixels per character, replacing the large three-line display from 1.1. Drawing is paced by VBlank while visible to reduce flicker. `ACTIVE` means the initial API checks succeeded; it does not independently confirm audible speaker output. Errors show a short message and hexadecimal API code. Detailed diagnostics stay in the log; `LOG ERROR` indicates a log failure.

The notification draws into a validated existing 480x272 LCD VRAM framebuffer without switching modes, clearing the screen or hooking the renderer. Rendering can still overwrite it, and a static homebrew screen can retain pixels until its next redraw. Set `AUDIOFIX_OSD_ENABLED` to **0** in `config.h` and rebuild to disable it completely. A plugin rejected before `module_start` cannot display its own message.

### Build and install

Start Docker Desktop in Linux-container mode. In PowerShell, from the project folder:

```powershell
cd psp_go_disabled_icon_audio_fix
docker run --rm --mount "type=bind,source=$($PWD.Path),target=/src" -w /src ghcr.io/pspdev/pspdev:latest sh -c "make clean && make"
```

With PSPDEV installed in Linux/WSL, run `make` in the project directory. The output to install is **`go_audiofix.prx`**. ELF and object files stay on the PC. Pin the container digest for reproducible builds.

1. Replace `ef0:/seplugins/go_audiofix.prx` with the new build. Its module version and notification identify version 1.2.
2. Disable `boost.prx` and `volcon.prx` during testing.
3. Append the following line to the **existing** `ef0:/seplugins/vsh.txt` and `ef0:/seplugins/game.txt`, preserving their other entries:

   ```text
   ef0:/seplugins/go_audiofix.prx 1
   ```

4. Verify that the plugin is enabled in Recovery and PRO-C is active. Restart VSH; test speakers and VOL+/VOL- in XMB, a homebrew and a freshly launched game. Do not continue an existing or saved game session when checking a plugin update.
5. To undo, disable its entries and restart VSH. Preserve USB/Recovery access to edit the entries if this experimental build freezes.

Example lists are in `seplugins/`. Keep sources and build files on the PC. The repository name can differ from the runtime filename, which remains `go_audiofix.prx`.

### Logs and troubleshooting

Logging starts in `module_start`, before model checks and worker creation. It records model, firmware, thread errors, routing and audio results. These paths are tried in order; the first writable path is reused:

```text
ef0:/seplugins/go_audiofix.log
ef0:/go_audiofix.log
ms0:/seplugins/go_audiofix.log
ms0:/go_audiofix.log
```

The worker retries failed logging at most once per second. A one-time summary after 10 seconds records mute/volume/route write attempts and current state. Context 0x100 identifies VSH; 0x200 identifies GAME. In an idle session with stable state, mute/volume counters should not keep increasing; route writes should normally total one startup request. Button presses and firmware corrections can increase them. Logs are appended across launches and can be removed manually.

If **neither overlay nor log appears**, verify the copied PRX, exact filename, `ef0:` path, enabled VSH entry and active CFW. Send the exact CFW version, plugin-list entries and any available log. Missing overlay alone is not proof of a load failure because drawing is best-effort.

Starting with version 1.1, the plugin uses the original PRO import descriptor `0x00090000`. Version 1.0 incorrectly used `0x00010011`, requesting minimum library version 0.17, which could prevent loading. The PRO resolver translates the old impose NIDs. Routing additionally tries setter `0x18B6F449` and getter `0x931ABEF5` if their historical NIDs are unavailable. If no route getter is found, the plugin logs this and keeps startup-only routing rather than blindly rewriting it.

Routing **mode 2** (force speakers) is based on reports for recent firmware and worked in the reported XMB and fresh game test; other devices and resume behavior still need validation. Historical SDK documentation lists only 0/1. If routing is rejected or accepted without sound, change `AUDIOFIX_ROUTE_MODE` to **1** (speakers + headphones), rebuild and retest. A different electrical fault may require a hardware repair.

### Files and hardware checks

`main.c`: audio, buttons, logging and lifecycle. `osd.c` / `osd.h`: overlay. `config.h`: timing and limits. `pro_imports.S`: PRO resolver import. `exports.exp`: PRX lifecycle exports. `Makefile`: build.

Check the new notification/log, whether the native volume icon stops constantly reappearing, actual speaker audio, volume bounds and hold-repeat; test XMB, homebrew, a game, suspend/resume and return to XMB. Check for freezes, cuts and duplicate volume steps. Volume returns to 12 when a new plugin instance loads. An accepted API result does not validate the speaker output.

## Português

### Versão 1.4 para teste de Bluetooth

O código e o PRX da raiz estão na versão 1.4; o download principal continua na 1.3, com limitação conhecida na troca para Bluetooth. O novo arquivo fica em `dist/v1.4/`.

A consulta de áudio Bluetooth é habilitada apenas no firmware 6.61 analisado. A interpretação do parâmetro ainda precisa de confirmação no console. Ela permite liberar o roteamento automático quando o sistema seleciona Bluetooth e voltar aos alto-falantes ao desconectar. Também é consultada na inicialização e na recuperação do Pause Game. Não altera o pareamento nem lê diretamente a memória dos módulos. Erros de consulta preservam a última seleção; outros firmwares mantêm o comportamento anterior.

Teste conexão e desconexão no XMB e em jogos, inicialização com fone conectado, uma nova sessão de Pause Game e Bluetooth ligado sem fone. Confira som, botões e indicador de volume. A compilação e os testes de lógica não substituem essa verificação.

### Recuperação de retomada e indicador de volume da 1.3

O proprietário confirmou a versão 1.3, incluindo recuperação após Pause Game e indicador de volume, em PSP Go com PRO-C e firmware 6.61. Outras configurações ainda não foram validadas.
O handler de eventos do sistema apenas sinaliza a thread. Áudio, log e tela são
tratados fora do callback. Uma leitura RTC a cada 250 ms ajuda a detectar uma
sessão restaurada com estado antigo do plugin/relógio do sistema. Intervalos
longos sem executar a thread também solicitam recuperação. Ajustes do RTC ou
atrasos longos de agendamento podem acionar a mesma sequência limitada.

Cada solicitação agenda até três atualizações automático -> alto-falantes,
começando após 350 ms e espaçadas em 750 ms. Eventos relacionados compartilham
a janela ativa, sem rajada para compensar prazos perdidos. A troca de rota pode
atualizar a saída física mesmo quando o getter ainda informa 2. O log registra
a origem (ID do evento, 1 para salto na thread, 2 para salto RTC) e o retorno de
cada tentativa. Sucesso da API não comprova som audível.

`logic.c` / `logic.h` contêm a lógica de tempo e formatação. Com um compilador C
nativo, execute os comandos de teste da seção inglesa na raiz do projeto.
`AUDIOFIX_VOLUME_OSD_US` controla a duração do indicador (1,5 segundo). Ele mostra
o nível desejado após as APIs aceitarem o ajuste, inclusive em 0/30.

Os fontes e o PRX recompilado na raiz são a versão 1.3. O download principal em `dist/go_audiofix.prx` e a cópia em `dist/v1.3/go_audiofix.prx`
são o binário 1.3 testado; a versão 1.2 está arquivada em `dist/v1.2/`.
Crie uma nova pausa após atualizar; uma sessão antiga pode restaurar o código
ou estado anterior, sem incluir a mudança.

### Estado e funcionamento

A versão **1.2 foi testada com sucesso em um PSP Go com firmware 6.61**, conforme relato do proprietário: áudio nos alto-falantes e VOL+/VOL- funcionam no XMB e em jogos; o aviso menor aparece corretamente; o ícone nativo de volume não fica mais visível no início. A versão também compila no PSPDEV com `-Wall -Wextra -Werror`. O resultado confirma o funcionamento no aparelho testado, sem representar todos os firmwares, jogos ou revisões de hardware.

- Somente PSP Go (05g, modelo 4), nos contextos VSH e GAME.
- Solicita a rota dos alto-falantes, limpa mute e ajusta o volume mestre.
- Controle somente por **VOL+ / VOL-**, sem gatilho ou direcional.
- Volume inicial **12/30**, limitado a 0..30. Ao segurar, repete após 400 ms e depois a cada 120 ms. Os dois botões juntos ou HOLD não ajustam pelo listener.
- Consulta os botões com Peek a cada 20 ms; verifica o estado do áudio a cada 250 ms e após ajustes. Só escreve mute, volume e rota quando a leitura difere do estado desejado. Durante o aviso curto, usa VBlank para controlar o ritmo.
- Usa o volume mestre do sistema, sem ganho por canal, multiplicação PCM ou boost acima do limite original.
- Sem expansão de RAM do usuário, canais reservados, buffers PCM ou escrita em flash0. `ef0:` é armazenamento interno, não uma partição RAM.

É uma tentativa de contorno por software, não um reparo físico. O plugin não altera diretamente o ícone desabilitado. O firmware pode reverter as configurações entre ciclos, causando cortes. Se o firmware também processar VOL+/VOL-, podem ocorrer passos duplicados. O listener depende de o `sceCtrl` entregar os estados dos botões. POPS, Bluetooth e video-out não foram validados.

### Aviso na tela (1.2)

Uma única linha pequena aparece no **canto inferior direito por cerca de 2,5 segundos**:

```text
AUDIOFIX 1.2 ACTIVE
```

A fonte tem 6x9 pixels por caractere, substituindo as três linhas grandes da 1.1. O desenho acompanha o VBlank enquanto está visível para reduzir a cintilação. `ACTIVE` confirma o sucesso das verificações iniciais das APIs; não comprova por si só que há som. Erros mostram mensagem curta e código hexadecimal. Os detalhes ficam no log; `LOG ERROR` indica falha de gravação.

O texto é desenhado em um framebuffer LCD 480x272 em VRAM validado, sem trocar o modo de tela, limpar a imagem ou interceptar o renderer. O jogo/XMB ainda pode sobrescrevê-lo, e telas estáticas podem reter os pixels até a próxima redesenhada. Para desligar o aviso por completo, defina `AUDIOFIX_OSD_ENABLED` como **0** em `config.h` e recompile. Se o loader rejeitar o PRX antes de `module_start`, ele não pode mostrar sua própria mensagem.

### Compilar e instalar

Abra o Docker Desktop em modo Linux containers e execute no PowerShell:

```powershell
cd psp_go_disabled_icon_audio_fix
docker run --rm --mount "type=bind,source=$($PWD.Path),target=/src" -w /src ghcr.io/pspdev/pspdev:latest sh -c "make clean && make"
```

Se o PSPDEV estiver instalado no Linux/WSL, rode `make` na pasta do projeto. O arquivo para instalar é **`go_audiofix.prx`**; ELF e objetos ficam no computador. Para reproduzir exatamente o build, fixe o digest da imagem Docker.

1. Copie o novo PRX para `ef0:/seplugins/go_audiofix.prx`, substituindo o anterior. Os metadados e o aviso identificam a versão 1.2.
2. Desative `boost.prx` e `volcon.prx` durante os testes.
3. Acrescente a linha abaixo aos arquivos **existentes** `ef0:/seplugins/vsh.txt` e `ef0:/seplugins/game.txt`, preservando as outras entradas:

   ```text
   ef0:/seplugins/go_audiofix.prx 1
   ```

4. Confirme no Recovery que o plugin está habilitado e que o PRO-C está ativo. Reinicie o VSH e teste alto-falantes e VOL+/VOL- no XMB, homebrew e jogo iniciado novamente. Não continue uma sessão existente ou salva ao verificar uma atualização do plugin.
5. Para desfazer, desative as entradas e reinicie o VSH. Preserve acesso USB/Recovery para editar os arquivos caso esta versão experimental trave.

Os exemplos de listas ficam em `seplugins/`. Não copie fontes e arquivos de build para o console. O nome do repositório pode ser diferente do nome do PRX, que continua sendo `go_audiofix.prx`.

### Log e diagnóstico

O log começa em `module_start`, antes de verificar o modelo ou criar a thread. Registra modelo, firmware, erros de thread, rota e APIs de áudio. Procure nesta ordem:

```text
ef0:/seplugins/go_audiofix.log
ef0:/go_audiofix.log
ms0:/seplugins/go_audiofix.log
ms0:/go_audiofix.log
```

O primeiro caminho gravável é reutilizado. Se falhar, a thread tenta novamente no máximo uma vez por segundo. Após 10 segundos, grava um único resumo com tentativas de escrita de mute, volume e rota e os valores lidos. O contexto 0x100 indica VSH; 0x200 indica GAME. Sem apertar botões e com estado estável, os contadores de mute/volume não devem continuar aumentando; a rota normalmente soma uma chamada inicial. Ajustes de botão e correções de estado podem aumentar esses valores. O arquivo acumula linhas entre carregamentos e pode ser apagado manualmente.

Se **não aparecer mensagem nem log**, confira o PRX copiado, nome exato, caminho `ef0:`, entrada VSH habilitada e CFW ativo. Informe a versão exata do CFW, as entradas das listas e qualquer log disponível. A ausência apenas da mensagem não comprova falha de carregamento, porque o desenho depende do framebuffer.

O descritor do import foi corrigido de `0x00010011` para `0x00090000`, igual ao fonte original do PRO. O anterior exigia versão mínima 0.17 da biblioteca, podendo impedir o carregamento. O resolvedor PRO traduz os NIDs antigos do impose. Para a rota, também são tentados o setter `0x18B6F449` e o getter `0x931ABEF5` quando os NIDs históricos não existem. Sem getter de rota, registra essa condição e mantém o roteamento apenas na inicialização, sem regravações cegas.

O **modo 2**, que solicita forçar alto-falantes, baseia-se em relatos de firmware recente e funcionou no teste informado de XMB e jogo recém-iniciado; outros aparelhos e suspensão/retorno ainda precisam ser validados. A documentação histórica enumera somente 0/1. Se a rota for recusada ou continuar sem som, experimente `AUDIOFIX_ROUTE_MODE = 1` (alto-falantes + fone) em `config.h`, recompile e teste novamente. Outro defeito elétrico pode exigir reparo físico.

### Arquivos e testes no console

`main.c`: áudio, botões, log e ciclo de vida. `osd.c` / `osd.h`: mensagem visual. `config.h`: limites e tempos. `pro_imports.S`: import do resolvedor PRO. `exports.exp`: exports do PRX. `Makefile`: compilação.

Confira o novo aviso/log, se o ícone nativo deixa de aparecer continuamente, som real nos alto-falantes, limites e repetição de volume; teste XMB, homebrew, jogo, suspensão/retorno e volta ao XMB. Observe travas, cortes e passos duplicados. O volume volta a 12 a cada nova instância. Retorno de sucesso das APIs não comprova a saída sonora real.

## Technical references / Referências técnicas

- [PSPSDK impose parameters](https://github.com/pspdev/pspsdk/blob/master/src/kernel/pspimpose_driver.h)
- [PSPSDK display framebuffer API](https://github.com/pspdev/pspsdk/blob/master/src/display/pspdisplay.h)
- [PSPSDK import descriptor structure](https://github.com/pspdev/pspsdk/blob/master/src/kernel/psploadcore.h)
- [Original PRO imports](https://github.com/MrColdbird/procfw/blob/master/SystemControl/libs/libpspsystemctrl_kernel/SystemCtrlForKernel.S)
- [PRO resolver implementation](https://github.com/MrColdbird/procfw/blob/master/SystemControl/systemctrl.c)
- [PSPSDK routing discussion, issue 388](https://github.com/pspdev/pspsdk/issues/388)
- [PSPDEV installation on Windows](https://pspdev.github.io/installation/windows.html)
- [PSPDEV container](https://github.com/pspdev/pspdev/pkgs/container/pspdev)
