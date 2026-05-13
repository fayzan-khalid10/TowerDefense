#ifndef GAME_H
#define GAME_H

#include "GameMap.h"
#include "Enemy.h"
#include "EnemyTypes.h"
#include "Tower.h"
#include "TowerTypes.h"
#include "WaveManager.h"
#include "SaveSystem.h"
#include "AudioManager.h"
#include "SpriteManager.h"
#include <vector>
#include <string>
#include <raylib.h>

#define SCREEN_WIDTH  1056
#define SCREEN_HEIGHT 768
#define MAP_PANEL_W   (MAP_COLS * TILE_SIZE)
#define UI_PANEL_H    96

#define STATE_MENU      0
#define STATE_PLAYING   1
#define STATE_GAMEOVER  2
#define STATE_WIN       3
#define STATE_PAUSED    4
#define STATE_HISCORES  5

class Game {
private:
    GameMap      gameMap;
    WaveManager  waveManager;
    SpriteManager spriteManager;
    SaveSystem   saveSystem;
    AudioManager audio;

    std::vector<Enemy*>  enemies;
    std::vector<Tower*>  towers;

    int gold;
    int lives;
    int score;
    int gameState;
    int prevState;   // state before pausing

    int selectedTowerType;
    Tower* selectedTower;

    int hoverCol, hoverRow;

    struct FloatText {
        float x, y;
        std::string text;
        float timer;
        Color color;
    };
    std::vector<FloatText> floatTexts;

    bool waitingForNextWave;

    // auto-save timer - saves every 10 seconds during play
    float autoSaveTimer;
    static const float AUTO_SAVE_INTERVAL;

    // track if game over sound was already played
    bool gameOverSoundPlayed;
    bool victorySoundPlayed;
    bool waveStartSoundPlayed;

public:
    Game();
    ~Game();
    void run();

private:
    void update(float dt);
    void draw();
    void handleInput();

    void placeTower(int col, int row);
    void tryUpgradeSelected();
    void spawnSplitChildren(float x, float y, int pathIdx, float progress);

    // save / load helpers
    void doAutoSave();
    void saveCurrentGame();
    void loadSavedGame();

    // draw helpers
    void drawHUD();
    void drawShopPanel();
    void drawMenu();
    void drawGameOver();
    void drawWin();
    void drawPauseOverlay();
    void drawHighScores();

    void addFloatText(float x, float y, std::string txt, Color c);
    void updateFloatTexts(float dt);
    void drawFloatTexts();

    void cleanup();
    void resetGame();
};

#endif
