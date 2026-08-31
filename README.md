# MY Retro CHess♟️(🇬🇧 English Version)
Are you tired of ads popping up and overloaded interfaces on every famous chess online game? Fortunately for you, fpwp2n2ks4-stack and I have created the perfect fully local retro-looking chess game to compete against your favourite local chess engine.
Actually we yield to modernity but the only extravagant features are an incorporated terminal (kinda retro) and a screenshot button (kinda useful for sharing with you friends when you checkmate StockFish level 20), we sincerely hope it does not disturb your purist soul 🤓!


## Credits
This game is co-authored with fpwp2n2ks4-stack, all pushes have been done using my account but he massively participated in the development, so far I made the interface that he refactored and he added the possibility to bind it to every uci-speaking chess engine. If you are really curious, in every commit of his work, the commit-message will mention his pseudo.
Also we are proud to say that this code is fully human written as well as the textures 😎, so no AI company can claim to own this code in the future.


## Installation
For any OS the installation starts with a "git clone" of the project, or with a "Download ZIP" if you are too lazy to open your terminal, the next step depends on your distribution:
 - Linux : TODO
 - MacOS : TODO
 - Windows : To run this code on windows you need three things, a C++ compiler, the SFML library and a chess engine, personally I used MSys2 UCRT64 shell to compile C++, you can directly install SFML inside of it using `pacman -S mingw-w64-ucrt-x86_64-sfml`, lastly I installed StockFish from the official website [https://stockfishchess.org/](https://stockfishchess.org/).


## Running the game
Last step is to call the binary or the .exe with the appropriate parameters :
./main <engine_path> [white|black] [skill_level] s=<engine_minutes> h=<human_minutes>
- <engine_path> : path to the chess engine bin or .exe
- white|black : human (pieces) color (default = white)
- skill_level : 0-20 (default = 20, 0 is the easiest)
- s=<min> : engine play time in minutes (default = 20)
- h=<min> : human play time in minutes (default = s)


Call example on windows : ./main.exe D:/stockfish/stockfish-windows-x86-64-avx2.exe "white" 0 5 5

## Changelog 📜
 - 8/31/2026 : First realease 🥳, the game is playable with a very few bugs. The interface displays the chessboard, the captured pieces, the clocks and three buttons. It works perfectly with stockfish.


## To do list 🎯
### Bug fix
 - Stop timer when checkmate
### New features
 - Show reachable squares when a piece is selected


# MY Retro CHess♟️(🇫🇷 English Version)
Toi aussi tu n'en peux plus des pubs pop up et des interfaces surchargées sur les jeux d'échecs en ligne? Heureusement pour toi, avec fpwp2n2ks4-stack nous avons créé le jeu d'échecs parfait pour jouer en local avec une interface rétro, dans laquelle tu peux jouer contre ton moteur d'échec local préféré.
A vrai dire nous avons succombé à la modernité et avons ajouté deux fonctionnalités superflues, un terminal intégré à l'interface (superflu mais vachement rétro) et un bouton capture d'écran (super pratique pour montrer à tes potes que tu as mis en échec StockFish level 20), on espère sincèrement que cela ne va pas froisser ton âme de puriste 🤓!


## Crédits
Ce jeu est co-écrit avec fpwp2n2ks4-stack, tous les git "pousse" on été faits avec mon compte mais il a massivement participé au développement, pour l'instant j'ai codé l'interface SFML, qu'il a restructuré et il a ajouté la possibilité de la lier avec n'importe quel moteur d'échec qui parle l'Interface Universelle d'Échec (UCI, oui on est fâché avec l'anglais).
Nous sommes également fiers d'avoir fait le code et les textures à la main 😎, de sorte à ce qu'aucune entreprise d'IA ne puisse se proclamer détentrice de ce code dans le futur.


## Installation
Pour n'importe quel système d'exploitation l'installation commence avec un "git clone" du projet, ou avec un "Download ZIP" si tu es trop fainéant pour ouvrir un terminal, l'étape suivante dépends de ta distribution:
 - Linux : TODO
 - MacOS : TODO
 - Windows : Pour faire tourner ce code sur windaube tu as besoin de trois choses, un compilateur C++, la librairie SFML et un moteur d'échec, personnellement j'ai utilisé MSys2 UCRT 64 shell pour compiler le C++, et tu peux directement y installer SFML grâce à `pacman -S mingw-w64-ucrt-x86_64-sfml`, enfin j'ai installé StockFish depuis le site officiel [https://stockfishchess.org/](https://stockfishchess.org/).


## Lancement du jeu
La dernière étape consiste à appeler le fichier binaire ou .exe avec les paramètres appropriés :
./main <engine_path> [white|black] [skill_level] s=<engine_minutes> h=<human_minutes>
- <engine_path> : chemin du moteur d'échec bin ou .exe
- white|black : couleur (des pièces) du joueur (défaut = white)
- skill_level : 0-20 (défaut = 20, 0 est le plus facile)
- s=<min> : cadence du moteur en minutes (défaut = 20)
- h=<min> : cadence du joueur en minutes (défaut = s)


Exemple d'appel du programme sur windows : ./main.exe D:/stockfish/stockfish-windows-x86-64-avx2.exe "white" 0 5 5

## Journal des modifications 📜
- 31/8/2026 : Première version 🥳, le jeu est jouable avec très peu de bugs. L'interface affiche l'échiquier, les pièces capturées, les horloges et trois boutons. L'interface fonctionne parfaitement avec StockFish.

# Liste des choses à faire 🎯
### Correction de bugs
 - Arrêter le temps quand il y a échec et mat.
### Nouvelles fonctionnalités
 - Montrer les cases accessibles quand une pièce est sélectionnée