/* ------------------------------------------
   - OLD OFFSETS STACK (playerBase)
   - 0xFD6B7C8
   - 0xFD6C7C8
   - 0xF41F4C8
   ------------------------------------------
   -    -||-           (zmXPScaleBase)
   - 0xFD2E540
   - 0xFD937C0
   - 0xFD947C0
   - 0xF4474C0
   ------------------------------------------
   DIF: 0x94D300    (1.7.4 -> 1.7.5.8408002)
        0x80A2F0    (1.7.5 -> 1.7.6.8408002)
   ------------------------------------------
*/

#include <Windows.h>
#include <iostream>                         // purely for std::cin.. yea.. throw shit at me..

#include <stdlib.h>

#include "driverctl.h"
#include "cowadlina.h"

int pid = 0;                                // game pid

/* player stuff */
uint8_t curPlayer = 0;                      // current selected player
char playerName[16];                        // current player name

/* --- BASE ADDRESSES --- */
uintptr_t BaseAddr = 0x0;
uintptr_t playerBase = 0x0;
uintptr_t ZMXPBase = 0x0;
uintptr_t ZMBotBase = 0x0;
uintptr_t ZMBotListBase = 0x0;
uintptr_t ZMGlobalBase = 0x0;

/* --- GOD MODE --- */
bool playerGodStatus[MAX_LOBBY_PLAYERS] = { false };

/* --- INFIAMMO --- */
volatile bool ammoMtx[MAX_LOBBY_PLAYERS] = { false };           /* ammo thread mutex */
HANDLE ammoTHandles[MAX_LOBBY_PLAYERS] = { NULL };              /* ammo thread handles */
DWORD ammoTIDs[MAX_LOBBY_PLAYERS] = { 0 };                      /* ammo thread IDs */
bool useSmallAmmo = false;

/* --- TELEPORT --- */
volatile bool teleportMtx = false;
HANDLE teleportTHandle = NULL;
DWORD teleportTID = 0;

/* --- INSTAKILL --- */
bool instaKillOnly = false;




void dbgPrint(uintptr_t addr) {
#ifdef COWADLINA_VERBOSE
    printf("[+] ReadReturn: %0x"PRIxPTR"\n", addr);
#endif
}

/* --- memctl --- */

/* Read addr from memory */
uintptr_t readAddrOff(int pid, uintptr_t address, size_t bytes_to_read) {
    uintptr_t readReturn = 0x0;

    if (bytes_to_read == 0) { bytes_to_read = sizeof(uintptr_t); }

    BYTE* buffer = new BYTE[bytes_to_read];
    memset(buffer, 0, bytes_to_read);

    Driver::read_memory(pid, address, (uintptr_t)&buffer[0], bytes_to_read);

    memcpy(&readReturn, buffer, bytes_to_read);

    dbgPrint(readReturn);

    delete[] buffer;
    return readReturn;
}

/* Read string from memory to (char *) dest */
void readStrOff(int pid, uintptr_t address, char* dest, size_t bytes_to_read) {
    BYTE* buffer = new BYTE[bytes_to_read];
    memset(buffer, 0, bytes_to_read);

    Driver::read_memory(pid, address, (uintptr_t)&buffer[0], bytes_to_read);

    memcpy(dest, buffer, bytes_to_read);
}

/* --- threaded hack functions --- */

/* infinite ammo */
DWORD WINAPI updateAmmo(void) {
    /* get curPlayer first, so it can be safely changed */
    uint8_t cPlayerLocalIndex = curPlayer;

    size_t wsz = 1;
    BYTE* wbuf = new BYTE[wsz];
    if (useSmallAmmo == true) {
        wbuf[0] = 0x07;     /* (int)7 */
    }
    else {
        wbuf[0] = 0x19;     /* (int)25 */
    }
    while (ammoMtx[cPlayerLocalIndex] == true) {
        Driver::write_memory(pid, playerBase + (uintptr_t)OFF_CURAMMO, (uintptr_t)&wbuf[0], wsz);
        Sleep(THREAD_SLEEP_MS);
    }

    printf("\n[T%hhu]: Stopped INF_AMMO Lock\n", curPlayer);
    delete[] wbuf;
    return EXIT_SUCCESS;
}

