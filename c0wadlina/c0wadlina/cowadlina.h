#ifndef _COWADLINA_BASE_H_
#define _COWADLINA_BASE_H_

#include <inttypes.h>

/* --- offsets --- */

/* --- 1.7.6.8408002 --- */
#define BASE_PLAYERBASE     0xFC297B8
#define BASE_ZMXPSCBASE     0xFC517B0

#define BOFF_ZMBOTBASE      0x68
#define BOFF_ZMGLBBASE      0x60
#define BOFF_ZMBLISTBS      0x8

/* PlayerBase */
#define OFF_NEXTPLAYER      0xB830
#define OFF_GODMODE         0xE67
#define OFF_POINTS          0x5CE4
#define OFF_INSTAKILL       0x398
#define OFF_CURAMMO         0x13D8          // this is supposed to be 0x13D4, but we're only gonna write to the last byte, because it's more than enough for current ammo..
#define OFF_MAXAMMO         0x1360
#define OFF_PLAYERNAME      0x5BDA

/* ZMBotBase */
#define OFF_ZMCOUNT         0x3C            // unused

/* ZMBotListBase */
#define OFF_ZMCOORDS        0x2D4
#define OFF_ZMHEALTH        0x398
#define OFF_NEXTZOMBIE      0x5F8

/* ZMXPScaleBase */
#define OFF_WPXP            0x32            // this (OFF_WPXP) is actually supposed to be 0x30 because the float value is represented like this: (LSB) : 00 00 80 3F : (MSB), but we only need to write to the LSB and LSB + 0x01..

#define BOFF_BASEWPXP       0xFC517E0
#define BOFF_BASEXPXP       0xFC517D2       // omit first 2 bytes again.. explanation up here ^^
#define BOFF_BASERLXP       0xFC517DA       // ^^

#define MAX_LOBBY_PLAYERS   4

#define THREAD_SLEEP_MS     10

/* necessary extern bases */
extern uintptr_t BaseAddr;
extern uintptr_t playerBase;
extern uintptr_t ZMXPBase;

extern int pid;								// game pid

/* player stuff */
extern uint8_t curPlayer;					// current selected player
extern char playerName[16];					// current player name


extern bool playerGodStatus[MAX_LOBBY_PLAYERS];			// God Mode Status
extern volatile bool ammoMtx[MAX_LOBBY_PLAYERS];		// Infinite Ammo Status

extern volatile bool teleportMtx;						// Teleport Spam Status
extern bool instaKillOnly;								// InstaKill only in TeleportSpam()

extern bool useSmallAmmo;								// Infinite ammo mag size

/* Check if Deamon is loaded and ready to obey */
int checkDaemonStatus(void);

/* Read and calculate offset addresses */
int readcalcEverything(void);

/* Print all addresses to stdout */
void displayAddresses(void);

/* Refresh God Status of curPlayer */
void refreshGodStatus(void);

/* Start Infinite Ammo thread */
int startAmmoThread(uint8_t playerIndex);
/* Stop Infinite Ammo thread */
void stopAmmoThread(uint8_t playerIndex);
/* Stop ALL Infinite Ammo threads */
void stopAllAmmoThreads(void);

/* Start Teleport thread */
int startTeleportThread(void);
/* Stop Teleport thread */
void stopTeleportThread(void);


/* Read addr from memory */
uintptr_t readAddrOff(int pid, uintptr_t address, size_t bytes_to_read);
/* Read string from memory to (char *) dest */
void readStrOff(int pid, uintptr_t address, char* dest, size_t bytes_to_read);



#endif // !_COWADLINA_BASE_H_
