#ifndef GO_AUDIOFIX_CONFIG_H
#define GO_AUDIOFIX_CONFIG_H

/* 2 = force speakers, observado em firmwares recentes. Experimental no 05g.
 * 1 = speakers + headphones, alternativa para testar apos recompilar.
 * A documentacao publica antiga do SDK enumera somente 0/1. */
#define AUDIOFIX_ROUTE_MODE       2
#define AUDIOFIX_BOOT_VOLUME      12  /* escala nativa: 0..30, sem boost */
#define AUDIOFIX_MAX_VOLUME       30
#define AUDIOFIX_POLL_US          20000u
#define AUDIOFIX_ENFORCE_US       250000u
#define AUDIOFIX_FIRST_REPEAT_US  400000u
#define AUDIOFIX_NEXT_REPEAT_US   120000u
#define AUDIOFIX_READY_TIMEOUT_US 30000000u

#define AUDIOFIX_LOG_PATH "ef0:/seplugins/go_audiofix.log"
#define AUDIOFIX_OSD_ENABLED 1
#define AUDIOFIX_OSD_US 2500000u  /* breve aviso de 2,5 s */

#if AUDIOFIX_ROUTE_MODE != 1 && AUDIOFIX_ROUTE_MODE != 2
#error "Use route 1 ou 2"
#endif
#if AUDIOFIX_BOOT_VOLUME < 0 || AUDIOFIX_BOOT_VOLUME > AUDIOFIX_MAX_VOLUME
#error "Volume inicial invalido"
#endif
#if AUDIOFIX_MAX_VOLUME < 1 || AUDIOFIX_MAX_VOLUME > 30
#error "Volume maximo deve estar entre 1 e 30"
#endif
#endif
