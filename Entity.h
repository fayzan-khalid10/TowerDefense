#ifndef ENTITY_H
#define ENTITY_H

#include <string>

// Base class for everything in the game
// basically all towers and enemies inherit from this
class Entity {
protected:
    float x, y;
    int hp;
    int maxHp;
    bool alive;
    std::string name;

public:
    Entity(float x, float y, int hp, std::string name) {
        this->x = x;
        this->y = y;
        this->hp = hp;
        this->maxHp = hp;
        this->alive = true;
        this->name = name;
    }

    virtual ~Entity() {
        // nothing to clean up here
    }

    // pure virtual functions - must be overridden
    virtual void render() = 0;
    virtual void takeDamage(int dmg) = 0;

    float getX() { return x; }
    float getY() { return y; }
    void setX(float newX) { x = newX; }
    void setY(float newY) { y = newY; }

    int getHp() { return hp; }
    int getMaxHp() { return maxHp; }
    bool isAlive() { return alive; }
    void setAlive(bool a) { alive = a; }

    std::string getName() { return name; }

    // overloading == operator to compare two entities by position
    bool operator==(const Entity& other) const {
        return (x == other.x && y == other.y);
    }
};

#endif
