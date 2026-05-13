#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <raylib.h>
#include <cmath>

// simple projectile struct for visual effects
// not a full class because its just visual
struct Projectile {
    float x, y;
    float targetX, targetY;
    float speed;
    Color color;
    int radius;
    bool alive;
    int damage;
    bool isSlow;    // for slow tower projectiles

    Projectile(float sx, float sy, float tx, float ty, float spd, Color c, int r, int dmg, bool slow = false) {
        x = sx;
        y = sy;
        targetX = tx;
        targetY = ty;
        speed = spd;
        color = c;
        radius = r;
        alive = true;
        damage = dmg;
        isSlow = slow;
    }

    void update(float dt) {
        float dx = targetX - x;
        float dy = targetY - y;
        float dist = sqrt(dx * dx + dy * dy);

        if (dist < speed * dt) {
            alive = false;
            return;
        }

        x += (dx / dist) * speed * dt;
        y += (dy / dist) * speed * dt;
    }

    void draw() {
        DrawCircle((int)x, (int)y, radius, color);
    }

    bool isCloseToTarget() {
        float dx = targetX - x;
        float dy = targetY - y;
        return sqrt(dx * dx + dy * dy) < 10.0f;
    }
};

#endif
