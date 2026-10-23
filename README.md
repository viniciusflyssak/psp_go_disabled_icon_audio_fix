# PSP Go Audio Fix

[English](#english) · [Português](#português)

## English

A fix for PSP Go consoles that stay muted or show the disabled-volume icon because of a faulty headphone jack.

The plugin restores audio through the built-in speakers and lets you use **VOL+ and VOL−** normally, both in the **XMB menu and in games**. It runs automatically, with no button combinations needed.

**Tested on:** PSP Go with PRO-C, firmware **6.61**, version **1.4**. Other setups have not been confirmed.

It also restores sound after **Pause Game** and shows the current level, such as **VOLUME 12/30**, when you adjust it.

**Bluetooth headphones are supported:** audio switches to the headset when connected and returns to the speakers when disconnected on firmware 6.61.

### Installation

1. [Download go_audiofix.prx — v1.4](https://github.com/viniciusflyssak/psp_go_disabled_icon_audio_fix/raw/refs/heads/main/dist/go_audiofix.prx).
2. Copy it to `ef0:/seplugins/` on your PSP Go.
3. Add this line to **both** `vsh.txt` and `game.txt` inside that folder. Create the files if needed and keep any existing entries:

   ```text
   ef0:/seplugins/go_audiofix.prx 1
   ```

4. Disable **Audio Boost** (`boost.prx`) and **Volume Control** (`volcon.prx`) if you use them, to avoid conflicts.
5. Restart VSH. To test in a game, launch it again rather than continuing an existing session.

### Using it

A small **AUDIOFIX 1.4 ACTIVE** message appears briefly when the plugin starts. Adjust the volume with **VOL+ / VOL−**; hold either button to keep adjusting. Volume starts at **12 out of 30** each time the plugin loads. After updating, create a new Pause Game session to use the resume fix.

To disable the plugin, change the final `1` in its entries to `0` and restart VSH.

This is a software workaround; the physical headphone-jack fault remains.

### Help and development

If it does not work, [open an issue](https://github.com/viniciusflyssak/psp_go_disabled_icon_audio_fix/issues) with your firmware version, where the problem occurs, and `ef0:/seplugins/go_audiofix.log` if available.

[Build and troubleshooting guide](docs/TECHNICAL.md#english) · [Changelog](CHANGELOG.md#english)

## Português

Uma solução para PSP Go que fica sem som ou mostra o ícone de volume desabilitado por causa de um defeito no conector de fone.

O plugin recupera o áudio dos alto-falantes e permite usar **VOL+ e VOL−** normalmente, tanto no **menu XMB quanto em jogos**. Funciona automaticamente, sem combinações de botões.

**Testado em:** PSP Go com PRO-C, firmware **6.61**, versão **1.4**. Outras configurações ainda não foram confirmadas.

Também recupera o som após **Pause Game** e mostra o nível atual, como **VOLUME 12/30**, ao ajustar o volume.

**Compatível com fones Bluetooth:** o áudio passa para o fone ao conectar e volta aos alto-falantes ao desconectar no firmware 6.61.

### Instalação

1. [Baixe go_audiofix.prx — v1.4](https://github.com/viniciusflyssak/psp_go_disabled_icon_audio_fix/raw/refs/heads/main/dist/go_audiofix.prx).
2. Copie para `ef0:/seplugins/` no seu PSP Go.
3. Adicione esta linha aos arquivos **vsh.txt e game.txt** dentro dessa pasta. Crie os arquivos caso não existam e preserve as outras entradas:

   ```text
   ef0:/seplugins/go_audiofix.prx 1
   ```

4. Desative o **Audio Boost** (`boost.prx`) e o **Volume Control** (`volcon.prx`), caso os utilize, para evitar conflitos.
5. Reinicie o VSH. Para testar em um jogo, abra-o novamente em vez de continuar uma sessão existente.

### Como usar

Um pequeno aviso **AUDIOFIX 1.4 ACTIVE** aparece por alguns instantes quando o plugin inicia. Ajuste o volume com **VOL+ / VOL−**; mantenha o botão pressionado para continuar ajustando. O volume começa em **12 de 30** a cada carregamento do plugin. Após atualizar, crie uma nova pausa com Pause Game para usar a correção da retomada.

Para desativar, troque o `1` final das entradas por `0` e reinicie o VSH.

A solução é por software; o defeito físico no conector de fone permanece.

### Ajuda e desenvolvimento

Se não funcionar, [abra uma issue](https://github.com/viniciusflyssak/psp_go_disabled_icon_audio_fix/issues) informando a versão do firmware, onde o problema acontece e o conteúdo de `ef0:/seplugins/go_audiofix.log`, se disponível.

[Guia de compilação e diagnóstico](docs/TECHNICAL.md#português) · [Histórico de versões](CHANGELOG.md#português)
