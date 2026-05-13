#ifndef WAVEMANAGER_H
#define WAVEMANAGER_H

#include "Enemy.h"
#include "EnemyTypes.h"
#include <vector>
#include <string>

#define TILE_SIZE 48

struct SpawnEntry {
    int type;
    float delay;
};

class WaveManager {
private:
    int currentWave;
    int totalWaves;
    float spawnTimer;
    int spawnIndex;
    bool waveActive;
    bool allWavesDone;
    std::vector<SpawnEntry> currentSpawnList;

    void addEntry(int type, float delay) {
        SpawnEntry e;
        e.type  = type;
        e.delay = delay;
        currentSpawnList.push_back(e);
    }

    void buildWave(int waveNum) {
        currentSpawnList.clear();
        spawnIndex = 0;
        spawnTimer = 0.0f;

        if (waveNum == 1) {
            // Wave 1 - easy intro, 6 basic enemies
            for (int i = 0; i < 6; i++) addEntry(0, i * 1.8f);
        }
        else if (waveNum == 2) {
            // Wave 2 - 5 basics then 4 fast
            for (int i = 0; i < 5; i++) addEntry(0, i * 1.4f);
            for (int i = 0; i < 4; i++) addEntry(1, 8.0f + i * 1.2f);
        }
        else if (waveNum == 3) {
            // Wave 3 - tanks introduced, total 11 enemies
            for (int i = 0; i < 4; i++) addEntry(0, i * 1.2f);
            for (int i = 0; i < 3; i++) addEntry(2, 5.0f + i * 4.0f);
            for (int i = 0; i < 4; i++) addEntry(1, 17.0f + i * 1.0f);
        }
        else if (waveNum == 4) {
            // Wave 4 - DOUBLED: 20 spawns (was 10) - hard mode!
            // splitters count as 3 enemies total when they split
            float t = 0;
            // First batch
            addEntry(3, t); t += 2.5f;
            addEntry(1, t); t += 1.5f;
            addEntry(3, t); t += 2.5f;
            addEntry(4, t); t += 3.5f;
            addEntry(2, t); t += 4.0f;
            addEntry(1, t); t += 1.5f;
            addEntry(4, t); t += 3.5f;
            addEntry(3, t); t += 2.5f;
            addEntry(2, t); t += 4.0f;
            addEntry(4, t); t += 3.5f;
            // Second batch - doubled enemies
            addEntry(3, t); t += 2.5f;
            addEntry(1, t); t += 1.5f;
            addEntry(3, t); t += 2.5f;
            addEntry(4, t); t += 3.5f;
            addEntry(2, t); t += 4.0f;
            addEntry(1, t); t += 1.5f;
            addEntry(4, t); t += 3.5f;
            addEntry(3, t); t += 2.5f;
            addEntry(2, t); t += 4.0f;
            addEntry(4, t);
        }
        else if (waveNum == 5) {
            // Wave 5 - DOUBLED: 28 spawns (was 14) - ultimate challenge!
            float t = 0;
            // First batch - fast rush
            for (int i = 0; i < 3; i++) { addEntry(1, t); t += 1.0f; }
            addEntry(2, t); t += 4.0f;
            for (int i = 0; i < 2; i++) { addEntry(3, t); t += 2.0f; }
            addEntry(4, t); t += 3.0f;
            addEntry(4, t); t += 3.5f;
            addEntry(2, t); t += 4.0f;
            addEntry(1, t); t += 1.0f;
            addEntry(0, t); t += 1.0f;
            addEntry(3, t); t += 2.0f;
            addEntry(1, t); t += 1.0f;
            addEntry(0, t); t += 1.2f;
            addEntry(2, t); t += 4.0f;
            // Second batch - doubled enemies, more intense
            for (int i = 0; i < 3; i++) { addEntry(1, t); t += 0.8f; }
            addEntry(2, t); t += 3.5f;
            for (int i = 0; i < 2; i++) { addEntry(3, t); t += 1.8f; }
            addEntry(4, t); t += 2.5f;
            addEntry(4, t); t += 3.0f;
            addEntry(2, t); t += 3.5f;
            addEntry(1, t); t += 0.8f;
            addEntry(0, t); t += 0.8f;
            addEntry(3, t); t += 1.8f;
            addEntry(1, t); t += 0.8f;
            addEntry(0, t); t += 1.0f;
            addEntry(2, t);
        }
    }

public:
    WaveManager() {
        currentWave  = 0;
        totalWaves   = 5;
        spawnTimer   = 0.0f;
        spawnIndex   = 0;
        waveActive   = false;
        allWavesDone = false;
    }

    void startNextWave() {
        if (allWavesDone) return;
        currentWave++;
        if (currentWave > totalWaves) {
            allWavesDone = true;
            waveActive   = false;
            return;
        }
        buildWave(currentWave);
        waveActive = true;
    }

    void setWave(int wave) {
        currentWave = wave;
        if (currentWave >= totalWaves) allWavesDone = true;
    }

    std::vector<Enemy*> update(float deltaTime, std::vector<std::pair<int,int>>& path) {
        std::vector<Enemy*> newEnemies;
        if (!waveActive) return newEnemies;

        spawnTimer += deltaTime;

        while (spawnIndex < (int)currentSpawnList.size()) {
            SpawnEntry& entry = currentSpawnList[spawnIndex];
            if (spawnTimer < entry.delay) break;

            float startX = path[0].first  * TILE_SIZE + TILE_SIZE / 2.0f;
            float startY = path[0].second * TILE_SIZE + TILE_SIZE / 2.0f;

            Enemy* e = nullptr;
            if      (entry.type == 0) e = new BasicEnemy(startX, startY);
            else if (entry.type == 1) e = new FastEnemy(startX, startY);
            else if (entry.type == 2) e = new TankEnemy(startX, startY);
            else if (entry.type == 3) {
                FlyingEnemy* fe = new FlyingEnemy(startX, startY);
                int last = (int)path.size() - 1;
                fe->setTarget(path[last].first  * TILE_SIZE + TILE_SIZE / 2.0f,
                              path[last].second * TILE_SIZE + TILE_SIZE / 2.0f);
                e = fe;
            }
            else if (entry.type == 4) e = new SplitterEnemy(startX, startY);

            if (e) newEnemies.push_back(e);
            spawnIndex++;
        }

        if (spawnIndex >= (int)currentSpawnList.size()) waveActive = false;

        return newEnemies;
    }

    int  getCurrentWave() { return currentWave;  }
    int  getTotalWaves()  { return totalWaves;    }
    bool isWaveActive()   { return waveActive;    }
    bool isAllDone()      { return allWavesDone;  }
};

#endif
