#ifndef TOWER_TYPES_H
#define TOWER_TYPES_H

#include "Tower.h"
#include "Projectile.h"
#include "SpriteManager.h"
#include <vector>
#include <cmath>
#include <raylib.h>

// ------------------------------------------------------------------
// CannonTower - high dmg, splash, slow fire rate
// ------------------------------------------------------------------
class CannonTower : public Tower {
public:
    std::vector<Projectile> projectiles;

    CannonTower(float x, float y)
        : Tower(x, y, 999, 120.0f, 60, 0.8f, 100, "Cannon", DARKGRAY, GRAY) {}

    void upgrade() override {
        if (upgradeLevel < 3) { upgradeLevel++; damage += 40; range += 10; }
    }

    void attack(std::vector<Enemy*>& enemies, float deltaTime) override {
        for (int i = 0; i < (int)projectiles.size(); i++) {
            projectiles[i].update(deltaTime);
            if (projectiles[i].isCloseToTarget()) {
                for (int j = 0; j < (int)enemies.size(); j++) {
                    if (!enemies[j]->isAlive()) continue;
                    float dx = enemies[j]->getX() - projectiles[i].targetX;
                    float dy = enemies[j]->getY() - projectiles[i].targetY;
                    if (sqrt(dx*dx + dy*dy) < 40.0f)
                        enemies[j]->takeDamage(projectiles[i].damage);
                }
                projectiles[i].alive = false;
            }
        }
        for (int i = (int)projectiles.size()-1; i >= 0; i--)
            if (!projectiles[i].alive) projectiles.erase(projectiles.begin()+i);

        fireCooldown -= deltaTime;
        if (fireCooldown > 0) return;

        Enemy* target = findTarget(enemies);
        if (!target) return;

        projectiles.push_back(Projectile(x, y, target->getX(), target->getY(), 250.0f, BLACK, 6, damage));
        fireCooldown = 1.0f / fireRate;
        shotFired = true;
    }

    void render() override {
        if (gSprites && gSprites->cannonTower.isLoaded()) {
            gSprites->cannonTower.draw(x, y, 44.0f);
        } else {
            DrawRectangle((int)x-18,(int)y-18,36,36, baseColor);
            DrawRectangleLines((int)x-18,(int)y-18,36,36, BLACK);
            DrawRectangle((int)x-5,(int)y-22,10,22, color);
            DrawRectangleLines((int)x-5,(int)y-22,10,22, BLACK);
        }
        for (int i = 0; i < upgradeLevel-1; i++)
            DrawCircle((int)x-8+i*8,(int)y+24, 3, GOLD);
        for (int i = 0; i < (int)projectiles.size(); i++)
            projectiles[i].draw();
    }
};

// ------------------------------------------------------------------
// SniperTower - very long range, static sprite with laser line effect
// ------------------------------------------------------------------
class SniperTower : public Tower {
public:
    std::vector<Projectile> projectiles;
    float lastShotX, lastShotY;
    float lineTimer;

    SniperTower(float x, float y)
        : Tower(x, y, 999, 250.0f, 80, 0.5f, 150, "Sniper",
                {0,100,200,255}, {100,149,237,255}) {
        lastShotX = x; lastShotY = y; lineTimer = 0.0f;
    }

    void upgrade() override {
        if (upgradeLevel < 3) { upgradeLevel++; damage += 50; range += 30; fireRate += 0.1f; }
    }

    void attack(std::vector<Enemy*>& enemies, float deltaTime) override {
        lineTimer -= deltaTime;

        for (int i = (int)projectiles.size()-1; i >= 0; i--) {
            projectiles[i].update(deltaTime);
            if (projectiles[i].isCloseToTarget()) {
                for (int j = 0; j < (int)enemies.size(); j++) {
                    if (!enemies[j]->isAlive()) continue;
                    float dx = enemies[j]->getX() - projectiles[i].targetX;
                    float dy = enemies[j]->getY() - projectiles[i].targetY;
                    if (sqrt(dx*dx+dy*dy) < 25.0f)
                        enemies[j]->takeDamage(projectiles[i].damage);
                }
                projectiles[i].alive = false;
            }
            if (!projectiles[i].alive) projectiles.erase(projectiles.begin()+i);
        }

        fireCooldown -= deltaTime;
        if (fireCooldown > 0) return;

        Enemy* target = findTarget(enemies);
        if (!target) return;

        lastShotX = target->getX();
        lastShotY = target->getY();
        lineTimer = 0.15f;

        projectiles.push_back(Projectile(x, y, target->getX(), target->getY(),
            800.0f, {0,200,255,255}, 3, damage));
        fireCooldown = 1.0f / fireRate;
        shotFired = true;
    }

