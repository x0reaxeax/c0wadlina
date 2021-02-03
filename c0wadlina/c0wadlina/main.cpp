/* 
 * c0wadlina - EFI COD Black Ops Cold War Zombies Tool
 * Credits:
 *   - TheCruZ      [ EFI DAEMON - https://github.com/TheCruZ/EFI_Driver_Access ]
 *   - JayKoZa2015  [ ZM Offsets ]
 *   - x0reaxeax    [ This Ugly Shit ]
 * 
 * INFO: 
 *   - This tool was built for private research and purely educational purposes. Yikes.
 *   - There are lot of improvements this tool could use, feel free to remodel this to your likings!
 *   - Some handy reworks could implement automatic endianness swap, better code layout and performance adjustments.
 *   - The ugly globals could be replaced and templates for R/W could be implemented to get rid of duplicate code.
 *   - As for the system() calls, please please feel free to replace them, they're only used for clearing the screen (https://docs.microsoft.com/en-us/windows/console/clearing-the-screen) and for "pause" command.
 *
 * All offsets can be easily updated, all you need is a sigscanner for playerBase:
 *  EXAMPLE:
 *          newPlayerBase +/- oldPlayerBase = difference
 *          oldXPScale +/- difference = newXPScale
 *          etc.
 * 
 * NOTE: This project is licensed under GNU GPLv3. Modifications are more then welcome, as long as the source and credits are disclosed too.
 * 
 * Selling this tool or modified versions of it is STRICTLY PROHIBITED.
 * 
 * https://github.com/x0reaxeax/c0wadlina
 * https://github.com/TheCruZ/EFI_Driver_Access
 * 
 * C++ standard: std:c++latest
 * 
*/

#include <iostream>
#include <sstream>
#include <vector>
#include "cowadlina.h"      /* cheat stuff */
#include "driverctl.h"      /* daemon stuff */

#include <stdlib.h>         /* for... ... ... system()... yeaaaaa.. */

#define CMENU_EXIT   99

void printMenu(void);
int menuctl(void);
void xpctl(void);

int main(int argc, const char *argv[]) {
    if (checkDaemonStatus() != EXIT_SUCCESS) {
        printf("[-] No EFI Daemon found! Exiting..\n");
        system("pause");
        return EXIT_FAILURE;
    }

    printf("[+] EFI Daemon Found!\n");
    printf("--------------------------\n");
    printf(" * Credits:\n");
    printf("   - TheCruZ      [ EFI DAEMON - https://github.com/TheCruZ/EFI_Driver_Access ]\n");
    printf("   - JayKoZa2015  [ ZM Offsets ]\n");
    printf("   - x0reaxeax    [ Lame-Ass tool ]\n\n");

    if (readcalcEverything() != EXIT_SUCCESS) {
        printf("[-] Unable to acquire memory addresses! Exiting..\n");
        system("pause");
        return EXIT_FAILURE;
    }

    printf("\n[+] Successfully acquired all required addresses!\n");
    Sleep(1000);


    while (true) {
        refreshGodStatus();

        system("cls");

        printMenu();

        if (menuctl() == CMENU_EXIT) {
            break;
        }
    }

    return EXIT_SUCCESS;
}

/* read memory */
void rmem_r(void) {
    printf("(HEX16) Address: ");
    fflush(stdout);

    uintptr_t addr = 0;

    std::string addrData;
    std::cin >> addrData;
    std::cin.clear();
    std::cin.ignore();

    addr = std::stoull(addrData, nullptr, 16);
    
    printf("Number of bytes: ");
    fflush(stdout);

    size_t bytes;
    std::cin >> bytes;
    std::cin.clear();
    std::cin.ignore();

    BYTE* buffer = new BYTE[bytes];
    memset(buffer, 0, bytes);
    Driver::read_memory(pid, addr, (uintptr_t)&buffer[0], bytes);

    printf("[+] Read:\n");
    for (size_t i = 0; i < bytes; i++) {
        printf("%02X ", buffer[i]);
    }
    printf("\n\n");

    delete[] buffer;

    system("pause");
}

