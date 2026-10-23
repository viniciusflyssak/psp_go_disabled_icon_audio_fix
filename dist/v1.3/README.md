# GoAudioFix 1.3 — test build

## English

Adds a brief `VOLUME 12/30` indicator on VOL+/VOL- and a proposed recovery for
sound lost after PSP Go Pause Game. Compiled with PSPDEV; host logic tests pass.
**Console validation is pending.** The tested 1.2 binary remains one folder up.

Copy this file to `ef0:/seplugins/go_audiofix.prx`, replacing the installed PRX.
Restart VSH, launch a game normally, create a **new Pause Game session**, then
resume it. An old snapshot may retain older plugin code/state.
Check sound after resume, the volume indicator and that the startup icon stays
quiet. If it fails, include the `resume recovery` log lines in your report.

SHA-256: `f23f8cbfdde671fcc1029fce9e835fc53d7318e389458a46eb13aad7b331bdf9`

## Português

Adiciona o indicador breve `VOLUME 12/30` em VOL+/VOL- e uma possível recuperação
do som perdido após Pause Game no PSP Go. Compilou no PSPDEV e passou nos testes
da lógica. **A validação no console está pendente.** A 1.2 validada está na pasta
anterior.

Copie este arquivo para `ef0:/seplugins/go_audiofix.prx`, substituindo o instalado.
Reinicie o VSH, abra um jogo normalmente, crie uma **nova pausa com Pause Game**
e retome. Uma pausa antiga pode manter o código/estado da versão anterior.
Confira som após retomar, indicador de volume e ausência de ícone persistente
no início. Se falhar, envie as linhas `resume recovery` do log.

O SHA-256 acima identifica este binário de teste.