    void render() override {
        // laser flash line
        if (lineTimer > 0)
            DrawLine((int)x,(int)y,(int)lastShotX,(int)lastShotY, {0,200,255,180});

        if (gSprites && gSprites->sniperTower.isLoaded()) {
            gSprites->sniperTower.drawStatic(x, y, 44.0f, 44.0f);
        } else {
            DrawRectangle((int)x-14,(int)y-14,28,28, baseColor);
            DrawRectangleLines((int)x-14,(int)y-14,28,28, DARKBLUE);
            DrawRectangle((int)x-3,(int)y-26,6,26, color);
            DrawRectangleLines((int)x-3,(int)y-26,6,26, DARKBLUE);
        }
        for (int i = 0; i < upgradeLevel-1; i++)
            DrawCircle((int)x-4+i*8,(int)y+24, 3, GOLD);
        for (int i = 0; i < (int)projectiles.size(); i++)
            projectiles[i].draw();
    }
};

// ------------------------------------------------------------------
// MachineGunTower - rapid fire, low damage
// ------------------------------------------------------------------
class MachineGunTower : public Tower {
public:
    std::vector<Projectile> projectiles;

    MachineGunTower(float x, float y)
        : Tower(x, y, 999, 100.0f, 12, 5.0f, 120, "MachineGun",
                {0,150,0,255}, {0,100,0,255}) {}

    void upgrade() override {
        if (upgradeLevel < 3) { upgradeLevel++; damage += 8; fireRate += 2.0f; }
    }

    void attack(std::vector<Enemy*>& enemies, float deltaTime) override {
        for (int i = (int)projectiles.size()-1; i >= 0; i--) {
            projectiles[i].update(deltaTime);
            if (projectiles[i].isCloseToTarget()) {
                for (int j = 0; j < (int)enemies.size(); j++) {
                    if (!enemies[j]->isAlive()) continue;
                    float dx = enemies[j]->getX() - projectiles[i].targetX;
                    float dy = enemies[j]->getY() - projectiles[i].targetY;
                    if (sqrt(dx*dx+dy*dy) < 15.0f)
                        enemies[j]->takeDamage(projectiles[i].damage);
                }
                projectiles[i].alive = false;
            }
            if (!projectiles[i].alive) projectiles.erase(projectiles.begin()+i);
        }

        fireCooldown -= deltaTime;
        if (fireCooldown > 0) return;

        Enemy* target = findTarget(enemies);
        if (!target) return;

        projectiles.push_back(Projectile(x, y, target->getX(), target->getY(),
            350.0f, LIME, 3, damage));
        fireCooldown = 1.0f / fireRate;
        shotFired = true;
    }

    void render() override {
        if (gSprites && gSprites->machineGunTower.isLoaded()) {
            gSprites->machineGunTower.draw(x, y, 44.0f);
        } else {
            DrawRectangle((int)x-14,(int)y-14,28,28, baseColor);
            DrawRectangleLines((int)x-14,(int)y-14,28,28, DARKGREEN);
            DrawRectangle((int)x-8,(int)y-22,4,22, color);
            DrawRectangle((int)x-2,(int)y-24,4,24, color);
            DrawRectangle((int)x+4,(int)y-22,4,22, color);
            DrawRectangleLines((int)x-8,(int)y-24,16,24, DARKGREEN);
        }
        for (int i = 0; i < upgradeLevel-1; i++)
            DrawCircle((int)x-4+i*8,(int)y+24, 3, GOLD);
        for (int i = 0; i < (int)projectiles.size(); i++)
            projectiles[i].draw();
    }
};

// ------------------------------------------------------------------
// SlowTower - AoE slow pulse
// ------------------------------------------------------------------
class SlowTower : public Tower {
private:
    float pulseTimer;

public:
    SlowTower(float x, float y)
        : Tower(x, y, 999, 110.0f, 5, 1.5f, 90, "SlowTower",
                {0,150,255,255}, {0,80,200,255}) {
        pulseTimer = 0.0f;
    }

