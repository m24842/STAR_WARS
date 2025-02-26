#include "AnalogOut.h"
#include "LPC17xx.h"
#include "PeripheralNames.h"
#include "cmsis_armclang_ltm.h"
#include "doublyLinkedList.hpp"
#include "globals.hpp"
#include "hardware.hpp"
#include "enemy.hpp"
#include "finalBoss.hpp"
#include "player.hpp"
#include "game.hpp"
#include "login.hpp"
#include "projectile.hpp"
#include "registration.hpp"
#include "gameMusic.hpp"
#include "graphics.hpp"
#include "users.hpp"
#include <cstdio>

#define timer_read_f(x)     chrono::duration<float>((x).elapsed_time()).count()
#define timer_read_s(x)     chrono::duration_cast<chrono::seconds>((x).elapsed_time()).count();
#define timer_read_ms(x)    chrono::duration_cast<chrono::milliseconds>((x).elapsed_time()).count()
#define timer_read_us(x)    (x).elapsed_time().count()
#define EXIT_GAME 999
#define REGISTER_USER 111

int main() {
    (hardware_init() == 0) ? printf("\033cGame Starting\n") : printf("\033cHardware Init Failed\n");
    uLCD.cls();
    musicInit();
    drawProfileImg();
    Timer startup;
    startup.start();
    while (startup.elapsed_time().count() < 5000000) loadMusic();
    drawControls();
    while (1) {
        if (!readInputs()->normalAttack) {
            buttonSound();
            break;
        }
        loadMusic();
    }
    while (1) {
        loginInit();
        int loginOutput;
        while ((loginOutput = loginUpdate()) == 0) loadMusic();
        deleteLogin();
        if (loginOutput == EXIT_GAME) {
            drawControls();
            while (1) {
                if (!readInputs()->normalAttack) {
                    buttonSound();
                    break;
                }
                loadMusic();
            }
            continue;
        }
        else if (loginOutput == REGISTER_USER) {
            registrationInit();
            int registrationOutput;
            while ((registrationOutput = registrationUpdate()) == 0) loadMusic();
            deleteRegistration();
            if (registrationOutput == EXIT_GAME) break;
            else if (registrationOutput == -1) continue; // Essentially returns to login page
        }
        while (1) {
            // Only enter game menu once logged in
            menuInit();
            int menuOutput;
            while ((menuOutput = menuUpdate()) == 0) loadMusic();
            if (menuOutput == EXIT_GAME) break; // Essentially returns to login page
            uLCD.cls();
            playerInit();
            
            if (getMenuSettings()->gameMode == GAME_MODE::LEVELS) {
                generateGameLoop(1);
            } else if (getMenuSettings()->gameMode == GAME_MODE::INFINITE) {
                generateGameLoop(4);
            } else if (getMenuSettings()->gameMode == GAME_MODE::SCORECAP) {
                generateGameLoop(5);
            } else if (getMenuSettings()->gameMode == GAME_MODE::PVP) {
                generateGameLoop(6);
            }
            resetGameLoop();

            // Update user data
            getUserInfo()->highScore = (getPlayer()->score > getUserInfo()->highScore) ? getPlayer()->score : getUserInfo()->highScore;
            getUserInfo()->totalEnemiesKilled += getPlayer()->sessionKills;
            getUserInfo()->totalPlayerDeaths += getPlayer()->sessionDeaths;
            getUserInfo()->totalPoints += getPlayer()->score;
            getUserInfo()->totalPlayTime += getPlayer()->sessionPlayTime / 1000000; // microseconds to seconds
            updateUser(getUserInfo());
            getPlayer()->score = 0;
            getPlayer()->sessionKills = 0;
            getPlayer()->sessionDeaths = 0;
            getPlayer()->sessionPlayTime = 0;
        }
    }
    uLCD.cls();
    deletePlayer();
    deallocateHardware();
    deleteMenu();
    printf("Ending Game\n");
    return 0;
}