/* write memory */
void wmem_w(void) {
    printf("(HEX16) Address: ");
    fflush(stdout);

    uintptr_t addr = 0;

    std::string addrData;
    std::cin >> addrData;
    std::cin.clear();
    std::cin.ignore();

    addr = std::stoull(addrData, nullptr, 16);

    printf("Number of bytes: ");
    fflush(stdout);

    size_t bytes;
    std::cin >> bytes;
    std::cin.clear();
    std::cin.ignore();

    BYTE* buffer = new BYTE[bytes + 0x1];
    memset(buffer, 0, bytes);

    std::string byteData;

    printf("hexData: ");
    fflush(stdout);

    std::cin >> byteData;

    uintptr_t byted = std::stoull(byteData, nullptr, 16);
    size_t shift = 8 * (bytes - 1);
    for (size_t i = 0; i < bytes; i++) {
        if (shift <= 0) {
            buffer[i] = byted & 0xFF;
        }
        else {
            buffer[i] = (byted >> shift) & 0xFF;
        }
        shift -= 8;
    }

    Driver::write_memory(pid, addr, (uintptr_t)&buffer[0], bytes);
    printf("Bytes Written:\n");
    for (size_t i = 0; i < bytes; i++) {
        printf("%02X ", buffer[i]);
    }
    printf("\n\n");

    delete[] buffer;

    system("pause");
}

void ftoxcalc(void) {
    /* FTOX Calculator */
    char fvalData[128];
    printf("(float) Value: ");
    fflush(stdout);

    std::cin >> fvalData;
    std::cin.clear();
    std::cin.ignore();

    float uval = (float)atof(fvalData);
    union {
        float fval;
        uint32_t uival;
    } f2u = { .fval = uval };

    printf("FTOX: %f   : 0x%" PRIx32 "\n", uval, f2u.uival);
    system("pause");
}

void printMenu(void) {
    printf("PID: %d (0x%2x)\n\n", pid, pid);
    printf("EFI Daemon - User Control Panel [for uc.me by x0reaxeax]\n");
    printf("Choose your destiny:\n");

    printf("\n--- Current Player: [ %s ] ---\n", playerName);
    printf("1   - Refresh offsets and addresses\n");
    printf("2   - Shift to Next Player\n");
    printf("3   - Shift to Previous Player\n");
    if (playerGodStatus[curPlayer] == true) {
        printf("4   - God Mode [ON]\n");
    }
    else {
        printf("4   - God Mode [OFF]\n");
    }

    printf("5   - Convert Float to Hex (FTOX Calculator)\n");
    printf("6   - Enter XPCTL Menu\n");
    printf("\n--- Threaded Operations ---\n\n");

    if (ammoMtx[curPlayer] == true) {
        printf("7   - Infinite ammo for current player [ON]\n");
    }
    else {
        printf("7   - Infinite ammo for current player [OFF]\n");
    }

    printf("8   - Stop ALL ammo threads\n");

    char swON[] =   "InstaKill Spam";
    char swOFF[] =  "Teleport & InstaKill 1HP Spam";
    char* info;
    info = (instaKillOnly == true ? (instaKillOnly == false ? swOFF : swON) : swOFF);
    
    if (teleportMtx == true) {
        printf("9   - %s [ON]\n", info);
    }
    else {
        printf("9   - %s [OFF]\n", info);
    }

    printf("\n--- Control Panel ---\n\n");

    if (useSmallAmmo == true) {
        printf("10  - Set Infinite Ammo magazine size to 25\n");
    } else {
        printf("10  - Set Infinite Ammo magazine size to 7\n");
    }

    printf("11  - Switch TeleportOnly / TeleportInstaKill\n");

    printf("\n--- RAW OPERATIONS ---\n\n");
    printf("77  - Read process memory\n");
    printf("79  - Write process memory\n");
    printf("80  - Display Base Addresses\n");

    printf("\n99  - Exit\n\n\n$: ");
    fflush(stdout);
}

