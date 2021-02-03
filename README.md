## Step-By-Step tutorial on how to load c0wadlina:
1.   Get a USB Flash Drive
2.   Download `EFILOADER` and extract it to the root of your USB, so the USB looks like this:
```
USB
├── EFI
│   └── Boot
│       └── bootx64.efi
└── memory.efi
```
3.   Disconnect any other storage drives connected to your computer and boot your PC from the USB
4.   A 'UEFI Shell' should appear on your screen. If you've disconnected all other USB storage drives, your USB should be labeled `fs0`. Navigate to it by typing `fs0:`
5.   Type `ls` to list all files on your USB, you should see a folder titled `EFI` and a file titled `memory.efi`, which you want to load by typing `load memory.efi`
6.   You should now see a confirmation text, this means everything went flawlessly.
![EFI Loaded](https://i.imgur.com/V2KueKk.jpg)
8.   Type `exit` to boot into Windows 10 and **disconnect the USB now!!!**
----------
9.   Start Cold War and hop into a private Die Maschine lobby. (You need to be the host)
10.   Once you're fully loaded into the game and you're free to move, pause the game and start `XOctl.exe`
11. A license confirmation check should pop up, click `OK`
![License Check](https://i.imgur.com/67WwIe7.png)
12.  Make sure the console window isn't placed on top of the game window!!!
13. The cheats should now be loaded, type `1` and press Enter. Enter game PID and you're all set!!
![Main Menu](https://i.imgur.com/rK2E9S2.png)

## Additional info:
'XPCTL menu' allows you to edit XP multipliers to your needs. However, the cheat was designed to treat all memory data as hex bytes, thus these values need to be entered as hex bytes.
The one drawback to this, is the fact that XP multipliers are `float` values, thus you need to convert your desired `float` values to hex data. In main menu and the XPCTL menu of the cheat, you'll find an 'FTOX' (float-to-hex) calculator, which will do this conversion for you.
![FTOX (float) 128 to hex](https://i.imgur.com/vY50GPg.png)
The one thing to keep in mind is, your CPU most probably offers x86 backwards compatibility, thus the byte endianness is little-endian. You may be wondering, what the fuck does that mean and how the fuck is it relevant???
To explain as simply as possible, the FTOX calculator will spit out values in big-endian order, for example `128` will be converted to `0x4300`. What you want to do is simply flip the order of those bytes, so our `0x4300` will become `0x0043`. 
The XPCTL menu only lets you write **2 bytes** MAX. So to set the multiplier to `128` (`0x4300`), once you're challenged to enter the desired value, you'll write: `0043`. You will notice that some floating point values when converted to hexadecimal, contain more than 2 bytes:
 ![FTOX bad value](https://i.imgur.com/vY50GPg.png). 
Those values are no good for you. If the FTOX calculator spits such a value, just find another one:
![FTOX good value](https://i.imgur.com/Mir7wAJ.png)