# Tower Defense Game

A simple tower defense game using concepts of OOP

A 2D tower defense game built in C++ using the Raylib graphics library. The game features multiple tower types, enemy waves, sprite-based animations, audio effects, high scores, and a save system.

## Features

* Multiple enemy types with unique behaviors
* Different tower classes including:

  * Cannon Tower
  * Sniper Tower
  * Machine Gun Tower
  * Frost Tower
* Sprite-based animations and effects
* Wave management system
* Gold, score, and lives system
* Upgradeable towers
* High score saving system
* Auto-save and load functionality
* Sound effects and background music
* Pause menu and game states

## Technologies Used

* **Language:** C++
* **Graphics Library:** Raylib
* **IDE:** Visual Studio

## Project Structure

```text
TowerDefense_WithSprites/
│
├── src/                 # Source code files
├── include/             # Raylib header files
├── lib/                 # Raylib library files
├── sprites/             # Game sprites and textures
├── bin/Debug/           # Compiled executable and assets
├── *.mp3                # Audio files
├── TowerDefense.sln     # Visual Studio solution
└── TowerDefense.vcxproj # Visual Studio project
```

## Main Components

### Game System

Handles:

* Game loop
* Rendering
* Input handling
* Game states
* Tower placement
* Enemy updates
* Collision and projectile logic

### Enemy System

Includes:

* Basic enemies
* Fast enemies
* Flying enemies
* Large enemies
* Split enemy behavior

### Tower System

Supports:

* Different attack styles
* Range and damage handling
* Upgrade mechanics
* Projectile spawning

### Wave Manager

Controls:

* Enemy spawning
* Wave progression
* Difficulty scaling

### Save System

Provides:

* Auto-save during gameplay
* Load previous sessions
* High score tracking

## Controls

| Key / Mouse | Action                |
| ----------- | --------------------- |
| Left Click  | Place or select tower |
| ESC         | Pause game            |
| Mouse Hover | Preview placement     |

## How to Run

### Option 1: Run Existing Executable

1. Open:

```text
bin/Debug/
```

2. Run:

```text
TowerDefense.exe
```

Make sure all sprite and audio files remain in their original folders.

### Option 2: Build in Visual Studio

1. Open `TowerDefense.sln`
2. Build the solution
3. Run the project

## Dependencies

This project uses:

* Raylib
* raylib.dll
* Visual C++ Build Tools

## Assets

The project includes:

* Sprite sheets for enemies and towers
* Custom audio effects
* Background music
* Character-based visual assets

## Future Improvements

Possible future updates:

* More tower types
* Boss enemies
* Additional maps
* Better UI animations
* Multiplayer support
* Tower abilities and skills

## Author

Created by Muhammad Fayzan Khalid.