int menuctl(void) {
    int action = 0;
    std::cin >> action;
    std::cin.clear();
    std::cin.ignore();

    switch (action) {

        case CMENU_EXIT:
            printf("[+] Exiting..\n");
            return CMENU_EXIT;
            break;

        case 1: /* Refresh addresses */
            if (readcalcEverything() != EXIT_SUCCESS) {
                printf("[-] Failed to read memory addresses!\n");
            } else {
                printf("[+] Successfully calculated all required offsets!\n");
            }
            system("pause");
            break;

        case 2: /* Shift to next player */
            playerBase += OFF_NEXTPLAYER;
            readStrOff(pid, playerBase + OFF_PLAYERNAME, playerName, 15);
            curPlayer++;
            break;

        case 3: /* Shift to prev player */
            playerBase -= OFF_NEXTPLAYER;
            readStrOff(pid, playerBase + OFF_PLAYERNAME, playerName, 15);
            curPlayer--;
            break;

        case 4: { /* Flip godmode */
            BYTE *buffer = new BYTE[2];
            buffer[1] = 0;

            uintptr_t godAddr = playerBase + (uintptr_t)OFF_GODMODE;
            uintptr_t godMode = readAddrOff(pid, godAddr, 1);

            if (godMode == 0x20) {
                buffer[0] = 0xa0;
                Driver::write_memory(pid, godAddr, (uintptr_t)&buffer[0], 1);
                printf("[+] GodMode has been enabled for player %s\n", playerName);
                playerGodStatus[curPlayer] = true;
            }
            else if (godMode == 0xA0) {
                buffer[0] = 0x20;
                Driver::write_memory(pid, godAddr, (uintptr_t)&buffer[0], 1);
                printf("[+] GodMode has been disabled for player %s\n", playerName);
                playerGodStatus[curPlayer] = false;
            }
            else {
                printf("[-] Failed to flip GodMode! Unknown GodStatus: %" PRIxPTR "\n", godMode);
            }
            delete[] buffer;

            system("pause");
            break;
        }

        case 5: { /* FTOX Calculator */
            ftoxcalc();
            break;
        }

        case 6: /* XPCTL */
            xpctl();
            break;

        case 7: { /* InfiAmmo */
            if (ammoMtx[curPlayer] == false) {
                int tret = startAmmoThread(curPlayer);
                if (tret != EXIT_SUCCESS) {
                    printf("[-] Failed to spawn ammo thread for playerID: %hhu\n", curPlayer);
                    ammoMtx[curPlayer] = false;
                }
                printf("[ + ID: %hhu] Ammo thread started..\n", curPlayer);
            }
            else if (ammoMtx[curPlayer] == true) {
                stopAmmoThread(curPlayer);
                printf("[ + ID: %hhu] Ammo thread stopped..\n", curPlayer);
            }
            system("pause");
            break;
        }

        case 8: {
            printf("[+] Stopping ALL ammo threads..\n");
            stopAllAmmoThreads();
            printf("[+] Success..\n");
            system("pause");
            break;
        }

        case 9: {
            if (teleportMtx == false) {
                int tret = startTeleportThread();
                if (tret != EXIT_SUCCESS) {
                    printf("[+] Failed to start TeleportSpam thread..\n");
                    teleportMtx = false;
                }
                else {
                    std::cout << "[+] Successfully started TeleportSpam!\n";
                }
            }
            else {
                stopTeleportThread();
                printf("[+] TeleportSpam Thread Finished\n");
            }
            system("pause");
            break;
        }

        case 10:
            useSmallAmmo ^= true;
            break;

        case 11:
            instaKillOnly ^= true;
            break;
        
        case 77:
            rmem_r();
            break;

        case 79:
            wmem_w();
            break;

        case 80:
            displayAddresses();
            break;
        
        default:
            break;

        break;
    }

    return EXIT_SUCCESS;
}

