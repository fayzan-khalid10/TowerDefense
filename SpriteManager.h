#ifndef SPRITEMANAGER_H
#define SPRITEMANAGER_H

#include "SpriteSheet.h"

// SpriteManager loads every sprite sheet once at startup
// and provides them to enemies and towers via the global gSprites pointer
class SpriteManager {
public:
    // --- enemy sheets ---
    // basic enemy: 6 cols x 6 rows = 36 frames  (3228x3444 -> 538x574 per frame)
    SpriteSheet basicEnemy;
    // fast enemy:  5 cols x 5 rows = 25 frames  (2530x2830 -> 506x566 per frame)
    SpriteSheet fastEnemy;
    // flying enemy: 6 cols x 6 rows = 36 frames (3672x3276 -> 612x546 per frame)
    SpriteSheet flyingEnemy;
    // tank enemy (large): 6x6 = 36 frames       (3252x3348 -> 542x558 per frame)
    SpriteSheet tankEnemy;

    // --- tower sheets ---
    // cannon tower: 6 cols x 6 rows = 36 frames (4074x2808 -> 679x468 per frame)
    SpriteSheet cannonTower;
    // machine gun:  5 cols x 5 rows = 25 frames (3200x2650 -> 640x530 per frame)
    SpriteSheet machineGunTower;
    // frost tower:  5 cols x 5 rows = 25 frames (3200x2430 -> 640x486 per frame)
    SpriteSheet frostTower;
    // slow tower:   5 cols x 5 rows = 25 frames (2580x2630 -> 516x526 per frame)
    SpriteSheet slowTower;
    // sniper tower: static single image          (768x768)
    SpriteSheet sniperTower;

    SpriteManager() {}

    void loadAll() {
        basicEnemy.load      ("basic enemy spritesheet.png",   6, 6, 10.0f);
        fastEnemy.load       ("fast enemy spritesheet.png",    5, 5, 14.0f);
        flyingEnemy.load     ("flying enemy spritesheet.png",  6, 6, 12.0f);
        tankEnemy.load       ("large enemy spritesheet.png",   6, 6,  8.0f);

        cannonTower.load     ("cannon_tower.png",              6, 6, 12.0f);
        machineGunTower.load ("machine gun spritesheet.png",   5, 5, 16.0f);
        frostTower.load      ("frostgun spritesheet.png",      5, 5, 10.0f);
        slowTower.load       ("slow tower spritesheet.png",    5, 5, 10.0f);
        sniperTower.load     ("sniper tower.png",              1, 1,  1.0f);
    }

    void unloadAll() {
        basicEnemy.unload();
        fastEnemy.unload();
        flyingEnemy.unload();
        tankEnemy.unload();
        cannonTower.unload();
        machineGunTower.unload();
        frostTower.unload();
        slowTower.unload();
        sniperTower.unload();
    }

    // advance all sheet animations each frame
    void updateAll(float dt) {
        basicEnemy.update(dt);
        fastEnemy.update(dt);
        flyingEnemy.update(dt);
        tankEnemy.update(dt);
        cannonTower.update(dt);
        machineGunTower.update(dt);
        frostTower.update(dt);
        slowTower.update(dt);
        // sniperTower is static - no update needed
    }
};

// global pointer - defined in main.cpp, used by EnemyTypes and TowerTypes
extern SpriteManager* gSprites;

#endif
