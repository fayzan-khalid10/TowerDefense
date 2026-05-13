#include "SpriteManager.h"
#include "Game.h"

// Global sprite manager - declared extern in SpriteManager.h, defined here
SpriteManager* gSprites = nullptr;

int main() {
    Game game;
    game.run();
    return 0;
}
