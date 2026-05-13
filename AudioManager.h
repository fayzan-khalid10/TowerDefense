#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <raylib.h>
#include <cmath>
#include <vector>
#include <cstring>

#define AUDIO_SAMPLE_RATE 44100

// AudioManager class - uses external sound files for tower sounds
// cannon.mp3 for cannon/machgun/slow/frost towers, sniper.mp3 for sniper tower
class AudioManager {
private:
    // loaded from files
    Sound cannonSound;      // cannon.mp3 for Cannon, MachGun, Slow, Frost towers
    Sound sniperSound;      // sniper.mp3 for Sniper tower
    Sound gameOverSound;
    Sound victorySound;
    Sound waveStartSound;
    Sound upgradeSound;
    Sound enemyDieSound;

    // background music - simple looping melody
    Music bgMusic;
    bool bgMusicLoaded;
    bool soundEnabled;

    // -------------------------------------------------------
    // Helper: generate a Sound from a raw float buffer
    // -------------------------------------------------------
    Sound makeSoundFromBuffer(std::vector<float>& buf) {
        Wave w;
        w.frameCount = (unsigned int)buf.size();
        w.sampleRate = AUDIO_SAMPLE_RATE;
        w.sampleSize = 32;
        w.channels   = 1;
        w.data = (void*)buf.data();
        return LoadSoundFromWave(w);
    }

    // -------------------------------------------------------
    // Helper: generate a simple tone with optional frequency sweep
    // -------------------------------------------------------
    std::vector<float> genTone(float freqStart, float freqEnd, float duration, float volume, bool fadeOut = true) {
        int samples = (int)(AUDIO_SAMPLE_RATE * duration);
        std::vector<float> buf(samples);
        for (int i = 0; i < samples; i++) {
            float t    = (float)i / AUDIO_SAMPLE_RATE;
            float prog = (float)i / samples;
            float freq = freqStart + (freqEnd - freqStart) * prog;
            float env  = fadeOut ? (1.0f - prog) : 1.0f;
            buf[i] = volume * env * sinf(2.0f * 3.14159f * freq * t);
        }
        return buf;
    }

    // combine two buffers together (mix)
    std::vector<float> mixBuffers(std::vector<float>& a, std::vector<float>& b) {
        int len = (int)(a.size() > b.size() ? a.size() : b.size());
        std::vector<float> out(len, 0.0f);
        for (int i = 0; i < (int)a.size(); i++) out[i] += a[i];
        for (int i = 0; i < (int)b.size(); i++) out[i] += b[i];
        // clamp
        for (int i = 0; i < len; i++) {
            if (out[i] >  1.0f) out[i] =  1.0f;
            if (out[i] < -1.0f) out[i] = -1.0f;
        }
        return out;
    }

    // append buffer b after a
    std::vector<float> appendBuffers(std::vector<float>& a, std::vector<float>& b) {
        std::vector<float> out;
        out.insert(out.end(), a.begin(), a.end());
        out.insert(out.end(), b.begin(), b.end());
        return out;
    }

    std::vector<float> silence(float duration) {
        int samples = (int)(AUDIO_SAMPLE_RATE * duration);
        return std::vector<float>(samples, 0.0f);
    }

