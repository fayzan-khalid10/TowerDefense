#include "Game.h"
#include <cmath>
#include <cstdio>
#include <algorithm>

const float Game::AUTO_SAVE_INTERVAL = 10.0f;

static int         towerCosts[5]  = {100, 150, 120, 90, 180};
static const char* towerNames[5]  = {"Cannon", "Sniper", "MachGun", "Slow", "Frost"};
static Color       towerColors[5] = {
    DARKGRAY,
    {0,100,200,255},
    {0,150,0,255},
    {0,150,255,255},
    {180,230,255,255}
};

// ---------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------
Game::Game() {
    gold               = 200;
    lives              = 20;
    score              = 0;
    gameState          = STATE_MENU;
    prevState          = STATE_MENU;
    selectedTowerType  = -1;
    selectedTower      = nullptr;
    hoverCol = hoverRow = -1;
    waitingForNextWave  = true;
    autoSaveTimer       = 0.0f;
    gameOverSoundPlayed  = false;
    victorySoundPlayed   = false;
    waveStartSoundPlayed = false;
}

Game::~Game() {
    cleanup();
    audio.unload();
}

void Game::cleanup() {
    for (int i = 0; i < (int)enemies.size(); i++) delete enemies[i];
    enemies.clear();
    for (int i = 0; i < (int)towers.size(); i++)  delete towers[i];
    towers.clear();
}

void Game::resetGame() {
    cleanup();
    gold               = 200;
    lives              = 20;
    score              = 0;
    selectedTowerType  = -1;
    selectedTower      = nullptr;
    waitingForNextWave  = true;
    autoSaveTimer       = 0.0f;
    gameOverSoundPlayed = false;
    waveStartSoundPlayed= false;
    floatTexts.clear();
    waveManager = WaveManager();
    gameState   = STATE_PLAYING;
}

// ---------------------------------------------------------------
// Main loop
// ---------------------------------------------------------------
void Game::run() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Tower Defense - OOP Project");
    SetTargetFPS(60);

    gameMap.loadAssets();
    audio.init();

    // set global sprite pointer and load all textures
    gSprites = &spriteManager;
    spriteManager.loadAll();

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        audio.update();
        spriteManager.updateAll(GetFrameTime());

        handleInput();

        if (gameState == STATE_PLAYING) {
            update(dt);
        }

        draw();
    }

    // auto-save on exit if mid-game
    if (gameState == STATE_PLAYING && waveManager.getCurrentWave() > 0) {
        saveCurrentGame();
    }

    gameMap.unloadAssets();
    spriteManager.unloadAll();
    gSprites = nullptr;
    CloseWindow();
}

