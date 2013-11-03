zSILENCER
=========

Compiling on Linux
------------------
SDL2 and SDL2_mixer development libraries will need to be installed.  
Run aclocal, autoconf and automake and then:  
./configure  
make  

Compiling on Windows
--------------------
SDL2 and SDL2_mixer development libraries will have to be installed into the Visual Studio include and lib directories  
Open zSILENCER.sln Visual Studio Project  
Compile project using Visual Studio  

Supported platforms
-------------------
The game is supported on Windows, Mac OS X, and Linux.  Other platforms, such as Android, work but are not fully tested.  

Game data
---------
The game data can be found in the Windows release archive at http://zsilencer.com/zsilencer_windows.zip  
Otherwise, the game data can be copied from the official game installation.  
Files needed:  
PALETTE.BIN  
sound.bin  
XBASE15A.SIL  
BIN_TIL.DAT  
BIN_SPR.DAT  
CLOSER.MP3  
(directories and all files within them:)  
bin_spr  
bin_til  
level  

Running game
------------
SDL2 and SDL2_mixer are required to run the game.