    void upgrade() override {
        if (upgradeLevel < 3) { upgradeLevel++; range += 20; fireRate += 0.5f; }
    }

    void attack(std::vector<Enemy*>& enemies, float deltaTime) override {
        pulseTimer += deltaTime;
        fireCooldown -= deltaTime;
        if (fireCooldown > 0) return;

        for (int i = 0; i < (int)enemies.size(); i++) {
            if (!enemies[i]->isAlive()) continue;
            float dx = enemies[i]->getX() - x;
            float dy = enemies[i]->getY() - y;
            float dist = sqrt(dx * dx + dy * dy);
            if (dist <= range) {
                enemies[i]->applySlow(1.5f);
                enemies[i]->takeDamage(damage);
            }
        }
        fireCooldown = 1.0f / fireRate;
        shotFired = true;
    }

    void render() override {
        // pulsing AoE ring
        float pulse  = fmod(pulseTimer, 1.0f);
        float pulseR = range * pulse;
        DrawCircleLines((int)x,(int)y,(int)pulseR,
            {0,150,255,(unsigned char)(150*(1.0f-pulse))});

        if (gSprites && gSprites->slowTower.isLoaded()) {
            gSprites->slowTower.draw(x, y, 44.0f);
        } else {
            DrawCircle((int)x,(int)y,18, baseColor);
            DrawCircleLines((int)x,(int)y,18, DARKBLUE);
            DrawCircle((int)x,(int)y,10, color);
            DrawCircleLines((int)x,(int)y,10, WHITE);
        }
        for (int i = 0; i < upgradeLevel-1; i++)
            DrawCircle((int)x-4+i*8,(int)y+24, 3, GOLD);
    }
};

// ------------------------------------------------------------------
// FrostTower - freezes single target for 1.5 seconds
// ------------------------------------------------------------------
class FrostTower : public Tower {
public:
    std::vector<Projectile> projectiles;

    FrostTower(float x, float y)
        : Tower(x, y, 999, 130.0f, 25, 1.2f, 180, "FrostTower",
                {180,230,255,255}, {100,180,255,255}) {}

    void upgrade() override {
        if (upgradeLevel < 3) { upgradeLevel++; damage += 15; range += 15; }
    }

    void attack(std::vector<Enemy*>& enemies, float deltaTime) override {
        for (int i = (int)projectiles.size()-1; i >= 0; i--) {
            projectiles[i].update(deltaTime);
            if (projectiles[i].isCloseToTarget()) {
                for (int j = 0; j < (int)enemies.size(); j++) {
                    if (!enemies[j]->isAlive()) continue;
                    float dx = enemies[j]->getX() - projectiles[i].targetX;
                    float dy = enemies[j]->getY() - projectiles[i].targetY;
                    if (sqrt(dx*dx+dy*dy) < 20.0f) {
                        enemies[j]->takeDamage(projectiles[i].damage);
                        enemies[j]->applyFreeze(1.5f);
                    }
                }
                projectiles[i].alive = false;
            }
            if (!projectiles[i].alive) projectiles.erase(projectiles.begin()+i);
        }

        fireCooldown -= deltaTime;
        if (fireCooldown > 0) return;

        Enemy* target = findClosestTarget(enemies);
        if (!target) return;

        projectiles.push_back(Projectile(x, y, target->getX(), target->getY(),
            300.0f, {180,230,255,255}, 5, damage, true));
        fireCooldown = 1.0f / fireRate;
        shotFired = true;
    }

    void render() override {
        if (gSprites && gSprites->frostTower.isLoaded()) {
            gSprites->frostTower.draw(x, y, 44.0f);
        } else {
            DrawCircle((int)x,(int)y,18, baseColor);
            DrawCircleLines((int)x,(int)y,18, {100,180,255,255});
            DrawLine((int)x-14,(int)y,(int)x+14,(int)y, WHITE);
            DrawLine((int)x,(int)y-14,(int)x,(int)y+14, WHITE);
            DrawLine((int)x-10,(int)y-10,(int)x+10,(int)y+10, WHITE);
            DrawLine((int)x-10,(int)y+10,(int)x+10,(int)y-10, WHITE);
        }
        for (int i = 0; i < upgradeLevel-1; i++)
            DrawCircle((int)x-4+i*8,(int)y+24, 3, GOLD);
        for (int i = 0; i < (int)projectiles.size(); i++)
            projectiles[i].draw();
    }
};

#endif