// ---------------------------------------------------------------
// Input
// ---------------------------------------------------------------
void Game::handleInput() {
    // --- MENU ---
    if (gameState == STATE_MENU) {
        if (IsKeyPressed(KEY_ENTER)) {
            // check for saved game
            if (saveSystem.hasSaveFile()) {
                loadSavedGame();
            } else {
                gameState = STATE_PLAYING;
            }
        }
        if (IsKeyPressed(KEY_N)) {
            saveSystem.deleteSave();
            resetGame();
        }
        if (IsKeyPressed(KEY_H)) {
            gameState = STATE_HISCORES;
        }
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            int mx = GetMouseX(), my = GetMouseY();
            // Continue button area
            Rectangle contBtn = {(float)(SCREEN_WIDTH/2 - 140), 460, 280, 50};
            Rectangle newBtn  = {(float)(SCREEN_WIDTH/2 - 140), 525, 280, 50};
            Rectangle hiBtn   = {(float)(SCREEN_WIDTH/2 - 140), 590, 280, 50};
            if (CheckCollisionPointRec({(float)mx,(float)my}, contBtn)) {
                if (saveSystem.hasSaveFile()) loadSavedGame();
                else { saveSystem.deleteSave(); resetGame(); }
            }
            if (CheckCollisionPointRec({(float)mx,(float)my}, newBtn)) {
                saveSystem.deleteSave();
                resetGame();
            }
            if (CheckCollisionPointRec({(float)mx,(float)my}, hiBtn)) {
                gameState = STATE_HISCORES;
            }
        }
        return;
    }

    // --- HIGH SCORES ---
    if (gameState == STATE_HISCORES) {
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_M) || IsKeyPressed(KEY_BACKSPACE)) {
            gameState = STATE_MENU;
        }
        return;
    }

    // --- GAME OVER / WIN ---
    if (gameState == STATE_GAMEOVER || gameState == STATE_WIN) {
        if (IsKeyPressed(KEY_R)) resetGame();
        if (IsKeyPressed(KEY_M)) { resetGame(); gameState = STATE_MENU; }
        if (IsKeyPressed(KEY_H)) gameState = STATE_HISCORES;
        return;
    }

    // --- PAUSE ---
    if (gameState == STATE_PAUSED) {
        if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE)) {
            gameState = prevState;
        }
        if (IsKeyPressed(KEY_M)) { saveCurrentGame(); resetGame(); gameState = STATE_MENU; }
        if (IsKeyPressed(KEY_S)) saveCurrentGame();
        return;
    }

    // --- PLAYING ---
    if (IsKeyPressed(KEY_P)) {
        prevState = gameState;
        gameState = STATE_PAUSED;
        saveCurrentGame();
        return;
    }

    int mx = GetMouseX(), my = GetMouseY();
    hoverCol = mx / TILE_SIZE;
    hoverRow = my / TILE_SIZE;

    if (IsKeyPressed(KEY_ONE))   selectedTowerType = 0;
    if (IsKeyPressed(KEY_TWO))   selectedTowerType = 1;
    if (IsKeyPressed(KEY_THREE)) selectedTowerType = 2;
    if (IsKeyPressed(KEY_FOUR))  selectedTowerType = 3;
    if (IsKeyPressed(KEY_FIVE))  selectedTowerType = 4;
    if (IsKeyPressed(KEY_ESCAPE)) { selectedTowerType = -1; selectedTower = nullptr; }

    // only allow starting next wave if we haven't completed all 5 waves yet
    if (IsKeyPressed(KEY_SPACE) && waitingForNextWave && waveManager.getCurrentWave() < waveManager.getTotalWaves()) {
        waveManager.startNextWave();
        waitingForNextWave    = false;
        waveStartSoundPlayed  = false;
    }

    if (IsKeyPressed(KEY_U) && selectedTower) tryUpgradeSelected();

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (my < MAP_ROWS * TILE_SIZE) {
            if (selectedTowerType >= 0) {
                placeTower(hoverCol, hoverRow);
            } else {
                selectedTower = nullptr;
                for (int i = 0; i < (int)towers.size(); i++) {
                    int tc = (int)(towers[i]->getX() / TILE_SIZE);
                    int tr = (int)(towers[i]->getY() / TILE_SIZE);
                    if (tc == hoverCol && tr == hoverRow) {
                        selectedTower = towers[i];
                        break;
                    }
                }
            }
        }

        // HUD button area
        if (my >= MAP_ROWS * TILE_SIZE) {
            int btnY = MAP_ROWS * TILE_SIZE + 10;
            int btnW = 90, btnH = 50, startX = 10;

            for (int i = 0; i < 5; i++) {
                Rectangle btn = {(float)(startX + i*(btnW+8)), (float)btnY, (float)btnW, (float)btnH};
                if (CheckCollisionPointRec({(float)mx,(float)my}, btn)) {
                    selectedTowerType = (selectedTowerType == i) ? -1 : i;
                    if (selectedTowerType >= 0) selectedTower = nullptr;
                }
            }

            Rectangle upgradeBtn = {(float)(startX + 5*(btnW+8)), (float)btnY, 100, (float)btnH};
            if (CheckCollisionPointRec({(float)mx,(float)my}, upgradeBtn) && selectedTower)
                tryUpgradeSelected();

            Rectangle waveBtn = {(float)(SCREEN_WIDTH - 140), (float)btnY, 130, (float)btnH};
            if (CheckCollisionPointRec({(float)mx,(float)my}, waveBtn)
                && waitingForNextWave && waveManager.getCurrentWave() < waveManager.getTotalWaves()) {
                waveManager.startNextWave();
                waitingForNextWave   = false;
                waveStartSoundPlayed = false;
            }
        }
    }

    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
        selectedTowerType = -1;
        selectedTower     = nullptr;
    }
}

// ---------------------------------------------------------------
// Tower placement
// ---------------------------------------------------------------
void Game::placeTower(int col, int row) {
    if (!gameMap.isBuildable(col, row)) return;

    int cost = towerCosts[selectedTowerType];
    if (gold < cost) {
        addFloatText(col*TILE_SIZE+24, row*TILE_SIZE, "Need gold!", RED);
        return;
    }

    for (int i = 0; i < (int)towers.size(); i++) {
        if ((int)(towers[i]->getX()/TILE_SIZE) == col &&
            (int)(towers[i]->getY()/TILE_SIZE) == row) return;
    }

    float px = col*TILE_SIZE + TILE_SIZE/2.0f;
    float py = row*TILE_SIZE + TILE_SIZE/2.0f;

    Tower* t = nullptr;
    if      (selectedTowerType == 0) t = new CannonTower(px, py);
    else if (selectedTowerType == 1) t = new SniperTower(px, py);
    else if (selectedTowerType == 2) t = new MachineGunTower(px, py);
    else if (selectedTowerType == 3) t = new SlowTower(px, py);
    else if (selectedTowerType == 4) t = new FrostTower(px, py);

    if (t) {
        towers.push_back(t);
        gold -= cost;
        addFloatText(px, py-30, "-"+std::to_string(cost)+"g", ORANGE);
        // use sniper sound for sniper tower, cannon for all others
        // sound plays when the tower actually fires, not when placed
    }
}

void Game::tryUpgradeSelected() {
    if (!selectedTower) return;
    if (!selectedTower->canUpgrade()) {
        addFloatText(selectedTower->getX(), selectedTower->getY()-30, "Max level!", YELLOW);
        return;
    }
    int upgCost = selectedTower->getUpgradeCost();
    if (gold < upgCost) {
        addFloatText(selectedTower->getX(), selectedTower->getY()-30, "Need gold!", RED);
        return;
    }
    gold -= upgCost;
    selectedTower->upgrade();
    addFloatText(selectedTower->getX(), selectedTower->getY()-30, "Upgraded!", GREEN);
    audio.playUpgrade();
}

