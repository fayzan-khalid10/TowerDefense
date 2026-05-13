#ifndef GAMEMAP_H
#define GAMEMAP_H

#include <vector>
#include <utility>
#include <cmath>
#include <raylib.h>

#define TILE_SIZE 48
#define MAP_COLS 22
#define MAP_ROWS 14

#define TILE_GRASS  0
#define TILE_PATH   1
#define TILE_START  2
#define TILE_END    3

// Holds info about a decorative tree on the map
struct TreeDecor {
    float x, y;
    float size;       // radius of tree crown
    Color trunkColor;
    Color leafColor;
};

// Holds info about Luffy's position
struct LuffyDecor {
    float x, y;
};

class GameMap {
private:
    int grid[MAP_ROWS][MAP_COLS];
    std::vector<std::pair<int,int>> path;

    Color grassColor = {85, 145, 55, 255};
    Color pathColor  = {180, 150, 100, 255};
    Color startColor = {0, 200, 80, 255};
    Color endColor   = {220, 50, 50, 255};
    Color gridLine   = {60, 120, 40, 80};

    std::vector<TreeDecor> trees;
    LuffyDecor luffy;
    float luffyAnim;  // for animation

    // PNG sprite textures
    Texture2D luffyTexture;
    Texture2D treeTexture;
    bool texturesLoaded;

    void loadTextures() {
        // initialize textures to zero first
        luffyTexture.id = 0;
        treeTexture.id = 0;
        texturesLoaded = false;

        bool luffyLoaded = false;
        bool treeLoaded = false;

        // try to load luffy sprite - look in multiple locations for VS and standalone
        if (FileExists("luffy gear2.png")) {
            luffyTexture = LoadTexture("luffy gear2.png");
            luffyLoaded = (luffyTexture.id != 0);
        } else if (FileExists("bin/Debug/luffy gear2.png")) {
            luffyTexture = LoadTexture("bin/Debug/luffy gear2.png");
            luffyLoaded = (luffyTexture.id != 0);
        } else if (FileExists("../bin/Debug/luffy gear2.png")) {
            luffyTexture = LoadTexture("../bin/Debug/luffy gear2.png");
            luffyLoaded = (luffyTexture.id != 0);
        }

        // try to load tree sprite
        if (FileExists("arcade tree.png")) {
            treeTexture = LoadTexture("arcade tree.png");
            treeLoaded = (treeTexture.id != 0);
        } else if (FileExists("bin/Debug/arcade tree.png")) {
            treeTexture = LoadTexture("bin/Debug/arcade tree.png");
            treeLoaded = (treeTexture.id != 0);
        } else if (FileExists("../bin/Debug/arcade tree.png")) {
            treeTexture = LoadTexture("../bin/Debug/arcade tree.png");
            treeLoaded = (treeTexture.id != 0);
        }

        texturesLoaded = luffyLoaded && treeLoaded;
    }

    void unloadTextures() {
        if (luffyTexture.id != 0) {
            UnloadTexture(luffyTexture);
            luffyTexture.id = 0;
        }
        if (treeTexture.id != 0) {
            UnloadTexture(treeTexture);
            treeTexture.id = 0;
        }
        texturesLoaded = false;
    }

    void placeTrees() {
        trees.clear();
        // put trees on bottom-right area, far from path
        // path mostly goes through rows 2, 5, 8 so rows 10-13 are safe
        // and cols 14-21 bottom area is mostly open grass

        // bottom right cluster
        addTree(16, 10);
        addTree(17, 11);
        addTree(15, 12);
        addTree(18, 12);
        addTree(19, 11);
        addTree(20, 10);
        addTree(21, 11);
        addTree(20, 12);

        // bottom left cluster
        addTree(1, 10);
        addTree(2, 11);
        addTree(1, 12);
        addTree(3, 12);

        // middle bottom area (cols 9-12 row 10-13 - away from path row 8)
        addTree(9,  10);
        addTree(11, 11);
        addTree(10, 13);
        addTree(12, 12);
    }

    void addTree(int col, int row) {
        // only place if it's grass
        if (col < 0 || col >= MAP_COLS || row < 0 || row >= MAP_ROWS) return;
        if (grid[row][col] != TILE_GRASS) return;
        TreeDecor t;
        t.x = col * TILE_SIZE + TILE_SIZE / 2.0f;
        t.y = row * TILE_SIZE + TILE_SIZE / 2.0f;
        t.size = 48.0f;  // sprite size - matches tile size
        t.trunkColor = WHITE;
        t.leafColor  = WHITE;
        trees.push_back(t);
    }

