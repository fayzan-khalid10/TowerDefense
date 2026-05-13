#ifndef SPRITESHEET_H
#define SPRITESHEET_H

#include <raylib.h>
#include <string>

// SpriteSheet class - loads a grid-based sprite sheet and animates it
// Used by enemies and towers for visual animations
class SpriteSheet {
private:
    Texture2D texture;
    bool      loaded;

    int   cols;
    int   rows;
    int   totalFrames;
    int   frameW;
    int   frameH;

    int   currentFrame;
    float frameTimer;
    float frameSpeed;   // seconds per frame

public:
    SpriteSheet() {
        loaded       = false;
        cols         = 1;
        rows         = 1;
        totalFrames  = 1;
        frameW       = 0;
        frameH       = 0;
        currentFrame = 0;
        frameTimer   = 0.0f;
        frameSpeed   = 0.1f;
    }

    ~SpriteSheet() {
        unload();
    }

    // try loading from multiple paths so it works both from Visual Studio
    // and from the bin/Debug folder directly
    bool load(const std::string& filename, int numCols, int numRows, float fps = 10.0f) {
        const char* paths[] = {
            "",                       // current dir (bin/Debug when running)
            "sprites/",               // sprites subfolder
            "bin/Debug/sprites/",     // from VS project root
            "../bin/Debug/sprites/",  // one level up
            "../../bin/Debug/sprites/" // two levels up
        };

        for (int i = 0; i < 5; i++) {
            std::string full = std::string(paths[i]) + filename;
            if (FileExists(full.c_str())) {
                texture      = LoadTexture(full.c_str());
                loaded       = true;
                cols         = numCols;
                rows         = numRows;
                totalFrames  = numCols * numRows;
                frameW       = texture.width  / numCols;
                frameH       = texture.height / numRows;
                frameSpeed   = 1.0f / fps;
                currentFrame = 0;
                frameTimer   = 0.0f;
                return true;
            }
        }
        loaded = false;
        return false;
    }

    void unload() {
        if (loaded) {
            UnloadTexture(texture);
            loaded = false;
        }
    }

    // advance animation - call every frame
    void update(float dt) {
        if (!loaded) return;
        frameTimer += dt;
        if (frameTimer >= frameSpeed) {
            frameTimer   = 0.0f;
            currentFrame = (currentFrame + 1) % totalFrames;
        }
    }

    // draw current frame centered at (cx, cy), scaled to drawSize x drawSize
    void draw(float cx, float cy, float drawSize, Color tint = WHITE) {
        if (!loaded) return;
        int col = currentFrame % cols;
        int row = currentFrame / cols;
        Rectangle src = { (float)(col * frameW), (float)(row * frameH),
                           (float)frameW, (float)frameH };
        Rectangle dst = { cx - drawSize / 2.0f, cy - drawSize / 2.0f,
                           drawSize, drawSize };
        DrawTexturePro(texture, src, dst, {0, 0}, 0.0f, tint);
    }

    // draw static (single-image, frame 0) - used for sniper tower
    void drawStatic(float cx, float cy, float drawW, float drawH, Color tint = WHITE) {
        if (!loaded) return;
        Rectangle src = { 0, 0, (float)texture.width, (float)texture.height };
        Rectangle dst = { cx - drawW / 2.0f, cy - drawH / 2.0f, drawW, drawH };
        DrawTexturePro(texture, src, dst, {0, 0}, 0.0f, tint);
    }

    bool isLoaded() const { return loaded; }
};

#endif