void Game::spawnSplitChildren(float x, float y, int pathIdx, float prog) {
    for (int i = 0; i < 2; i++) {
        SplitterEnemy* child = new SplitterEnemy(x + (i*10-5), y + (i*10-5), true);
        child->setPathIndex(pathIdx);
        child->setProgress(prog);
        enemies.push_back(child);
    }
}

// ---------------------------------------------------------------
// Save / Load
// ---------------------------------------------------------------
void Game::doAutoSave() {
    autoSaveTimer += GetFrameTime();
    if (autoSaveTimer >= AUTO_SAVE_INTERVAL) {
        autoSaveTimer = 0.0f;
        saveCurrentGame();
    }
}

void Game::saveCurrentGame() {
    GameSaveData data;
    data.hasSave           = true;
    data.gold              = gold;
    data.lives             = lives;
    data.score             = score;
    data.currentWave       = waveManager.getCurrentWave();
    data.waitingForNextWave= waitingForNextWave;

    for (int i = 0; i < (int)towers.size(); i++) {
        GameSaveData::TowerSave ts;
        ts.col  = (int)(towers[i]->getX() / TILE_SIZE);
        ts.row  = (int)(towers[i]->getY() / TILE_SIZE);
        ts.upgradeLevel = towers[i]->getUpgradeLevel();

        // figure out tower type from name
        std::string n = towers[i]->getName();
        if      (n == "Cannon")    ts.type = 0;
        else if (n == "Sniper")    ts.type = 1;
        else if (n == "MachineGun")ts.type = 2;
        else if (n == "SlowTower") ts.type = 3;
        else if (n == "FrostTower")ts.type = 4;
        else                        ts.type = 0;

        data.towers.push_back(ts);
    }

    saveSystem.saveGame(data);
}

void Game::loadSavedGame() {
    GameSaveData data;
    if (!saveSystem.loadGame(data)) {
        resetGame();
        return;
    }

    cleanup();
    gold               = data.gold;
    lives              = data.lives;
    score              = data.score;
    waitingForNextWave  = data.waitingForNextWave;
    selectedTowerType  = -1;
    selectedTower      = nullptr;
    floatTexts.clear();
    gameOverSoundPlayed = false;
    waveStartSoundPlayed= false;

    waveManager = WaveManager();
    waveManager.setWave(data.currentWave);

    // rebuild towers
    for (int i = 0; i < (int)data.towers.size(); i++) {
        GameSaveData::TowerSave& ts = data.towers[i];
        float px = ts.col*TILE_SIZE + TILE_SIZE/2.0f;
        float py = ts.row*TILE_SIZE + TILE_SIZE/2.0f;

        Tower* t = nullptr;
        if      (ts.type == 0) t = new CannonTower(px, py);
        else if (ts.type == 1) t = new SniperTower(px, py);
        else if (ts.type == 2) t = new MachineGunTower(px, py);
        else if (ts.type == 3) t = new SlowTower(px, py);
        else if (ts.type == 4) t = new FrostTower(px, py);

        if (t) {
            // apply upgrade levels
            for (int u = 1; u < ts.upgradeLevel; u++) t->upgrade();
            towers.push_back(t);
        }
    }

    gameState = STATE_PLAYING;
}

// ---------------------------------------------------------------
// Update
// ---------------------------------------------------------------
void Game::update(float dt) {
    gameMap.update(dt);

    // play wave start sound once per wave
    if (waveManager.isWaveActive() && !waveStartSoundPlayed) {
        audio.playWaveStart();
        waveStartSoundPlayed = true;
    }

    auto& path = gameMap.getPath();
    std::vector<Enemy*> newEnemies = waveManager.update(dt, path);
    for (int i = 0; i < (int)newEnemies.size(); i++)
        enemies.push_back(newEnemies[i]);

    for (int i = 0; i < (int)enemies.size(); i++) {
        if (enemies[i]->isAlive())
            enemies[i]->move(path, dt);
    }

    for (int i = 0; i < (int)towers.size(); i++) {
        towers[i]->attack(enemies, dt);

        // play sound if tower fired this frame
        if (towers[i]->shotFired) {
            towers[i]->shotFired = false;  // reset flag
            // sniper gets its own sound, all others use cannon sound
            if (towers[i]->getName() == "Sniper") {
                audio.playSniper();
            } else {
                audio.playCannon();
            }
        }
    }

    // handle dead enemies
    for (int i = (int)enemies.size()-1; i >= 0; i--) {
        Enemy* e = enemies[i];
        if (!e->isAlive()) {
            if (e->hasReachedEnd()) {
                lives--;
                addFloatText(e->getX(), e->getY()-20, "-1 Life!", RED);
                if (lives <= 0) {
                    lives = 0;
                    gameState = STATE_GAMEOVER;
                    if (!gameOverSoundPlayed) {
                        audio.playGameOver();
                        gameOverSoundPlayed = true;
                        saveSystem.addHighScore(score, waveManager.getCurrentWave(), lives);
                        saveSystem.deleteSave();
                    }
                }
            } else {
                SplitterEnemy* sp = dynamic_cast<SplitterEnemy*>(e);
                if (sp && !sp->hasSplit) {
                    sp->hasSplit = true;
                    spawnSplitChildren(e->getX(), e->getY(), e->getPathIndex(), e->getProgress());
                }
                int reward = e->getReward();
                gold  += reward;
                score += reward;
                addFloatText(e->getX(), e->getY()-20, "+"+std::to_string(reward)+"g", GOLD);
                audio.playEnemyDie();
            }
            delete e;
            enemies.erase(enemies.begin()+i);
        }
    }

    if (!waveManager.isWaveActive() && enemies.empty() && !waitingForNextWave) {
        waitingForNextWave   = true;
        waveStartSoundPlayed = false;
        // win immediately after killing all enemies in wave 5 (don't wait for wave 6)
        if (waveManager.getCurrentWave() >= waveManager.getTotalWaves()) {
            gameState = STATE_WIN;
            saveSystem.addHighScore(score, waveManager.getCurrentWave(), lives);
            saveSystem.deleteSave();
            if (!victorySoundPlayed) {
                audio.playVictory();
                victorySoundPlayed = true;
            }
        }
    }

    updateFloatTexts(dt);

    // auto-save every 10 seconds
    autoSaveTimer += dt;
    if (autoSaveTimer >= AUTO_SAVE_INTERVAL) {
        autoSaveTimer = 0.0f;
        saveCurrentGame();
    }
}

