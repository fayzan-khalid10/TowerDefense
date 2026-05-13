#ifndef ENEMY_TYPES_H
#define ENEMY_TYPES_H

#include "Enemy.h"
#include "SpriteManager.h"
#include <raylib.h>
#include <cmath>
#include <vector>
#include <utility>

#define TILE_SIZE 48

// helper to move along waypoints - inline to avoid multiple definition
inline void moveAlongPath(Enemy* e, std::vector<std::pair<int,int>>& path, float deltaTime) {
    if (e->getPathIndex() >= (int)path.size() - 1) {
        e->setReachedEnd(true);
        e->setAlive(false);
        return;
    }
    int nextIdx = e->getPathIndex() + 1;
    float targetX = path[nextIdx].first  * TILE_SIZE + TILE_SIZE / 2.0f;
    float targetY = path[nextIdx].second * TILE_SIZE + TILE_SIZE / 2.0f;
    float dx = targetX - e->getX();
    float dy = targetY - e->getY();
    float dist = sqrt(dx * dx + dy * dy);
    float moveAmount = e->getSpeed() * deltaTime;
    if (dist <= moveAmount) {
        e->setX(targetX); e->setY(targetY);
        e->setPathIndex(nextIdx);
        e->setProgress(e->getProgress() + dist);
    } else {
        e->setX(e->getX() + (dx / dist) * moveAmount);
        e->setY(e->getY() + (dy / dist) * moveAmount);
        e->setProgress(e->getProgress() + moveAmount);
    }
}

// draw HP bar above enemy
inline void drawHPBar(float x, float y, int hp, int maxHp, int barW, int yOff) {
    float ratio = (float)hp / (float)maxHp;
    Color hpCol = ratio > 0.5f ? GREEN : (ratio > 0.25f ? YELLOW : RED);
    DrawRectangle((int)x - barW/2, (int)y - yOff, barW, 4, DARKGRAY);
    DrawRectangle((int)x - barW/2, (int)y - yOff, (int)(barW * ratio), 4, hpCol);
}

// draw status effect ring around enemy
inline void drawStatusIndicator(Enemy* e, int radius = 16) {
    if (e->isFrozen()) {
        DrawCircleLines((int)e->getX(), (int)e->getY(), (float)radius,       {100, 200, 255, 255});
        DrawCircle     ((int)e->getX(), (int)e->getY(), (float)(radius - 4), {180, 230, 255, 70});
    } else if (e->isSlowed()) {
        DrawCircleLines((int)e->getX(), (int)e->getY(), (float)radius, BLUE);
    }
}

// ------------------------------------------------------------------
// BasicEnemy - robot walker
// ------------------------------------------------------------------
class BasicEnemy : public Enemy {
public:
    BasicEnemy(float x, float y)
        : Enemy(x, y, 250, 60.0f, 10, "BasicEnemy", RED) {}

    void move(std::vector<std::pair<int,int>>& path, float deltaTime) override {
        updateStatusEffects(deltaTime);
        moveAlongPath(this, path, deltaTime);
    }

    void render() override {
        if (gSprites && gSprites->basicEnemy.isLoaded()) {
            Color tint = isFrozen() ? Color{180, 220, 255, 200} : WHITE;
            gSprites->basicEnemy.draw(x, y, 40.0f, tint);
        } else {
            DrawCircle((int)x, (int)y, 12, color);
            DrawCircleLines((int)x, (int)y, 12, MAROON);
        }
        drawHPBar(x, y, hp, maxHp, 28, 24);
        drawStatusIndicator(this, 15);
    }
};

// ------------------------------------------------------------------
// FastEnemy - sleek fast runner
// ------------------------------------------------------------------
class FastEnemy : public Enemy {
public:
    FastEnemy(float x, float y)
        : Enemy(x, y, 130, 140.0f, 15, "FastEnemy", YELLOW) {}

    void move(std::vector<std::pair<int,int>>& path, float deltaTime) override {
        updateStatusEffects(deltaTime);
        moveAlongPath(this, path, deltaTime);
    }

    void render() override {
        if (gSprites && gSprites->fastEnemy.isLoaded()) {
            Color tint = isFrozen() ? Color{180, 220, 255, 200} : WHITE;
            gSprites->fastEnemy.draw(x, y, 38.0f, tint);
        } else {
            Vector2 pts[4] = {{x,y-13},{x+10,y},{x,y+13},{x-10,y}};
            DrawTriangle(pts[0], pts[1], pts[2], color);
            DrawTriangle(pts[0], pts[2], pts[3], color);
        }
        drawHPBar(x, y, hp, maxHp, 26, 22);
        drawStatusIndicator(this, 15);
    }
};