/* Teleport zombies to one place and set their HP to 1 */
DWORD WINAPI teleportZombies(void) {
    size_t rsz = 1;
    size_t wsz = 12;

    BYTE* wbufpos = new BYTE[wsz];
    BYTE* wbufdog = new BYTE[wsz];
    BYTE* wbufhpp = new BYTE[0x2];
    wbufhpp[0] = 0x1;    // 1HP
    wbufhpp[1] = 0x0;

    // This is the location to where ZMs get teleported :: POS TO TP:  .. F0 92 29 44 95 63 99 C3 C5 FD F0 C1 ..
    memcpy(wbufpos, "\xF0\x92\x29\x44\x95\x63\x99\xC3\xC5\xFD\xF0\xC1", 12);
    memcpy(wbufdog, "\xCD\x54\x0E\x44\x8D\xB6\x8F\x43\xE8\xE7\x7A\xC0", 12);    // hellhound tp location: .. CD 54 0E 44 8D B6 8F 43 E8 E7 7A C0 ..
    uintptr_t zombiePosAddr = ZMBotListBase + OFF_ZMCOORDS;
    uintptr_t zombieHPPAddr = ZMBotListBase + OFF_ZMHEALTH;
    uintptr_t zombieDogInfo = ZMBotListBase + 0x65;
    uintptr_t curOffset = 0x0;

    /* The number of zombies is hardcoded here, feel free to change it.. */
    size_t zombieTopVal = 120;

    while (teleportMtx == true) {
        for (size_t i = 0; i <= zombieTopVal; i++) {
            BYTE* curHPBuf = new BYTE[2];   // For checking Zombie HP
            BYTE* houndData = new BYTE[2];  // to check if ZM is hellhound

            Driver::read_memory(pid, zombieHPPAddr + curOffset, (uintptr_t)&curHPBuf[0], 2);            // Read HP
            Driver::read_memory(pid, zombieDogInfo + curOffset, (uintptr_t)&houndData[0], 1);           // Read Dog Byte

            if (curHPBuf[0] != 0x0 || curHPBuf[1] != 0x0) {
                Driver::write_memory(pid, zombieHPPAddr + curOffset, (uintptr_t)&wbufhpp[0], 0x2);      // Write 1HP
                if (instaKillOnly == false) {
                    /* Do not teleport zombies if instaKillOnly switch is enabled. This is not an optimal solution, since we're reading and setting unnecessary stuff above. I don't really care, but please feel free to tweak this.. */
                    if (houndData[0] == 0x06) {
                        Driver::write_memory(pid, zombiePosAddr + curOffset, (uintptr_t)&wbufdog[0], 0x12); // Write POS
                    }
                    else {
                        Driver::write_memory(pid, zombiePosAddr + curOffset, (uintptr_t)&wbufpos[0], 0x12); // Write POS
                    }
                }
            }
            /* jump to next zombie */
            curOffset += OFF_NEXTZOMBIE;
            
            delete[] curHPBuf;
            delete[] houndData;
        }
        curOffset = 0x0;
    }

    delete[] wbufpos;
    delete[] wbufdog;
    delete[] wbufhpp;

    return EXIT_SUCCESS;
}

/* --- thread ctl --- */

int startAmmoThread(uint8_t playerIndex) {
    ammoMtx[playerIndex] = true;
    ammoTHandles[playerIndex] = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)updateAmmo, NULL, 0, &ammoTIDs[playerIndex]);
    if (ammoTHandles[playerIndex] == NULL) { return EXIT_FAILURE; }

    return EXIT_SUCCESS;
}

void stopAmmoThread(uint8_t playerIndex) {
    ammoMtx[playerIndex] = false;
    WaitForSingleObject(ammoTHandles[playerIndex], INFINITE);
}

void stopAllAmmoThreads(void) {
    for (uint8_t i = 0; i < MAX_LOBBY_PLAYERS; i++) {
        if (ammoMtx[i] == true) {
            ammoMtx[i] = false;
            WaitForSingleObject(ammoTHandles[i], INFINITE);
            printf("[ID: %hhu] Thread Stopped\n", i);
        }
    }
}