// ---------------------------------------------------------------
// Draw
// ---------------------------------------------------------------
void Game::draw() {
    BeginDrawing();
    ClearBackground({30,30,30,255});

    if (gameState == STATE_MENU)      { drawMenu();       EndDrawing(); return; }
    if (gameState == STATE_HISCORES)  { drawHighScores(); EndDrawing(); return; }

    if (gameState == STATE_GAMEOVER) {
        gameMap.render();
        for (auto t : towers)  t->render();
        for (auto e : enemies) if (e->isAlive()) e->render();
        drawGameOver();
        EndDrawing(); return;
    }
    if (gameState == STATE_WIN) {
        gameMap.render();
        for (auto t : towers) t->render();
        drawWin();
        EndDrawing(); return;
    }

    // STATE_PLAYING or STATE_PAUSED
    gameMap.render();

    if (selectedTower) {
        selectedTower->drawRange();
        int tx = (int)(selectedTower->getX() - TILE_SIZE/2);
        int ty = (int)(selectedTower->getY() - TILE_SIZE/2);
        DrawRectangleLines(tx, ty, TILE_SIZE, TILE_SIZE, YELLOW);
    }

    if (selectedTowerType >= 0 && hoverRow >= 0 && hoverRow < MAP_ROWS &&
        hoverCol >= 0 && hoverCol < MAP_COLS) {
        if (gameMap.isBuildable(hoverCol, hoverRow)) {
            DrawRectangle(hoverCol*TILE_SIZE, hoverRow*TILE_SIZE, TILE_SIZE, TILE_SIZE, {255,255,0,60});
            DrawRectangleLines(hoverCol*TILE_SIZE, hoverRow*TILE_SIZE, TILE_SIZE, TILE_SIZE, YELLOW);
            float px2 = hoverCol*TILE_SIZE + TILE_SIZE/2.0f;
            float py2 = hoverRow*TILE_SIZE + TILE_SIZE/2.0f;
            float ranges[5] = {120,250,100,110,130};
            DrawCircleLines((int)px2,(int)py2, ranges[selectedTowerType], {255,255,100,100});
        } else {
            DrawRectangle(hoverCol*TILE_SIZE, hoverRow*TILE_SIZE, TILE_SIZE, TILE_SIZE, {255,0,0,60});
        }
    }

    for (auto t : towers)  t->render();
    for (auto e : enemies) if (e->isAlive()) e->render();

    drawFloatTexts();
    drawHUD();
    drawShopPanel();

    if (gameState == STATE_PAUSED) drawPauseOverlay();

    EndDrawing();
}

// ---------------------------------------------------------------
// HUD / Shop
// ---------------------------------------------------------------
void Game::drawHUD() {
    int hudY = MAP_ROWS * TILE_SIZE;
    DrawRectangle(0, hudY, SCREEN_WIDTH, UI_PANEL_H, {20,20,30,255});
    DrawLine(0, hudY, SCREEN_WIDTH, hudY, {80,80,80,255});
}

