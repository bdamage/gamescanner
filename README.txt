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


TreeView XML layout:


game="2"     - Game index (points to ET, ETQW, Q3...COD etc)
value="0"    - Can be any kind of value for ie. Filter servers with lower Ping than 100
icon="74"    - Icon index 
expanded="0" - The node/item has childs member, this will remember if the user has expanded the node on the next start of ETSV.
type="2"     - If type is 28 then it is Global Ping item
state="0"    - State of the item if it is Checked or Unchecked.

action="1"  - If action is 1 then redraw list based on current game="?" index.  
	      If action is 8 then a Global filter Edit (ie Ping, Min , Max players).
	      If action is 10 then it is a country item.


<Item name="Quake 3: Arena" value="0" icon="74" expanded="0" type="2" state="0" game="2" action="1">


XML tree logic is following when a user interaction has occured:
1. user clicks on an item function TreeView_GetSelectionV3 () is called
2. item is looked up and checks which ACTION value it has 
Typical ACTION could be:
 (a) redraw a serverlist and exit function (TreeView_GetSelectionV3 ).
 (b) filter modification for a game, see step 3
 (c) global filter setting changes, see step 4

3. depending on ACTION value then next function is called
4. and here is TYPE looked up depending of type value a filter change is being processed.

