# Changelog

## English

### 1.3 — user-confirmed on PSP Go, PRO-C 6.61

- The console owner confirmed the requested behavior, including Pause Game
  audio recovery and the volume-level indicator. The tested binary is unchanged.

- Show `VOLUME 00/30` through `VOLUME 30/30` after VOL+/VOL- adjustments;
  hide the notification about 1.5 seconds after the last adjustment.
- Detect resume notifications and gaps in the worker/RTC timeline, including
  a restored Pause Game session when cached audio settings still look correct.
- Refresh routing through auto -> speakers at most three times per recovery
  window, with a delay and spaced attempts. Normal polling stays conditional.
- Keep audio/display/file operations outside system-event callbacks, and
  unregister the handler before unloading the module.
- Add host tests for bounded/coalesced retries, timer wrap, restored clocks
  and volume labels. These do not substitute for a PSP hardware test.
- Make 1.3 the main download; keep versioned copies in `dist/v1.3/` and
  preserve 1.2 in `dist/v1.2/`.

### 1.2 — user-confirmed on PSP Go, firmware 6.61

- Confirmed on the user's console: XMB and game audio, volume buttons, correct
  small notification and no persistent startup volume icon.

- Read mute, master volume and route before writing. Stable polling no longer
  repeatedly requests identical settings, addressing a suspected cause of the
  native volume popup continually reappearing.
- Replace the large three-line startup overlay with a small 6x9-pixel-font
  notification at the bottom-right for about 2.5 seconds, paced by VBlank.
- Add an option to disable the notification, VSH/GAME context logging and a
  one-time state/write-counter summary after 10 seconds.
- Keep the routing mode and VOL+/VOL- method reported working in version 1.1.

### 1.1 — user-reported success on one PSP Go, firmware 6.61

- Speaker audio and volume buttons worked in XMB and a freshly launched game.
- Fixed the PRO import descriptor; added early/fallback logging and an overlay.
- Reported issues: large/flickering overlay and continually visible volume icon.

## Português

### 1.3 — confirmada pelo usuário em PSP Go, PRO-C 6.61

- O proprietário confirmou o comportamento desejado, incluindo recuperação de
  áudio após Pause Game e indicador de volume. O binário testado não mudou.

- Exibe de `VOLUME 00/30` a `VOLUME 30/30` após ajustar VOL+/VOL-;
  oculta o aviso cerca de 1,5 segundo depois do último ajuste.
- Detecta notificações de retomada e saltos no tempo da thread/RTC, incluindo
  retorno de Pause Game quando os valores de áudio em memória parecem corretos.
- Atualiza a rota por automático -> alto-falantes em até três tentativas
  espaçadas por recuperação. A verificação normal continua sem escritas inúteis.
- Mantém áudio, tela e arquivos fora do callback de eventos e remove o handler
  antes de descarregar o módulo.
- Adiciona testes de tentativas limitadas, eventos agrupados, virada do relógio,
  restauração de tempo e texto de volume. Não substituem o teste no PSP.
- Torna a 1.3 o download principal, mantém a cópia em `dist/v1.3/`
  e preserva a 1.2 em `dist/v1.2/`.

### 1.2 — confirmada pelo usuário em PSP Go, firmware 6.61

- Confirmado no console do usuário: áudio no XMB e em jogos, botões de volume,
  aviso pequeno correto e ausência do ícone de volume persistente no início.

- Consulta mute, volume mestre e rota antes de escrever. A verificação de estado
  estável não repete ajustes idênticos, tratando uma possível causa do ícone de
  volume reaparecer continuamente.
- Substitui as três linhas grandes por aviso pequeno, com fonte de 6x9 pixels,
  no canto inferior direito por cerca de 2,5 segundos, acompanhado pelo VBlank.
- Adiciona opção de desativar o aviso, contexto VSH/GAME no log e um resumo único
  de estado e contadores de escrita após 10 segundos.
- Mantém o modo de rota e o controle por VOL+/VOL- que funcionaram na versão 1.1.

### 1.1 — sucesso relatado em um PSP Go, firmware 6.61

- Áudio nos alto-falantes e botões de volume funcionaram no XMB e em jogo iniciado
  novamente.
- Corrigiu o descritor de importação do PRO; adicionou log antecipado, caminhos
  alternativos de gravação e mensagem na tela.
- Problemas relatados: mensagem grande/com cintilação e ícone de volume constante.