void Game::drawShopPanel() {
    int hudY  = MAP_ROWS * TILE_SIZE;
    int btnW  = 90, btnH = 50, startX = 10;
    int btnY  = hudY + 10;

    for (int i = 0; i < 5; i++) {
        int bx = startX + i*(btnW+8);
        bool sel       = (selectedTowerType == i);
        bool canAfford = (gold >= towerCosts[i]);

        Color btnBg;
        if      (sel)        btnBg = {60,60,100,255};
        else if (!canAfford) btnBg = {40,25,25,255};
        else                 btnBg = {40,40,60,255};

        DrawRectangle(bx, btnY, btnW, btnH, btnBg);

        Color border;
        if      (sel)        border = YELLOW;
        else if (canAfford)  border = GRAY;
        else                 border = {80,40,40,255};
        DrawRectangleLines(bx, btnY, btnW, btnH, border);

        DrawCircle(bx+15, btnY+25, 8, towerColors[i]);

        Color nameC = canAfford ? WHITE : Color{150,80,80,255};
        DrawText(towerNames[i], bx+27, btnY+8,  11, nameC);

        char cs[16]; sprintf(cs, "%dg", towerCosts[i]);
        Color costC = canAfford ? GOLD : Color{150,80,80,255};
        DrawText(cs, bx+27, btnY+26, 12, costC);

        char key[4]; sprintf(key,"[%d]",i+1);
        DrawText(key, bx+65, btnY+36, 10, {120,120,120,255});
    }

    // upgrade button
    int upgX = startX + 5*(btnW+8);
    if (selectedTower) {
        bool canUpg    = selectedTower->canUpgrade();
        int  upgCost   = selectedTower->getUpgradeCost();
        bool canAfford = (gold >= upgCost);

        Color upgBg = (canAfford && canUpg) ? Color{30,60,30,255} : Color{40,40,40,255};
        DrawRectangle(upgX, btnY, 100, btnH, upgBg);
        DrawRectangleLines(upgX, btnY, 100, btnH, canUpg ? GREEN : GRAY);
        DrawText("UPGRADE", upgX+8, btnY+8, 12, canUpg ? GREEN : GRAY);
        if (canUpg) {
            char cs2[32];
            sprintf(cs2,"Lv%d->%d:%dg", selectedTower->getUpgradeLevel(),
                    selectedTower->getUpgradeLevel()+1, upgCost);
            DrawText(cs2, upgX+4, btnY+26, 10, canAfford ? GOLD : RED);
        } else {
            DrawText("MAX LEVEL", upgX+10, btnY+28, 10, GRAY);
        }
        DrawText("[U]", upgX+75, btnY+36, 10, {120,120,120,255});
    } else {
        DrawRectangle(upgX, btnY, 100, btnH, {30,30,30,255});
        DrawRectangleLines(upgX, btnY, 100, btnH, {60,60,60,255});
        DrawText("UPGRADE", upgX+8, btnY+8,  12, {80,80,80,255});
        DrawText("(select",  upgX+15,btnY+24, 10, {80,80,80,255});
        DrawText(" tower)",  upgX+15,btnY+36, 10, {80,80,80,255});
    }

    // stats
    int statsX = upgX + 115;
    int statsY = btnY + 5;

    DrawText("GOLD:",  statsX,       statsY,    14, GOLD);
    char gs[32]; sprintf(gs,"%d",gold);
    DrawText(gs, statsX+50, statsY, 16, GOLD);

    DrawText("LIVES:", statsX, statsY+20, 14, {255,100,100,255});
    char ls[32]; sprintf(ls,"%d",lives);
    Color lc = lives>10 ? GREEN : (lives>5 ? YELLOW : RED);
    DrawText(ls, statsX+52, statsY+20, 16, lc);

    DrawText("SCORE:", statsX+100, statsY,    14, WHITE);
    char ss[32]; sprintf(ss,"%d",score);
    DrawText(ss, statsX+154, statsY, 16, WHITE);

    char ws[64]; sprintf(ws,"Wave: %d / %d", waveManager.getCurrentWave(), waveManager.getTotalWaves());
    DrawText(ws, statsX+100, statsY+20, 14, SKYBLUE);

    // wave button
    int waveBtnX = SCREEN_WIDTH-140, waveBtnY = btnY;
    // only show wave start button if there are more waves to start (not after wave 5)
    if (waitingForNextWave && waveManager.getCurrentWave() < waveManager.getTotalWaves()) {
        float t = (float)GetTime();
        unsigned char pulse = (unsigned char)(180+75*sinf(t*4));
        DrawRectangle(waveBtnX, waveBtnY, 130, btnH, {30,70,30,255});
        DrawRectangleLines(waveBtnX, waveBtnY, 130, btnH, {pulse,pulse,0,255});
        char wb[32]; sprintf(wb,"WAVE %d", waveManager.getCurrentWave()+1);
        DrawText(wb, waveBtnX+28, waveBtnY+7,  16, {pulse,pulse,0,255});
        DrawText("  [SPACE]", waveBtnX+15, waveBtnY+28, 12, {180,180,0,255});
    } else if (waveManager.isWaveActive()) {
        DrawRectangle(waveBtnX, waveBtnY, 130, btnH, {40,20,20,255});
        DrawRectangleLines(waveBtnX, waveBtnY, 130, btnH, {100,50,50,255});
        DrawText("WAVE",        waveBtnX+30, waveBtnY+5,  16, {180,80,80,255});
        DrawText("IN PROGRESS", waveBtnX+8,  waveBtnY+28, 11, {180,80,80,255});
    } else if (waveManager.getCurrentWave() >= waveManager.getTotalWaves() && !enemies.empty()) {
        // after wave 5, show "finishing" while enemies are still alive
        DrawRectangle(waveBtnX, waveBtnY, 130, btnH, {20,40,20,255});
        DrawText("FINISHING...", waveBtnX+5, waveBtnY+18, 12, GREEN);
    }

    DrawText("[P] Pause  [ESC] Deselect  RClick: Cancel", 10, hudY+68, 11, {100,100,100,255});

    if (selectedTower) {
        char info[128];
        sprintf(info,"Selected: %s  Dmg:%d  Range:%.0f  Lvl:%d",
            selectedTower->getName().c_str(), selectedTower->getDamage(),
            selectedTower->getRange(), selectedTower->getUpgradeLevel());
        DrawText(info, statsX, statsY+40, 11, {200,200,200,255});
    } else if (selectedTowerType >= 0) {
        DrawText("Click a green tile to place tower", statsX, statsY+40, 11, {180,220,180,255});
    }
}