    void drawTree(TreeDecor& t) {
        if (texturesLoaded && treeTexture.id != 0) {
            // draw PNG sprite - centered, scaled to tile size
            Rectangle source = {0, 0, (float)treeTexture.width, (float)treeTexture.height};
            Rectangle dest = {t.x - t.size/2, t.y - t.size, t.size, t.size};
            DrawTexturePro(treeTexture, source, dest, {0, 0}, 0, WHITE);
        } else {
            // fallback: draw procedural tree
            DrawRectangle((int)t.x - 4, (int)t.y - 4, 8, 20, t.trunkColor);
            DrawCircle((int)t.x, (int)t.y - 10, 18, t.leafColor);
            DrawCircle((int)t.x - 12, (int)t.y - 5, 14, GREEN);
            DrawCircle((int)t.x + 12, (int)t.y - 5, 14, DARKGREEN);
        }
    }

    // draw Luffy in Gear 2 - using PNG sprite
    void drawLuffy(float animT) {
        if (texturesLoaded && luffyTexture.id != 0) {
            float x = luffy.x;
            float y = luffy.y;
            float bob = sinf(animT * 3.0f) * 2.0f;  // slight bobbing animation
            y += bob;

            // draw PNG sprite - centered, with slight bobbing animation
            Rectangle source = {0, 0, (float)luffyTexture.width, (float)luffyTexture.height};
            float scale = 0.15f;  // scale down the sprite
            float w = luffyTexture.width * scale;
            float h = luffyTexture.height * scale;
            Rectangle dest = {x - w/2, y - h/2 + bob, w, h};
            DrawTexturePro(luffyTexture, source, dest, {0, 0}, 0, WHITE);
        } else {
            // fallback: draw procedural Luffy (original code)
            float x = luffy.x;
            float y = luffy.y;
            float bob = sinf(animT * 3.0f) * 2.0f;
            y += bob;
            Color skinColor  = {255, 200, 160, 255};
            Color shirtColor = {255, 60,  60,  255};
            Color pantsColor = {30,  80,  200, 255};
            Color steamColor = {255, 150, 150, 120};
            Color hatMain    = {210, 180, 140, 255};
            DrawCircle((int)x, (int)(y + 2), 20, steamColor);
            DrawCircle((int)x, (int)(y + 2), 16, {255, 100, 80, 80});
            DrawRectangle((int)x - 7, (int)(y + 10), 6, 12, pantsColor);
            DrawRectangle((int)x + 1, (int)(y + 10), 6, 12, pantsColor);
            DrawEllipse((int)x - 4, (int)(y + 22), 5, 3, {40, 20, 10, 255});
            DrawEllipse((int)x + 4, (int)(y + 22), 5, 3, {40, 20, 10, 255});
            DrawRectangle((int)x - 9, (int)(y - 4), 18, 16, shirtColor);
            DrawLine((int)x - 3, (int)(y + 2), (int)x + 3, (int)(y + 8), {180, 30, 30, 255});
            DrawLine((int)x + 3, (int)(y + 2), (int)x - 3, (int)(y + 8), {180, 30, 30, 255});
            DrawRectangle((int)x - 20, (int)(y - 2), 12, 5, skinColor);
            DrawRectangle((int)x +  8, (int)(y - 2), 12, 5, skinColor);
            DrawCircle((int)x - 14, (int)(y + 1), 5, skinColor);
            DrawCircle((int)x + 14, (int)(y + 1), 5, skinColor);
            DrawRectangle((int)x - 3, (int)(y - 10), 6, 8, skinColor);
            DrawCircle((int)x, (int)(y - 16), 11, skinColor);
            DrawCircle((int)x - 4, (int)(y - 17), 2, BLACK);
            DrawCircle((int)x + 4, (int)(y - 17), 2, BLACK);
            DrawCircle((int)x - 3, (int)(y - 17), 1, WHITE);
            DrawCircle((int)x + 3, (int)(y - 17), 1, WHITE);
            DrawLine((int)x - 5, (int)(y - 13), (int)x,     (int)(y - 11), BLACK);
            DrawLine((int)x,     (int)(y - 11), (int)x + 5, (int)(y - 13), BLACK);
            DrawEllipse((int)x, (int)(y - 24), 16, 4, hatMain);
            DrawEllipseLines((int)x, (int)(y - 24), 16, 4, {150, 120, 80, 255});
            DrawCircle((int)x, (int)(y - 28), 10, hatMain);
            DrawRectangle((int)x - 10, (int)(y - 27), 20, 4, shirtColor);
            float st = sinf(animT * 5.0f);
            DrawCircle((int)x - 8, (int)(y - 35 + st * 3), 3, {255, 200, 200, 100});
            DrawCircle((int)x + 8, (int)(y - 35 - st * 2), 2, {255, 200, 200, 80});
            DrawCircle((int)x,     (int)(y - 40 + st * 2), 3, {255, 220, 220, 90});
            DrawText("Luffy", (int)x - 14, (int)(y + 27), 10, {255, 220, 100, 220});
            DrawText("Gear 2", (int)x - 16, (int)(y + 38), 9, {255, 140, 100, 200});
        }
    }

public:
    GameMap() {
        for (int r = 0; r < MAP_ROWS; r++)
            for (int c = 0; c < MAP_COLS; c++)
                grid[r][c] = TILE_GRASS;

        luffyAnim = 0.0f;
        texturesLoaded = false;
        luffyTexture.id = 0;
        treeTexture.id = 0;

        buildPath();
        placeTrees();

        // Luffy in bottom-right corner, away from path and trees
        luffy.x = 14 * TILE_SIZE + TILE_SIZE / 2.0f;
        luffy.y = 11 * TILE_SIZE + TILE_SIZE / 2.0f;
    }

