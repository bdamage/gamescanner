Game Scanner
------------


Description

The open source project Game Scanner is a lightweight, non-bloated and low-memory footprint 
software for easy browsing of your favorite or any supported internet game servers without 
starting the game itself.

With just an instant of an click and you're away to join the selected server. It also 
supports features such as RCON*, in-game minimizing* (for fast and easy switching between 
desktop and the game) and keeping track of your buddies to see which server they currently 
is playing on. Integrated mIRC status messaging when joing servers.


Currently supported games:

Wolfenstein: Enemy Territory D
Enemy Territory - Quake Wars
Return to Castle of Wolfenstein
QuakeWorld 
Quake 2
Quake 3
Quake 4
Open Arena
Call of Duty
Call of Duty 2
Call of Duty 4 - Modern Warfare
Warsow
Urban Terror
Counter-Strike
Counter-Strike: Condition Zero
Counter-Strike: Source
Half-Life 2 mods
- Half-life 2
- Team Fortress: Source
- Day of Defeat: Source
- Battle Grounds 2
- PVKII
- Dark Messiah Might and Magic
- Zombie Panic
- Hidden: Source 
etc....


Compiling
---------

Visual Studio 2008 has been used but should compile fine on VS2005 also.


After a compilation you need the files inside the redist folder.
You can copy them to Release or/and the Debug folders.


Directories global vars
-----------------------

EXE_PATH - 

Contains executable path. NO data should be written into this path, as this fucks up Vista instead 
use SAVE_PATH.

Currently EXE, DLL and screenshot preview is read from this dir.




SAVE_PATH - 

Following command is used to retrieve the path: SHGetFolderPath(NULL,CSIDL_APPDATA,NULL,SHGFP_TYPE_CURRENT,SAVE_PATH);

Files that should be stored into this path is:
log.htm (logging inforamtion)
config.xml
treeview.xml
*.servers     - saved server informaiton
ipcountry.dat - Country database (created with the gamescanner.exe file itself, it is a "simplifed" structure based file instead of an CSV file)



net.jpg??



Typical user save path are following 

for Vista:
C:\Users\UserName\AppData\Roaming\GameScanner

for Windows XP:
C:\Documents and Settings\USERNAME\Application Data\GameScanner