// ---------------------------------------------------------------
// Menu - more spacious and readable
// ---------------------------------------------------------------
void Game::drawMenu() {
    ClearBackground({15,20,35,255});

    bool hasSave = saveSystem.hasSaveFile();

    // title with shadow effect
    const char* title = "TOWER DEFENSE";
    int tw = MeasureText(title, 64);
    DrawText(title, SCREEN_WIDTH/2 - tw/2 + 3, 53, 64, {80,60,0,255});  // shadow
    DrawText(title, SCREEN_WIDTH/2 - tw/2,     50, 64, {255,220,0,255});

    const char* sub = "YOU CAN DO IT. ";
    int sw = MeasureText(sub, 20);
    DrawText(sub, SCREEN_WIDTH/2 - sw/2, 128, 20, {150,150,200,255});

    DrawLine(80, 162, SCREEN_WIDTH-80, 162, {80,80,120,255});

    // two column layout - controls left, towers right
    int leftX  = 80;
    int rightX = SCREEN_WIDTH/2 + 40;
    int startY = 178;
    int lineH  = 24;  // line height - more space than before

    // LEFT COLUMN - How to play
    DrawText("HOW TO PLAY", leftX, startY, 20, WHITE);
    int y = startY + 30;
    DrawText("[1-5]  Select a tower type",           leftX, y, 16, {200,220,200,255}); y += lineH;
    DrawText("Click  Place tower on green tile",     leftX, y, 16, {200,220,200,255}); y += lineH;
    DrawText("[U]    Upgrade selected tower",         leftX, y, 16, {200,220,200,255}); y += lineH;
    DrawText("[SPACE] Start next wave",              leftX, y, 16, {200,220,200,255}); y += lineH;
    DrawText("[P]    Pause / Resume",                leftX, y, 16, {200,220,200,255}); y += lineH;
    DrawText("Survive 5 waves to WIN!",             leftX, y, 16, {100,220,100,255}); y += lineH + 6;

    DrawText("ENEMIES", leftX, y, 18, {255,120,120,255}); y += 26;
    DrawText("Basic    -  Average speed & HP",        leftX, y, 15, {200,200,200,255}); y += lineH;
    DrawText("Fast     -  Very fast, low HP",         leftX, y, 15, {200,200,200,255}); y += lineH;
    DrawText("Tank     -  Slow but extremely tough",  leftX, y, 15, {200,200,200,255}); y += lineH;
    DrawText("Flying   -  Ignores the path entirely", leftX, y, 15, {200,200,200,255}); y += lineH;
    DrawText("Splitter -  Splits into 2 on death",    leftX, y, 15, {200,200,200,255});

    // RIGHT COLUMN - Tower types
    DrawText("TOWERS", rightX, startY, 20, YELLOW);
    int ry = startY + 30;
    DrawText("[1] Cannon    100g  High dmg, splash",   rightX, ry, 15, {220,220,160,255}); ry += lineH;
    DrawText("[2] Sniper    150g  Very long range",     rightX, ry, 15, {160,200,255,255}); ry += lineH;
    DrawText("[3] MachGun   120g  Rapid fire, swarms",  rightX, ry, 15, {160,255,160,255}); ry += lineH;
    DrawText("[4] Slow       90g  Slows AoE enemies",   rightX, ry, 15, {160,200,255,255}); ry += lineH;
    DrawText("[5] Frost     180g  Freezes on hit",      rightX, ry, 15, {220,240,255,255}); ry += lineH;
    DrawText("All towers upgrade up to Level 3!",       rightX, ry, 14, {180,180,130,255}); ry += lineH + 10;

    DrawText("TIPS", rightX, ry, 18, {255,200,100,255}); ry += 28;
    DrawText("Slow + Cannon = great combo",    rightX, ry, 14, {200,200,200,255}); ry += lineH;
    DrawText("Sniper for flying enemies",       rightX, ry, 14, {200,200,200,255}); ry += lineH;
    DrawText("Game auto-saves every 10 sec",   rightX, ry, 14, {200,200,200,255}); ry += lineH;
    DrawText("[P] saves immediately",          rightX, ry, 14, {200,200,200,255});

    // buttons - spaced out to avoid overlap with tips
    int btnW = 280, btnH = 50;
    int btnX = SCREEN_WIDTH/2 - btnW/2;
    int b1Y  = 480, b2Y = 545, b3Y = 610;  // moved down to clear the tips section

    // Continue / New Game button
    if (hasSave) {
        DrawRectangle(btnX, b1Y, btnW, btnH, {30,70,30,255});
        DrawRectangleLines(btnX, b1Y, btnW, btnH, GREEN);
        const char* ct = "CONTINUE SAVED GAME";
        int ctw = MeasureText(ct, 18);
        DrawText(ct, btnX + btnW/2 - ctw/2, b1Y+16, 18, GREEN);
    } else {
        DrawRectangle(btnX, b1Y, btnW, btnH, {50,50,80,255});
        DrawRectangleLines(btnX, b1Y, btnW, btnH, {100,100,150,255});
        const char* ct = "PRESS ENTER TO START";
        int ctw = MeasureText(ct, 18);
        DrawText(ct, btnX + btnW/2 - ctw/2, b1Y+16, 18, {180,180,220,255});
    }

    // New Game button
    DrawRectangle(btnX, b2Y, btnW, btnH, {60,30,30,255});
    DrawRectangleLines(btnX, b2Y, btnW, btnH, {180,80,80,255});
    const char* ng = hasSave ? "[N] NEW GAME  (clears save)" : "[N] NEW GAME";
    int ngw = MeasureText(ng, 18);
    DrawText(ng, btnX + btnW/2 - ngw/2, b2Y+16, 18, {220,130,130,255});

    // High Scores button
    DrawRectangle(btnX, b3Y, btnW, btnH, {30,50,70,255});
    DrawRectangleLines(btnX, b3Y, btnW, btnH, {80,150,200,255});
    const char* hs = "[H] HIGH SCORES";
    int hsw = MeasureText(hs, 18);
    DrawText(hs, btnX + btnW/2 - hsw/2, b3Y+16, 18, {130,190,220,255});

    // flash hint at very bottom
    float t = (float)GetTime();
    unsigned char alpha = (unsigned char)(160 + 95*sinf(t*2.5f));
    const char* hint = hasSave ? "ENTER = Continue  |  N = New  |  H = Scores"
                               : "ENTER = Play  |  N = New  |  H = Scores";
    int hw = MeasureText(hint, 14);
    DrawText(hint, SCREEN_WIDTH/2 - hw/2, 685, 14, {alpha, alpha, (unsigned char)(alpha/2), 255});
}

