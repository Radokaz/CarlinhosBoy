# CarlinhosBoy

## Modo de uso:

Para abrir uma rom basta escolher o arquivo diretamente pelo botão "Abrir ROM" ou selecionar a pasta em que o programa irá buscar arquivos com a extensão ".gb" ou ".gbc" para exibir na lista à direita do menu principal (essa lista da raylib é meio bugada, então é comum não mostrar todos os arquivos).
Para salvar um jogo que tenha suporte a bateria do cartucho, certifique-se de fechar o jogo pelo menu. Também há saves automáticos feitos periodicamente quando o jogo escreve na SRAM.
Também há suporte para save states, apenas evite compartilhar o mesmo slot para o mesmo jogo em modo cgb e dmg_only pois podem ocorrer bugs estranhos.

## Controles:

Por padrão:

- UP: `W`
- LEFT: `A`
- DOWN: `S`
- RIGHt: `D`
- B: `N`
- A: `M`
- START: `O`
- SELECT: `P`

Algumas hotkeys:

- T: Alterna entre uma paleta verde que emula as cores da tela LCD do dmg original e a paleta preto e branco (DMG Only).
- C: Abre o menu de pausa durante a reprodução do jogo.
- ESC: Mesma coisa que o comando acima, mas não pode ser remapeada no menu.
- F: Ativa o Fast Forward (2x).
- F11: Alterna entre modo janela e fullscreen.
- F1: Save State no slot selecionado.
- F2: Load State no slot selecionado.

Os comandos listados acima (com exceção de ESC) podem ser alterados tanto no menu principal quanto no menu de pausa.
Apertar 'ALT + (1, 2, 3 ou 4)' desativa o respectivo canal de áudio (assim como no SameBoy, útil pra debugging).
Também é possível conectar um controle e mapear seus inputs pelo menu (com o nome dos botões do controle de Xbox apenas, não tive saco pra diferenciar por marca).

## Modo CBG

No menu principal é possivel apertar em um botão para alternar entre o modo CGB com compatibilidade DMG e o modo DMG puro. A diferença é que jogos DMG Only podem ser reproduzidos com cor e algumas melhorias do Game Boy Color, como a ausencia de alguns bugs do Game Boy original, caso sejam abertos no modo CBG. Jogos de Game Boy Color já rodam em CGB Mode por regra.