int startTeleportThread(void) {
    teleportMtx = true;
    teleportTHandle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)teleportZombies, NULL, 0, &teleportTID);
    if (teleportTHandle == NULL) {
        teleportMtx = false;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void stopTeleportThread(void) {
    teleportMtx = false;
    WaitForSingleObject(teleportTHandle, INFINITE);
}


/* Refresh God Status of curPlayer */
void refreshGodStatus(void) {
    BYTE* buffer = new BYTE[2];
    buffer[1] = 0;

    uintptr_t godAddr = playerBase + (uintptr_t)OFF_GODMODE;
    uintptr_t godMode = readAddrOff(pid, godAddr, 1);
    if (godMode == 0x20) {
        playerGodStatus[curPlayer] = false;
    }
    else if (godMode == 0xA0) {
        playerGodStatus[curPlayer] = true;
    }
    delete[] buffer;
}

int getBaseAddr(void) {
    BaseAddr = Driver::GetBaseAddress(pid);
    if (BaseAddr == 0x0) {
        printf("[-] Failed to retrieve Base Address!\n");
        return EXIT_FAILURE;
    }

    printf("[+] Got Base Address: 0x%" PRIxPTR "\n", BaseAddr);

    return EXIT_SUCCESS;
}

int getPlayerBase(void) {
    playerBase = readAddrOff(pid, BaseAddr + (uintptr_t)BASE_PLAYERBASE, sizeof(uintptr_t));
    if (playerBase == 0x0) {
        printf("[-] Failed to retrieve PlayerBase Address!\n");
        return EXIT_FAILURE;
    }

    printf("[+] Got PlayerBase: 0x%" PRIxPTR "\n", playerBase);

    readStrOff(pid, playerBase + OFF_PLAYERNAME, playerName, 15);

    return EXIT_SUCCESS;
}

int getZMXPBase(void) {
    ZMXPBase = BaseAddr + (uintptr_t)BASE_ZMXPSCBASE;
    if (ZMXPBase == 0x0) {
        printf("[-] Failed to retrieve ZMXPScaleBase!\n");
        return EXIT_FAILURE;
    }

    printf("[+] Got ZMXPScaleBase: 0x%" PRIxPTR "\n", ZMXPBase);
    return EXIT_SUCCESS;
}

int getZMGlobalBase(void) {
    uintptr_t ZMGBaseAddr = BaseAddr + (uintptr_t)BASE_PLAYERBASE + (uintptr_t)BOFF_ZMGLBBASE;

    ZMGlobalBase = readAddrOff(pid, ZMGBaseAddr, sizeof(uintptr_t));
    if (ZMGlobalBase == 0x0) {
        printf("[-] Failed to retrieve ZMGlobalBase!\n");
        return EXIT_FAILURE;
    }

    printf("[+] Got ZMGlobalBase: 0x%" PRIxPTR "\n", ZMGlobalBase);
    return EXIT_SUCCESS;
}

int getZMBotBase(void) {
    /* Get ZMBotBase */
    uintptr_t ZMBotBaseAddr = BaseAddr + (uintptr_t)BASE_PLAYERBASE + (uintptr_t)BOFF_ZMBOTBASE;
    ZMBotBase = readAddrOff(pid, ZMBotBaseAddr, sizeof(uintptr_t));
    if (ZMBotBase == 0x0) {
        printf("[-] Failed to get ZMBotBase!\n");
        return EXIT_FAILURE;
    }

    printf("[+] Got ZMBotBase: 0x%" PRIxPTR "\n", ZMBotBase);
    return EXIT_SUCCESS;
}

int getZMBotListBase(void) {
    ZMBotListBase = readAddrOff(pid, ZMBotBase + (uintptr_t)BOFF_ZMBLISTBS, sizeof(uintptr_t));
    if (ZMBotListBase == 0x0) {
        printf("[-] Failed to get ZMBotListBase!\n");
        return EXIT_FAILURE;
    }

    printf("[+] Got ZMBotListBase: 0x%" PRIxPTR "\n", ZMBotListBase);
    return EXIT_SUCCESS;
}

int readcalcEverything(void) {
    if (!pid) {
        printf("Process ID: ");
        fflush(stdout);
        std::cin >> pid;
        std::cin.clear();
        std::cin.ignore();
    }

    if (getBaseAddr()       != EXIT_SUCCESS) { return EXIT_FAILURE; }
    if (getPlayerBase()     != EXIT_SUCCESS) { return EXIT_FAILURE; }
    if (getZMXPBase()       != EXIT_SUCCESS) { return EXIT_FAILURE; }
    if (getZMGlobalBase()   != EXIT_SUCCESS) { return EXIT_FAILURE; }
    if (getZMBotBase()      != EXIT_SUCCESS) { return EXIT_FAILURE; }
    if (getZMBotListBase()  != EXIT_SUCCESS) { return EXIT_FAILURE; }

    //printf("[+] Successfully calculated all required offsets!\n");

    return EXIT_SUCCESS;
}

void displayAddresses(void) {
    printf("[+] BaseAddr:        0x%" PRIxPTR "\n", BaseAddr);
    printf("[+] PlayerBase:      0x%" PRIxPTR "\n", playerBase);
    printf("[+] ZMGlobalBase:    0x%" PRIxPTR "\n", ZMGlobalBase);
    printf("[+] ZMXPBase:        0x%" PRIxPTR "\n", ZMXPBase);
    printf("[+] ZMBotBase:       0x%" PRIxPTR "\n", ZMXPBase);
    printf("[+] ZMBotListBase:   0x%" PRIxPTR "\n", ZMBotListBase);
    
    printf("\n");
    system("pause");
}

int checkDaemonStatus(void) {
    if (!Driver::initialize() || !CheckDriverStatus()) {
        UNICODE_STRING VariableName = RTL_CONSTANT_STRING(VARIABLE_NAME);
        NtSetSystemEnvironmentValueEx(
            &VariableName,
            &DummyGuid,
            0,
            0,
            ATTRIBUTES);//delete var

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}