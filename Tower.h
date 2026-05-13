#ifndef TOWER_H
#define TOWER_H

#include "Entity.h"
#include "Enemy.h"
#include <vector>
#include <raylib.h>

// abstract tower class - all towers inherit from this
class Tower : public Entity {
protected:
    float range;
    int damage;
    float fireRate;       // shots per second
    float fireCooldown;   // current cooldown timer
    int cost;
    int upgradeLevel;
    Color color;
    Color baseColor;

public:
    bool shotFired;   // set to true by attack() when a shot is fired, Game reads and resets it

    // --- public members below ---
    Tower(float x, float y, int hp, float range, int damage, float fireRate, int cost, std::string name, Color c, Color base)
        : Entity(x, y, hp, name) {
        this->range = range;
        this->damage = damage;
        this->fireRate = fireRate;
        this->fireCooldown = 0.0f;
        this->cost = cost;
        this->upgradeLevel = 1;
        this->color = c;
        this->baseColor = base;
        this->shotFired = false;
    }

    virtual ~Tower() {}

    // pure virtual - attack behavior differs per tower
    virtual void attack(std::vector<Enemy*>& enemies, float deltaTime) = 0;
    virtual void render() = 0;
    virtual void upgrade() = 0;

    void takeDamage(int dmg) override {
        // towers dont take damage in this game
        // but we have to implement it because its pure virtual in Entity
        hp -= dmg;
    }

    // find the enemy that is furthest along the path
    Enemy* findTarget(std::vector<Enemy*>& enemies) {
        Enemy* target = nullptr;
        float bestProgress = -1.0f;

        for (int i = 0; i < (int)enemies.size(); i++) {
            Enemy* e = enemies[i];
            if (!e->isAlive()) continue;

            float dx = e->getX() - x;
            float dy = e->getY() - y;
            float dist = sqrt(dx * dx + dy * dy);

            if (dist <= range) {
                if (e->getProgress() > bestProgress) {
                    bestProgress = e->getProgress();
                    target = e;
                }
            }
        }
        return target;
    }

    // find closest enemy
    Enemy* findClosestTarget(std::vector<Enemy*>& enemies) {
        Enemy* target = nullptr;
        float closestDist = 99999.0f;

        for (int i = 0; i < (int)enemies.size(); i++) {
            Enemy* e = enemies[i];
            if (!e->isAlive()) continue;

            float dx = e->getX() - x;
            float dy = e->getY() - y;
            float dist = sqrt(dx * dx + dy * dy);

            if (dist <= range && dist < closestDist) {
                closestDist = dist;
                target = e;
            }
        }
        return target;
    }

    // draw the range circle (called when tower is selected/hovered)
    void drawRange() {
        DrawCircleLines((int)x, (int)y, range, {255, 255, 255, 80});
    }

    float getRange() { return range; }
    int getDamage() { return damage; }
    float getFireRate() { return fireRate; }
    int getCost() { return cost; }
    int getUpgradeLevel() { return upgradeLevel; }
    int getUpgradeCost() { return cost * upgradeLevel; }
    bool canUpgrade() { return upgradeLevel < 3; }
};

#endif