    // -------------------------------------------------------
    // Generate the background chiptune loop
    // Inspired by a simple heroic march melody
    // -------------------------------------------------------
    std::vector<float> generateBGMelody() {
        // note frequencies for a simple 8-note heroic phrase
        // roughly inspired by a dramatic anime-style theme
        float notes[] = {
            329.63f, 392.00f, 493.88f, 587.33f,   // E4 G4 B4 D5
            523.25f, 440.00f, 392.00f, 349.23f,   // C5 A4 G4 F4
            392.00f, 493.88f, 587.33f, 659.25f,   // G4 B4 D5 E5
            587.33f, 523.25f, 493.88f, 440.00f,   // D5 C5 B4 A4
            // second phrase - builds up
            523.25f, 587.33f, 659.25f, 783.99f,   // C5 D5 E5 G5
            698.46f, 659.25f, 587.33f, 523.25f,   // F5 E5 D5 C5
            493.88f, 440.00f, 392.00f, 349.23f,   // B4 A4 G4 F4
            329.63f, 293.66f, 261.63f, 0.0f        // E4 D4 C4 rest
        };

        float noteDur = 0.22f;
        float vol = 0.18f;  // faint volume - background music

        std::vector<float> melody;
        int noteCount = sizeof(notes) / sizeof(float);

        for (int i = 0; i < noteCount; i++) {
            if (notes[i] < 1.0f) {
                // rest
                auto s = silence(noteDur);
                melody = appendBuffers(melody, s);
            } else {
                auto note = genTone(notes[i], notes[i], noteDur, vol, true);
                melody = appendBuffers(melody, note);
            }
        }

        // bass line underneath - lower octave, half notes
        float bassNotes[] = {
            130.81f, 130.81f, 146.83f, 146.83f,
            130.81f, 123.47f, 130.81f, 116.54f,
            130.81f, 146.83f, 164.81f, 196.00f,
            174.61f, 164.81f, 146.83f, 130.81f,
            130.81f, 146.83f, 164.81f, 196.00f,
            174.61f, 164.81f, 146.83f, 130.81f,
            123.47f, 110.00f, 98.00f, 87.31f,
            82.41f, 73.42f, 65.41f, 65.41f
        };

        std::vector<float> bass;
        for (int i = 0; i < noteCount; i++) {
            auto note = genTone(bassNotes[i], bassNotes[i], noteDur, 0.08f, false);
            bass = appendBuffers(bass, note);
        }

        // mix melody + bass
        auto mixed = mixBuffers(melody, bass);

        // add a slight "reverb" feel by mixing a delayed copy
        std::vector<float> delayed(mixed.size(), 0.0f);
        int delayS = (int)(0.06f * AUDIO_SAMPLE_RATE);
        for (int i = delayS; i < (int)mixed.size(); i++) {
            delayed[i] = mixed[i - delayS] * 0.25f;
        }
        auto final = mixBuffers(mixed, delayed);

        return final;
    }

    void loadAllSounds() {
        // load cannon sound - used for Cannon, MachGun, Slow, Frost towers
        if (FileExists("cannon.mp3")) {
            cannonSound = LoadSound("cannon.mp3");
            SetSoundVolume(cannonSound, 0.20f);  // lowered
        } else {
            // fallback: generate a low boom procedurally
            auto a = genTone(120.0f, 60.0f, 0.18f, 0.7f, true);
            auto b = genTone(200.0f, 80.0f, 0.12f, 0.4f, true);
            auto buf = mixBuffers(a, b);
            cannonSound = makeSoundFromBuffer(buf);
        }

        // load sniper sound
        if (FileExists("sniper.mp3")) {
            sniperSound = LoadSound("sniper.mp3");
            SetSoundVolume(sniperSound, 0.22f);  // lowered
        } else {
            // fallback: generate a sharp shot procedurally
            auto buf = genTone(400.0f, 200.0f, 0.1f, 0.6f, true);
            sniperSound = makeSoundFromBuffer(buf);
        }

        // victory sound - loaded from Victory.mp3
        if (FileExists("Victory.mp3")) {
            victorySound = LoadSound("Victory.mp3");
            SetSoundVolume(victorySound, 0.7f);
        } else if (FileExists("bin/Debug/Victory.mp3")) {
            victorySound = LoadSound("bin/Debug/Victory.mp3");
            SetSoundVolume(victorySound, 0.7f);
        } else if (FileExists("../bin/Debug/Victory.mp3")) {
            victorySound = LoadSound("../bin/Debug/Victory.mp3");
            SetSoundVolume(victorySound, 0.7f);
        } else {
            // fallback: generate ascending victory fanfare
            float vNotes[] = {261.63f, 329.63f, 392.0f, 523.25f, 659.25f, 783.99f};
            std::vector<float> buf;
            for (int i = 0; i < 6; i++) {
                auto n = genTone(vNotes[i], vNotes[i], 0.18f, 0.5f, true);
                buf = appendBuffers(buf, n);
            }
            victorySound = makeSoundFromBuffer(buf);
        }

        // game over - descending sad tones
        {
            float goNotes[] = {523.25f, 440.0f, 349.23f, 261.63f};
            std::vector<float> buf;
            for (int i = 0; i < 4; i++) {
                auto n = genTone(goNotes[i], goNotes[i] * 0.9f, 0.35f, 0.55f, true);
                auto s = silence(0.05f);
                buf = appendBuffers(buf, n);
                buf = appendBuffers(buf, s);
            }
            gameOverSound = makeSoundFromBuffer(buf);
        }

        // wave start - ascending fanfare
        {
            float fanNotes[] = {261.63f, 329.63f, 392.0f, 523.25f};
            std::vector<float> buf;
            for (int i = 0; i < 4; i++) {
                auto n = genTone(fanNotes[i], fanNotes[i], 0.12f, 0.5f, true);
                buf = appendBuffers(buf, n);
            }
            waveStartSound = makeSoundFromBuffer(buf);
        }

        // upgrade - short positive ding
        {
            auto a = genTone(660.0f, 880.0f, 0.1f, 0.45f, true);
            auto b = genTone(880.0f, 1100.0f, 0.08f, 0.35f, true);
            auto buf = appendBuffers(a, b);
            upgradeSound = makeSoundFromBuffer(buf);
        }

        // enemy die - little pop
        {
            auto buf = genTone(300.0f, 150.0f, 0.07f, 0.35f, true);
            enemyDieSound = makeSoundFromBuffer(buf);
        }
    }