// ---------------------------------------------------------------
// Pause overlay
// ---------------------------------------------------------------
void Game::drawPauseOverlay() {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, {0,0,0,140});

    const char* pt = "PAUSED";
    int pw = MeasureText(pt, 72);
    DrawText(pt, SCREEN_WIDTH/2 - pw/2 + 3, 223, 72, {40,40,40,255});
    DrawText(pt, SCREEN_WIDTH/2 - pw/2,     220, 72, {255,220,0,255});

    int cx = SCREEN_WIDTH/2, cy = 325;
    DrawText("[P] or [ESC]  Resume",   cx - MeasureText("[P] or [ESC]  Resume",  22)/2, cy,      22, WHITE);     cy += 38;
    DrawText("[S]  Save Game",          cx - MeasureText("[S]  Save Game",         22)/2, cy,      22, SKYBLUE);   cy += 38;
    DrawText("[M]  Main Menu",          cx - MeasureText("[M]  Main Menu",         22)/2, cy,      22, {200,160,100,255});

    DrawText("Game is auto-saved on pause", cx - MeasureText("Game is auto-saved on pause",16)/2, cy+55, 16, {120,120,120,255});
}

// ---------------------------------------------------------------
// Game Over
// ---------------------------------------------------------------
void Game::drawGameOver() {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, {0,0,0,170});

    const char* go = "GAME OVER";
    int gw = MeasureText(go, 72);
    DrawText(go, SCREEN_WIDTH/2 - gw/2 + 3, 183, 72, {120,0,0,255});
    DrawText(go, SCREEN_WIDTH/2 - gw/2,     180, 72, RED);

    char ss[64]; sprintf(ss,"Final Score: %d", score);
    DrawText(ss, SCREEN_WIDTH/2 - MeasureText(ss,30)/2, 280, 30, WHITE);

    char ws2[64]; sprintf(ws2,"Reached Wave: %d / %d", waveManager.getCurrentWave(), waveManager.getTotalWaves());
    DrawText(ws2, SCREEN_WIDTH/2 - MeasureText(ws2,22)/2, 325, 22, YELLOW);

    // show top 3 scores
    auto scores = saveSystem.loadHighScores();
    if (!scores.empty()) {
        DrawText("TOP SCORES:", SCREEN_WIDTH/2 - 80, 370, 18, {200,200,100,255});
        for (int i = 0; i < (int)scores.size() && i < 3; i++) {
            char sc2[64];
            sprintf(sc2, "#%d  %d pts  (Wave %d)", i+1, scores[i].score, scores[i].wave);
            DrawText(sc2, SCREEN_WIDTH/2 - MeasureText(sc2,16)/2, 395+i*22, 16, {200,200,200,255});
        }
    }

    DrawText("[R] Restart   [H] High Scores   [M] Menu",
        SCREEN_WIDTH/2 - MeasureText("[R] Restart   [H] High Scores   [M] Menu",20)/2,
        470, 20, {200,200,200,255});
}