    ~GameMap() {
        unloadTextures();
    }

    // prevent copying to avoid double-free of textures
    GameMap(const GameMap&) = delete;
    GameMap& operator=(const GameMap&) = delete;

    void loadAssets() {
        loadTextures();
    }

    void unloadAssets() {
        unloadTextures();
    }

    void buildPath() {
        std::vector<std::pair<int,int>> waypoints = {
            {0, 2}, {4, 2}, {4, 5}, {8, 5}, {8, 2},
            {13, 2}, {13, 8}, {18, 8}, {18, 5}, {21, 5}
        };

        path.clear();

        for (int w = 0; w + 1 < (int)waypoints.size(); w++) {
            int c1 = waypoints[w].first,   r1 = waypoints[w].second;
            int c2 = waypoints[w+1].first, r2 = waypoints[w+1].second;

            if (r1 == r2) {
                int step = (c2 > c1) ? 1 : -1;
                for (int c = c1; c != c2 + step; c += step) {
                    if (r1 >= 0 && r1 < MAP_ROWS && c >= 0 && c < MAP_COLS) {
                        path.push_back({c, r1});
                        grid[r1][c] = TILE_PATH;
                    }
                }
            } else if (c1 == c2) {
                int step = (r2 > r1) ? 1 : -1;
                for (int r = r1; r != r2 + step; r += step) {
                    if (r >= 0 && r < MAP_ROWS && c1 >= 0 && c1 < MAP_COLS) {
                        if (path.empty() || path.back() != std::make_pair(c1, r)) {
                            path.push_back({c1, r});
                            grid[r][c1] = TILE_PATH;
                        }
                    }
                }
            }
        }

        if (!path.empty()) {
            grid[path.front().second][path.front().first] = TILE_START;
            grid[path.back().second][path.back().first]   = TILE_END;
        }
    }

    void update(float dt) {
        luffyAnim += dt;
    }

    void render() {
        for (int r = 0; r < MAP_ROWS; r++) {
            for (int c = 0; c < MAP_COLS; c++) {
                int tile = grid[r][c];
                Color col;
                if      (tile == TILE_GRASS) col = grassColor;
                else if (tile == TILE_PATH)  col = pathColor;
                else if (tile == TILE_START) col = startColor;
                else                          col = endColor;

                DrawRectangle(c * TILE_SIZE, r * TILE_SIZE, TILE_SIZE, TILE_SIZE, col);
                DrawRectangleLines(c * TILE_SIZE, r * TILE_SIZE, TILE_SIZE, TILE_SIZE, gridLine);
            }
        }

        if (!path.empty()) {
            int sc = path.front().first, sr = path.front().second;
            DrawText("IN",  sc * TILE_SIZE + 8,  sr * TILE_SIZE + 16, 14, WHITE);
            int ec = path.back().first,  er = path.back().second;
            DrawText("OUT", ec * TILE_SIZE + 4,  er * TILE_SIZE + 16, 13, WHITE);
        }

        // draw trees
        for (int i = 0; i < (int)trees.size(); i++) {
            drawTree(trees[i]);
        }

        // draw Luffy
        drawLuffy(luffyAnim);
    }

    bool isBuildable(int col, int row) {
        if (col < 0 || col >= MAP_COLS || row < 0 || row >= MAP_ROWS) return false;
        // also block tiles where trees are
        for (int i = 0; i < (int)trees.size(); i++) {
            int tc = (int)(trees[i].x / TILE_SIZE);
            int tr = (int)(trees[i].y / TILE_SIZE);
            if (tc == col && tr == row) return false;
        }
        return grid[row][col] == TILE_GRASS;
    }

    bool isPath(int col, int row) {
        if (col < 0 || col >= MAP_COLS || row < 0 || row >= MAP_ROWS) return false;
        return grid[row][col] != TILE_GRASS;
    }

    std::vector<std::pair<int,int>>& getPath() { return path; }
    int getGrid(int row, int col) { return grid[row][col]; }
};

#endif