    // load background music from a file if it exists
    // otherwise use our generated chiptune
    void setupBackgroundMusic() {
        bgMusicLoaded = false;

        // try to load external file - look in multiple locations
        // this works both when running from VS and from bin/Debug folder
        if (FileExists("naruto.mp3")) {
            bgMusic = LoadMusicStream("naruto.mp3");
            bgMusicLoaded = true;
        } else if (FileExists("bin/Debug/naruto.mp3")) {
            bgMusic = LoadMusicStream("bin/Debug/naruto.mp3");
            bgMusicLoaded = true;
        } else if (FileExists("../bin/Debug/naruto.mp3")) {
            bgMusic = LoadMusicStream("../bin/Debug/naruto.mp3");
            bgMusicLoaded = true;
        } else if (FileExists("naruto.ogg")) {
            bgMusic = LoadMusicStream("naruto.ogg");
            bgMusicLoaded = true;
        } else if (FileExists("naruto.wav")) {
            bgMusic = LoadMusicStream("naruto.wav");
            bgMusicLoaded = true;
        }

        if (bgMusicLoaded) {
            SetMusicVolume(bgMusic, 0.15f);  // slightly lower again per user request
            PlayMusicStream(bgMusic);
        }
    }

public:
    AudioManager() {
        bgMusicLoaded = false;
        soundEnabled  = true;
    }

    void init() {
        InitAudioDevice();
        loadAllSounds();
        setupBackgroundMusic();
    }

    void update() {
        // must be called every frame to keep music streaming
        if (bgMusicLoaded) {
            UpdateMusicStream(bgMusic);
        }
    }

    void unload() {
        UnloadSound(cannonSound);
        UnloadSound(sniperSound);
        UnloadSound(gameOverSound);
        UnloadSound(victorySound);
        UnloadSound(waveStartSound);
        UnloadSound(upgradeSound);
        UnloadSound(enemyDieSound);
        if (bgMusicLoaded) {
            UnloadMusicStream(bgMusic);
        }
        CloseAudioDevice();
    }

    // -------------------------------------------------------
    // Play functions
    // -------------------------------------------------------
    void playCannon() {
        if (!soundEnabled) return;
        // only play if not already playing - prevents MachineGun rapid fire from stacking into continuous noise
        if (!IsSoundPlaying(cannonSound)) {
            SetSoundVolume(cannonSound, 0.20f);  // lowered further
            PlaySound(cannonSound);
        }
    }
    void playSniper() {
        if (!soundEnabled) return;
        if (!IsSoundPlaying(sniperSound)) {
            SetSoundVolume(sniperSound, 0.22f);  // lowered further
            PlaySound(sniperSound);
        }
    }
    void playVictory()   { if (soundEnabled) PlaySound(victorySound);    }
    void playGameOver()  { if (soundEnabled) PlaySound(gameOverSound);   }
    void playWaveStart() { if (soundEnabled) PlaySound(waveStartSound);  }
    void playUpgrade()   { if (soundEnabled) PlaySound(upgradeSound);    }
    void playEnemyDie()  { if (soundEnabled) PlaySound(enemyDieSound);   }

    void toggleSound() { soundEnabled = !soundEnabled; }
    bool isSoundOn()   { return soundEnabled; }

    bool isBgMusicLoaded() { return bgMusicLoaded; }

    // set bg music volume (0.0 - 1.0)
    void setBgVolume(float v) {
        if (bgMusicLoaded) SetMusicVolume(bgMusic, v);
    }
};

#endif