// ---------------------------------------------------------------
// Win
// ---------------------------------------------------------------
void Game::drawWin() {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, {0,0,0,150});

    float t = (float)GetTime();
    unsigned char r2 = (unsigned char)(128+127*sinf(t*2));
    unsigned char g2 = (unsigned char)(128+127*sinf(t*2+2));
    unsigned char b2 = (unsigned char)(128+127*sinf(t*2+4));

    const char* win = "YOU WIN!";
    int ww = MeasureText(win, 80);
    DrawText(win, SCREEN_WIDTH/2 - ww/2 + 3, 173, 80, {40,40,40,255});
    DrawText(win, SCREEN_WIDTH/2 - ww/2,     170, 80, {r2,g2,b2,255});

    DrawText("All 5 waves defeated!", SCREEN_WIDTH/2 - MeasureText("All 5 waves defeated!",24)/2, 268, 24, WHITE);

    char ss[64]; sprintf(ss,"Final Score: %d", score);
    DrawText(ss, SCREEN_WIDTH/2 - MeasureText(ss,28)/2, 308, 28, GOLD);

    char ls2[64]; sprintf(ls2,"Lives Remaining: %d", lives);
    DrawText(ls2, SCREEN_WIDTH/2 - MeasureText(ls2,22)/2, 348, 22, GREEN);

    // top scores
    auto scores = saveSystem.loadHighScores();
    if (!scores.empty()) {
        DrawText("TOP SCORES:", SCREEN_WIDTH/2 - 80, 386, 18, {200,200,100,255});
        for (int i = 0; i < (int)scores.size() && i < 3; i++) {
            char sc2[64];
            sprintf(sc2, "#%d  %d pts  (Wave %d, %d lives)", i+1, scores[i].score, scores[i].wave, scores[i].lives);
            DrawText(sc2, SCREEN_WIDTH/2 - MeasureText(sc2,15)/2, 410+i*22, 15, {200,200,200,255});
        }
    }

    DrawText("[R] Play Again   [H] High Scores   [M] Menu",
        SCREEN_WIDTH/2 - MeasureText("[R] Play Again   [H] High Scores   [M] Menu",20)/2,
        480, 20, {200,200,200,255});
}

// ---------------------------------------------------------------
// High Scores screen
// ---------------------------------------------------------------
void Game::drawHighScores() {
    ClearBackground({10,15,25,255});

    const char* ht = "HIGH SCORES";
    DrawText(ht, SCREEN_WIDTH/2 - MeasureText(ht,52)/2 + 2, 62, 52, {80,60,0,255});
    DrawText(ht, SCREEN_WIDTH/2 - MeasureText(ht,52)/2,     60, 52, GOLD);

    DrawLine(100, 125, SCREEN_WIDTH-100, 125, {80,80,80,255});

    auto scores = saveSystem.loadHighScores();

    if (scores.empty()) {
        const char* empty = "No scores yet - go play!";
        DrawText(empty, SCREEN_WIDTH/2 - MeasureText(empty,24)/2, 280, 24, {150,150,150,255});
    } else {
        // header
        DrawText("Rank", 200,  150, 18, {180,180,100,255});
        DrawText("Score", 340, 150, 18, {180,180,100,255});
        DrawText("Wave",  500, 150, 18, {180,180,100,255});
        DrawText("Lives", 640, 150, 18, {180,180,100,255});
        DrawLine(180, 175, SCREEN_WIDTH-180, 175, {60,60,60,255});

        for (int i = 0; i < (int)scores.size(); i++) {
            int ry2 = 185 + i * 40;
            Color rowColor = (i == 0) ? GOLD : (i == 1) ? LIGHTGRAY : (i == 2) ? Color{180,100,50,255} : Color{180,180,180,255};

            char rankStr[16];   sprintf(rankStr,  "#%d",     i+1);
            char scoreStr2[16]; sprintf(scoreStr2, "%d",    scores[i].score);
            char waveStr2[8];  sprintf(waveStr2,  "%d / 5", scores[i].wave);
            char livesStr2[8]; sprintf(livesStr2, "%d",     scores[i].lives);

            DrawText(rankStr,   200, ry2, 20, rowColor);
            DrawText(scoreStr2, 340, ry2, 20, rowColor);
            DrawText(waveStr2,  500, ry2, 20, rowColor);
            DrawText(livesStr2, 640, ry2, 20, rowColor);
        }
    }

    DrawLine(100, SCREEN_HEIGHT-80, SCREEN_WIDTH-100, SCREEN_HEIGHT-80, {80,80,80,255});
    const char* back = "[ESC] / [BACKSPACE]  Back to Menu";
    DrawText(back, SCREEN_WIDTH/2 - MeasureText(back,18)/2, SCREEN_HEIGHT-60, 18, {150,150,150,255});
}

// ---------------------------------------------------------------
// Float text
// ---------------------------------------------------------------
void Game::addFloatText(float x, float y, std::string txt, Color c) {
    FloatText ft;
    ft.x = x; ft.y = y; ft.text = txt; ft.timer = 1.5f; ft.color = c;
    floatTexts.push_back(ft);
}

void Game::updateFloatTexts(float dt) {
    for (int i = (int)floatTexts.size()-1; i >= 0; i--) {
        floatTexts[i].timer -= dt;
        floatTexts[i].y -= 25.0f * dt;
        if (floatTexts[i].timer <= 0)
            floatTexts.erase(floatTexts.begin()+i);
    }
}

void Game::drawFloatTexts() {
    for (int i = 0; i < (int)floatTexts.size(); i++) {
        FloatText& ft = floatTexts[i];
        float ratio = ft.timer / 1.5f;
        unsigned char alpha = (unsigned char)(255 * ratio);
        Color c = {ft.color.r, ft.color.g, ft.color.b, alpha};
        DrawText(ft.text.c_str(), (int)ft.x, (int)ft.y, 14, c);
    }
}
