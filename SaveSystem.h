#ifndef SAVESYSTEM_H
#define SAVESYSTEM_H

#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

// holds one high score entry
struct ScoreEntry {
    int score;
    int wave;
    int lives;

    ScoreEntry(int s, int w, int l) {
        score = s;
        wave = w;
        lives = l;
    }
    ScoreEntry() { score = 0; wave = 0; lives = 0; }
};

// holds a mid-game save snapshot
struct GameSaveData {
    bool hasSave;
    int gold;
    int lives;
    int score;
    int currentWave;
    bool waitingForNextWave;

    // tower saves: type, col, row, upgradeLevel
    struct TowerSave {
        int type;        // 0=cannon 1=sniper 2=machinegun 3=slow 4=frost
        int col, row;
        int upgradeLevel;
    };
    std::vector<TowerSave> towers;

    GameSaveData() {
        hasSave = false;
        gold = 0; lives = 0; score = 0;
        currentWave = 0;
        waitingForNextWave = true;
    }
};

// SaveSystem class - handles reading and writing save files
class SaveSystem {
private:
    std::string saveFile;
    std::string highScoreFile;

public:
    SaveSystem() {
        saveFile      = "savegame.dat";
        highScoreFile = "highscores.dat";
    }

    // save mid-game state to file
    void saveGame(GameSaveData& data) {
        std::ofstream f(saveFile, std::ios::binary);
        if (!f.is_open()) return;

        bool has = true;
        f.write((char*)&has, sizeof(bool));
        f.write((char*)&data.gold, sizeof(int));
        f.write((char*)&data.lives, sizeof(int));
        f.write((char*)&data.score, sizeof(int));
        f.write((char*)&data.currentWave, sizeof(int));
        f.write((char*)&data.waitingForNextWave, sizeof(bool));

        int numTowers = (int)data.towers.size();
        f.write((char*)&numTowers, sizeof(int));
        for (int i = 0; i < numTowers; i++) {
            f.write((char*)&data.towers[i], sizeof(GameSaveData::TowerSave));
        }

        f.close();
    }

    // load mid-game state from file
    // returns false if no save exists
    bool loadGame(GameSaveData& data) {
        std::ifstream f(saveFile, std::ios::binary);
        if (!f.is_open()) {
            data.hasSave = false;
            return false;
        }

        bool has = false;
        f.read((char*)&has, sizeof(bool));
        if (!has) {
            data.hasSave = false;
            f.close();
            return false;
        }

        data.hasSave = true;
        f.read((char*)&data.gold, sizeof(int));
        f.read((char*)&data.lives, sizeof(int));
        f.read((char*)&data.score, sizeof(int));
        f.read((char*)&data.currentWave, sizeof(int));
        f.read((char*)&data.waitingForNextWave, sizeof(bool));

        int numTowers = 0;
        f.read((char*)&numTowers, sizeof(int));
        data.towers.clear();
        for (int i = 0; i < numTowers; i++) {
            GameSaveData::TowerSave ts;
            f.read((char*)&ts, sizeof(GameSaveData::TowerSave));
            data.towers.push_back(ts);
        }

        f.close();
        return true;
    }

    // delete the save file (called on game over or win)
    void deleteSave() {
        std::remove(saveFile.c_str());
    }

    bool hasSaveFile() {
        std::ifstream f(saveFile, std::ios::binary);
        if (!f.is_open()) return false;
        bool has = false;
        f.read((char*)&has, sizeof(bool));
        f.close();
        return has;
    }

    // save a high score entry
    void addHighScore(int score, int wave, int lives) {
        std::vector<ScoreEntry> scores = loadHighScores();
        scores.push_back(ScoreEntry(score, wave, lives));

        // sort descending by score
        for (int i = 0; i < (int)scores.size() - 1; i++) {
            for (int j = i + 1; j < (int)scores.size(); j++) {
                if (scores[j].score > scores[i].score) {
                    ScoreEntry tmp = scores[i];
                    scores[i] = scores[j];
                    scores[j] = tmp;
                }
            }
        }

        // keep only top 10
        if ((int)scores.size() > 10) {
            scores.resize(10);
        }

        // write back
        std::ofstream f(highScoreFile, std::ios::binary);
        if (!f.is_open()) return;

        int count = (int)scores.size();
        f.write((char*)&count, sizeof(int));
        for (int i = 0; i < count; i++) {
            f.write((char*)&scores[i], sizeof(ScoreEntry));
        }
        f.close();
    }

    std::vector<ScoreEntry> loadHighScores() {
        std::vector<ScoreEntry> scores;
        std::ifstream f(highScoreFile, std::ios::binary);
        if (!f.is_open()) return scores;

        int count = 0;
        f.read((char*)&count, sizeof(int));
        for (int i = 0; i < count; i++) {
            ScoreEntry e;
            f.read((char*)&e, sizeof(ScoreEntry));
            scores.push_back(e);
        }
        f.close();
        return scores;
    }
};

#endif