// ------------------------------------------------------------------
// TankEnemy - heavy mech, uses large enemy spritesheet
// ------------------------------------------------------------------
class TankEnemy : public Enemy {
public:
    TankEnemy(float x, float y)
        : Enemy(x, y, 1300, 25.0f, 50, "TankEnemy", DARKGRAY) {}

    void move(std::vector<std::pair<int,int>>& path, float deltaTime) override {
        updateStatusEffects(deltaTime);
        moveAlongPath(this, path, deltaTime);
    }

    void render() override {
        if (gSprites && gSprites->tankEnemy.isLoaded()) {
            Color tint = isFrozen() ? Color{180, 220, 255, 200} : WHITE;
            gSprites->tankEnemy.draw(x, y, 46.0f, tint);
        } else {
            DrawRectangle((int)x-16,(int)y-16,32,32, DARKGRAY);
            DrawRectangleLines((int)x-16,(int)y-16,32,32, BLACK);
        }
        drawHPBar(x, y, hp, maxHp, 36, 28);
        drawStatusIndicator(this, 20);
    }
};

// ------------------------------------------------------------------
// FlyingEnemy - drone bug, ignores path, flies straight to exit
// ------------------------------------------------------------------
class FlyingEnemy : public Enemy {
private:
    float targetX, targetY;
    bool  initialized;

public:
    FlyingEnemy(float x, float y)
        : Enemy(x, y, 150, 80.0f, 25, "FlyingEnemy", PURPLE) {
        targetX = 0; targetY = 0; initialized = false;
    }

    void setTarget(float tx, float ty) {
        targetX = tx; targetY = ty; initialized = true;
    }

    void move(std::vector<std::pair<int,int>>& path, float deltaTime) override {
        updateStatusEffects(deltaTime);

        if (!initialized && path.size() > 0) {
            int last = (int)path.size() - 1;
            targetX = path[last].first  * TILE_SIZE + TILE_SIZE / 2.0f;
            targetY = path[last].second * TILE_SIZE + TILE_SIZE / 2.0f;
            initialized = true;
        }

        float dx = targetX - x;
        float dy = targetY - y;
        float dist = sqrt(dx * dx + dy * dy);
        if (dist < 5.0f) { reachedEnd = true; alive = false; return; }

        float moveAmt = getSpeed() * deltaTime;
        x += (dx / dist) * moveAmt;
        y += (dy / dist) * moveAmt;
        progress += moveAmt;
    }

    void render() override {
        if (gSprites && gSprites->flyingEnemy.isLoaded()) {
            Color tint = isFrozen() ? Color{180, 220, 255, 200} : WHITE;
            gSprites->flyingEnemy.draw(x, y, 38.0f, tint);
        } else {
            DrawTriangle({x+14,y},{x-10,y-10},{x-10,y+10}, color);
            DrawTriangleLines({x+14,y},{x-10,y-10},{x-10,y+10}, VIOLET);
        }
        drawHPBar(x, y, hp, maxHp, 26, 22);
        drawStatusIndicator(this, 16);
    }
};

// ------------------------------------------------------------------
// SplitterEnemy - splits into 2 children on death
//                 reuses basic enemy sprite, tinted orange
// ------------------------------------------------------------------
class SplitterEnemy : public Enemy {
public:
    bool hasSplit;
    bool isChild;

    SplitterEnemy(float x, float y, bool child = false)
        : Enemy(x, y, child ? 100 : 380, child ? 90.0f : 50.0f,
                child ? 5 : 30, "SplitterEnemy", ORANGE) {
        hasSplit = child;
        isChild  = child;
        if (child) color = {255, 165, 0, 200};
    }

    void move(std::vector<std::pair<int,int>>& path, float deltaTime) override {
        updateStatusEffects(deltaTime);
        moveAlongPath(this, path, deltaTime);
    }

    void render() override {
        if (gSprites && gSprites->basicEnemy.isLoaded()) {
            float sz   = isChild ? 28.0f : 36.0f;
            Color tint = isFrozen() ? Color{180, 220, 255, 200}
                                    : Color{255, 160,  60, 255};
            gSprites->basicEnemy.draw(x, y, sz, tint);
        } else {
            DrawRectangle((int)x-8,(int)y-14,16,28, color);
            DrawRectangle((int)x-14,(int)y-8,28,16, color);
            DrawRectangleLines((int)x-8,(int)y-14,16,28, DARKBROWN);
            DrawRectangleLines((int)x-14,(int)y-8,28,16, DARKBROWN);
        }
        drawHPBar(x, y, hp, maxHp, isChild ? 22 : 28, 22);
        drawStatusIndicator(this, isChild ? 13 : 16);
    }
};

#endif
