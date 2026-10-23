# Changelog

## English

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