void xpctl(void) {
    bool gotXPM = false;
    bool gotXXP = false;

    uintptr_t xpmValue = 0x0;
    uintptr_t xxpValue = 0x0;
    uintptr_t xrpValue = 0x0;

    while (true) {
        system("cls");
        printf("XP Multiplier Menu\nValues are written in LITTLE ENDIAN and FTOX calculator returns BIG ENDIAN!\nSo watchu gonna do?\n\n");
        if (gotXPM) {
            printf("1 - Refresh Current Weapon Multiplier (HEX16) [0x%" PRIxPTR "]\n", xpmValue);
        } else {
            printf("1 - Read Current Weapon Multiplier (HEX16)\n");
        }

        printf("2 - Overwrite Current Weapon XP Multiplier (HEX16)\n");

        if (gotXXP) {
            printf("3 - Refresh Current XP Multiplier (HEX16) [0x%" PRIxPTR "]\n", xxpValue);
        } else {
            printf("3 - Read Current XP Multiplier (HEX16)\n");
        }

        printf("4 - Overwrite Current XP Multiplier (HEX16)\n");
        printf("5 - Convert Float to Hex ( FTOX CALC /BE/ )\n");
        printf("9 - Book it back to main menu\n\n\n$: ");
        fflush(stdout);

        int xpmaction = 0;
        std::cin >> xpmaction;
        std::cin.clear();
        std::cin.ignore();

        switch (xpmaction) {
        
            case 1: {
                xpmValue = readAddrOff(pid, ZMXPBase + OFF_WPXP, 2);
                if (xpmValue != 0x0) {
                    printf("[+] Current Modifier: %" PRIxPTR "\n", xpmValue);
                    gotXPM = true;
                } else {
                    printf("[-] The XPM value is 0x0, you've read some wrong bullshit.. Here is some DBG for you: [ADDR] 0x%" PRIxPTR "\n", (ZMXPBase + OFF_WPXP));
                    gotXPM = false;
                }
                system("pause");
                break;
            }

            case 2: {
                size_t wsz = 0x02;
                BYTE* wbuf = new BYTE[wsz];
                memset(wbuf, 0, wsz);

                std::string byteData;

                printf("hexData to write: ");
                fflush(stdout);
                std::cin >> byteData;

                uintptr_t byted = std::stoull(byteData, nullptr, 16);
                size_t shift = 8 * (wsz - 1);
                for (size_t i = 0; i < wsz; i++) {
                    if (shift <= 0) {
                        wbuf[i] = byted & 0xFF;
                    }
                    else {
                        wbuf[i] = (byted >> shift) & 0xFF;
                    }
                    shift -= 8;
                }
                Driver::write_memory(pid, ZMXPBase + (uintptr_t)OFF_WPXP, (uintptr_t)&wbuf[0], wsz);

                printf("[+] Written:\n");
                for (size_t i = 0; i < wsz; i++) {
                    printf("%02X ", wbuf[i]);
                }
                printf("\n\n");
                delete[] wbuf;
                
                system("pause");
                break;
            }

            case 3: {
                xxpValue = readAddrOff(pid, BaseAddr + (uintptr_t)BOFF_BASEXPXP, 2);
                xrpValue = readAddrOff(pid, BaseAddr + (uintptr_t)BOFF_BASERLXP, 2);
                if (xxpValue != 0x0) {
                    if (xxpValue != xrpValue) {
                        printf("[-] RealXP value doesn't match VisualXP!! Something got fucked up..\n");
                    }
                    printf("[+] Current Modifier: %" PRIxPTR "\n", xxpValue);
                    gotXXP = true;
                }
                else {
                    printf("[-] The XPP value is 0x0, you've read some wrong bullshit.. Here is some DBG for you: [ADDR] 0x%" PRIxPTR "\n", (BaseAddr + (uintptr_t)BOFF_BASEXPXP));
                    gotXXP = false;
                }
                system("pause");
                break;
            }

            case 4: {
                size_t wsz = 0x02;
                BYTE* wbuf = new BYTE[wsz];
                memset(wbuf, 0, wsz);

                std::string byteData;

                printf("hexData to write: ");
                fflush(stdout);
                std::cin >> byteData;

                uintptr_t byted = std::stoull(byteData, nullptr, 16);
                size_t shift = 8 * (wsz - 1);
                for (size_t i = 0; i < wsz; i++) {
                    if (shift <= 0) {
                        wbuf[i] = byted & 0xFF;
                    }
                    else {
                        wbuf[i] = (byted >> shift) & 0xFF;
                    }
                    shift -= 8;
                }
                Driver::write_memory(pid, BaseAddr + (uintptr_t)BOFF_BASEXPXP, (uintptr_t)&wbuf[0], wsz);    // Write VisualXP
                Driver::write_memory(pid, BaseAddr + (uintptr_t)BOFF_BASERLXP, (uintptr_t)&wbuf[0], wsz);    // Write RealXP

                printf("[+] Written:\n");
                for (size_t i = 0; i < wsz; i++) {
                    printf("%02X ", wbuf[i]);
                }
                printf("\n\n");
                delete[] wbuf;
                system("pause");
                break;
            }
        

            case 5: {
                ftoxcalc();
                break;
            }

            case 9:
                printf("[+] Going back to main menu..\n");
                system("pause");
                return;
                break;
            default:
                break;
            
            break;
        }
    }
}