#ifndef ENEMY_H
#define ENEMY_H

#include "Entity.h"
#include <vector>
#include <utility>
#include <raylib.h>

// abstract base for all enemies
class Enemy : public Entity {
protected:
    float speed;
    int   reward;
    int   pathIndex;
    float progress;
    bool  reachedEnd;
    Color color;

    // SlowTower effect: reduces speed to 40%
    bool  slowed;
    float slowTimer;

    // FrostTower effect: completely stops enemy
    bool  frozen;
    float frozenTimer;

public:
    Enemy(float x, float y, int hp, float speed, int reward, std::string name, Color c)
        : Entity(x, y, hp, name) {
        this->speed       = speed;
        this->reward      = reward;
        this->pathIndex   = 0;
        this->progress    = 0.0f;
        this->reachedEnd  = false;
        this->color       = c;
        this->slowed      = false;
        this->slowTimer   = 0.0f;
        this->frozen      = false;
        this->frozenTimer = 0.0f;
    }

    virtual ~Enemy() {}

    virtual void move(std::vector<std::pair<int,int>>& path, float deltaTime) = 0;
    virtual void render() = 0;

    void takeDamage(int dmg) override {
        hp -= dmg;
        if (hp <= 0) { hp = 0; alive = false; }
    }

    // SlowTower: reduces speed to 40% for the duration
    void applySlow(float duration) {
        slowed    = true;
        slowTimer = duration;
    }

    // FrostTower: completely stops enemy for the duration
    // Only applies if not already frozen - prevents projectile re-hitting from resetting timer endlessly
    void applyFreeze(float duration) {
        if (!frozen) {
            frozen      = true;
            frozenTimer = duration;
        }
        // if already frozen, do nothing - let existing timer run out naturally
    }

    // call every frame to tick down status effect timers
    void updateStatusEffects(float dt) {
        if (frozen) {
            frozenTimer -= dt;
            if (frozenTimer <= 0.0f) {
                frozen      = false;
                frozenTimer = 0.0f;
            }
        }
        if (slowed) {
            slowTimer -= dt;
            if (slowTimer <= 0.0f) {
                slowed    = false;
                slowTimer = 0.0f;
            }
        }
    }

    float getSpeed() {
        if (frozen) return 0.0f;          // FrostTower: completely stopped
        if (slowed) return speed * 0.4f;  // SlowTower: 40% speed (was 0.0 - that was the bug)
        return speed;
    }

    float getBaseSpeed()       { return speed; }
    int   getReward()          { return reward; }
    int   getPathIndex()       { return pathIndex; }
    void  setPathIndex(int i)  { pathIndex = i; }
    bool  hasReachedEnd()      { return reachedEnd; }
    void  setReachedEnd(bool v){ reachedEnd = v; }
    float getProgress()        { return progress; }
    void  setProgress(float p) { progress = p; }
    bool  isSlowed()           { return slowed; }
    bool  isFrozen()           { return frozen; }
};

#endif
