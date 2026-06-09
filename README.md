<div align="center">

# ProtoDungeonSW2

**An isometric dungeon crawler prototype showcasing Nintendo Switch 2 dock/handheld mechanics**

*Built in Unreal Engine 5.7 source build*

[![Engine](https://img.shields.io/badge/Unreal%20Engine-5.7-313131?logo=unrealengine)](https://www.unrealengine.com/)
[![Language](https://img.shields.io/badge/C%2B%2B-20-00599C?logo=cplusplus)](https://en.cppreference.com/)
[![Platform](https://img.shields.io/badge/Platform-Nintendo%20Switch%202-E60012?logo=nintendoswitch)](https://www.nintendo.com)
[![Status](https://img.shields.io/badge/Status-Prototype%20Complete-success)]()
[![License](https://img.shields.io/badge/License-Proprietary-lightgrey)]()

[Overview](#overview) • [Features](#features) • [Quick start](#quick-start) • [Architecture](#architecture) • [Controls](#controls) • [Documentation](#documentation)

</div>

---

## Overview

ProtoDungeonSW2 is a **two-level prototype** designed as a technical showcase of Nintendo Switch 2's hallmark mechanic: the seamless transition between **Docked** and **Handheld** mode. The prototype demonstrates how a single game can adapt its camera, control scheme, gameplay pawn, and rendering budget on the fly when the player physically detaches or reattaches the JoyCons.

The game is a small dungeon crawler with melee combat and a marble puzzle, built with production-quality architecture in **C++** for systems and **Blueprint** for UI configuration.

### The pitch in one paragraph

> Walk into a dungeon as a chibi humanoid. Fight an orc with rhythmic melee combat — basic strikes on left-click, a special attack triggered by *shaking the JoyCon*. Push through to a second room with a closed door. Detach the JoyCons (or hit M on PC) and the world transforms: the camera flips top-down, you take control of a marble, and you tilt the JoyCons to roll it through a maze. Drop the marble in the hole, the chest appears, grab the key, walk through the door. The end.

---

## Features

<table>
<tr>
<td width="50%" valign="top">

### 🎮 Gameplay

- **Melee combat** with directional hit reactions and animation-synced damage
- **JoyCon shake special attack** with custom shake detector
- **Gyroscope-driven puzzle** marble with manual physics
- **Two camera modes**: isometric for combat, top-down for puzzle
- **Game Over + Retry** with cinematic death animation
- **Level-to-level transitions** with cinematic fade

</td>
<td width="50%" valign="top">

### ⚙️ Technical

- **Platform Mode subsystem** auto-detects Docked/Handheld
- **Auto-scalability** adjusts quality per mode (Medium / Epic)
- **Substrate auto-fade walls** with SphereMask
- **Lumen** dynamic global illumination
- **Behavior Tree AI** with Blackboard + custom Tasks/Services
- **Switch 2 SDK-ready** code with `#if PLATFORM_SWITCH` placeholders

</td>
</tr>
<tr>
<td width="50%" valign="top">

### 🏗️ Architecture

- **5 GameInstanceSubsystems** as source-of-truth for global state
- **Polymorphic interactable** base class for chests/doors/keys
- **One-shot animation pattern** with FTimerHandle synchronization
- **Decoupled HUD message system**
- **Single input source** for global actions (toggle mode)

</td>
<td width="50%" valign="top">

### 🎨 Visuals & audio

- **Kenney mini-dungeon** asset pack throughout
- **Cinematic dungeon lighting** with dynamic torches
- **Sound Cues** with Random Without Replacement for variety
- **UMG-based HUD** with Cinzel + Montserrat typography
- **Animated JoyCon shake icon** as input feedback
- **End-game screen** with custom amber/coral palette

</td>
</tr>
</table>

---

## Screenshots

> *(Screenshots placeholder — add `Docs/Images/` with captures of L_Combat, L_Puzzle, and L_Victory before publishing.)*

```
┌─────────────────────────────────┐  ┌─────────────────────────────────┐
│                                 │  │                                 │
│   L_Combat — isometric, orc     │  │   L_Puzzle — top-down marble    │
│                                 │  │                                 │
└─────────────────────────────────┘  └─────────────────────────────────┘

┌─────────────────────────────────┐  ┌─────────────────────────────────┐
│                                 │  │                                 │
│   Mode toggle HUD message       │  │   L_Victory — end screen        │
│                                 │  │                                 │
└─────────────────────────────────┘  └─────────────────────────────────┘
```

---

## Quick start

### Requirements

- **Windows 10/11** with **Visual Studio 2022** (workloads: *Game development with C++*, *Desktop development with C++*).
- **Unreal Engine 5.7 source build** at `C:\UE57` (or update paths accordingly).
- **Git** with **Git LFS** installed and configured.
- **~30 GB** free disk space for the engine + project + Derived Data Cache.

### Installation

```powershell
# 1. Clone the repo (Git LFS will fetch binary assets automatically)
git clone git@github.com:ManuMendezMPG/ProtoDungeonSW2.git C:\dev\ProtoDungeonSW2
cd C:\dev\ProtoDungeonSW2

# 2. Generate Visual Studio project files
& "C:\UE57\Engine\Build\BatchFiles\GenerateProjectFiles.bat" `
    -project="C:\dev\ProtoDungeonSW2\ProtoDungeonSW2.uproject" -game

# 3. Open the solution
start ProtoDungeonSW2.sln

# 4. In Visual Studio: set configuration to "Development Editor | Win64" and Build → Build Solution
```

After the build succeeds, double-click `ProtoDungeonSW2.uproject` to launch the editor.

### Playing the game

1. Open `Content/Maps/L_Combat.umap`.
2. Hit **Play** in the editor.
3. Use **WASD** to move, **left-click** to attack the orc, **right-click + shake the mouse** for the special.
4. After defeating the orc, the level transitions to `L_Puzzle`.
5. Hit **M** to toggle Handheld mode. The camera flips top-down and you control the marble.
6. Roll the marble into the hole.
7. Back in the room, press **E** on the chest, then **E** on the door.
8. Enjoy `L_Victory`.

---

## Controls

| Action | PC | Switch 2 (planned) |
|---|---|---|
| Move | <kbd>W</kbd> <kbd>A</kbd> <kbd>S</kbd> <kbd>D</kbd> | Left stick |
| Basic attack | <kbd>Left Click</kbd> | <kbd>A</kbd> |
| Special attack | <kbd>Right Click</kbd> + shake mouse | Shake JoyCon (real gyro) |
| Interact | <kbd>E</kbd> | <kbd>A</kbd> in range |
| Toggle Docked/Handheld | <kbd>M</kbd> | Detach / dock JoyCons (automatic) |
| Tilt marble (puzzle mode) | Mouse movement | JoyCon gyroscope |

---

## Architecture

### High-level diagram

```
                          ┌─────────────────────────────┐
                          │   UDPPlatformModeSubsystem  │
                          │   (source of truth)         │
                          └──────────────┬──────────────┘
                                         │ OnPlatformModeChanged
            ┌────────────────────────────┼────────────────────────────┐
            │                            │                            │
            ▼                            ▼                            ▼
   ┌────────────────┐         ┌──────────────────┐         ┌──────────────────┐
   │ Scalability    │         │  PuzzleCtrl      │         │  HUD Message     │
   │ (perf budget)  │         │  (camera/pawn)   │         │  ("Handheld...") │
   └────────────────┘         └──────────────────┘         └──────────────────┘

   ┌─────────────────────────────────────────────────────────────────────┐
   │                          C++ subsystems                             │
   ├─────────────────────────────────────────────────────────────────────┤
   │  PlatformMode  PuzzleState  Message  LevelTransition  Shake  Gyro   │
   └─────────────────────────────────────────────────────────────────────┘
   ┌─────────────────────────────────────────────────────────────────────┐
   │                          Gameplay actors                            │
   ├─────────────────────────────────────────────────────────────────────┤
   │  CharacterBase ─► PlayerCharacter   ┌── InteractableBase            │
   │                ─► EnemyBase         │  ├── PuzzleKey                │
   │  PuzzleBall (custom physics)        │  ├── PuzzleChest              │
   │                                     │  └── PuzzleDoor               │
   │  HoleTrigger (resets to Docked)     │                               │
   └─────────────────────────────────────────────────────────────────────┘
```

### Folder structure

```
ProtoDungeonSW2/
├── Source/ProtoDungeonSW2/
│   ├── AI/             # EnemyAIController, BTTasks, BTServices
│   ├── Characters/     # CharacterBase, PlayerCharacter, EnemyBase, PuzzleBall
│   ├── Combat/         # CombatComponent
│   ├── GameModes/      # GameModes + LevelTransitionSubsystem
│   ├── Input/          # ShakeDetector, GyroInput, PlatformMode
│   ├── Puzzle/         # InteractableBase, Key, Chest, Door, HoleTrigger,
│   │                   #   PuzzleStateSubsystem, PuzzlePlayerController
│   └── UI/             # MessageSubsystem
├── Content/
│   ├── AI/             # Blackboard + Behavior Tree
│   ├── Audio/          # SoundCues + raw Sound Waves
│   ├── Blueprints/     # BP child classes for C++ types
│   ├── Input/          # Input Actions + Mapping Contexts (Enhanced Input)
│   ├── Kenney/         # Imported mini-dungeon GLB pack
│   ├── Maps/           # L_Combat, L_Puzzle, L_Victory
│   ├── Materials/      # FadeOutWall + variants
│   ├── MPC/            # MPC_Global (PlayerPosition vector)
│   └── UI/             # Widgets, fonts, textures
├── CLAUDE.md           # Project conventions (read this if working with AI)
└── ProtoDungeonSW2.uproject
```

### Conventions

| Aspect | Rule |
|---|---|
| Project class prefix | `DP` (e.g., `ADPPlayerCharacter`) |
| UE standard prefixes | `A` actors, `U` UObjects, `F` structs, `E` enums |
| Comment language | English (after final translation pass) |
| Identifier language | English |
| UObject pointers | `TObjectPtr<T>` always, never raw `T*` |
| Cross-folder includes | Relative path with `../` |
| Commits | Conventional Commits (`feat:`, `fix:`, `refactor:`, `docs:`) |

Full conventions live in [`CLAUDE.md`](./CLAUDE.md).

---

## Project status

| Phase | Status |
|---|---|
| Architecture & systems | ✅ Complete |
| Combat (L_Combat) | ✅ Complete |
| Puzzle (L_Puzzle) | ✅ Complete |
| Victory screen (L_Victory) | ✅ Complete |
| Game Over & Retry | ✅ Complete |
| Kenney assets migration | ✅ Complete |
| Platform Mode + auto-scalability | ✅ Complete |
| HUD with shake feedback | ✅ Complete |
| Switch 2 SDK placeholders | ✅ Complete (TODO comments in place) |
| Switch 2 devkit deployment | ⏳ Pending |
| JoyCon rumble support | ⏳ Pending |
| Real JoyCon icons in HUD | ⏳ Pending |

---

## Documentation

Comprehensive documentation lives in this repo:

- **[Technical Guide (English)](./Docs/ProtoDungeonSW2_Technical_Guide.md)** — full architecture, code walkthrough, design patterns, 16 documented bugs with fixes, Switch 2 deployment roadmap.
- **[Guía Técnica (Español)](./Docs/ProtoDungeonSW2_Guia_Tecnica.md)** — same content in Spanish.
- **[CLAUDE.md](./CLAUDE.md)** — project conventions for AI-assisted development.

The Technical Guide is the **single source of truth** for the project. Read it before working on the codebase if you didn't write it yourself.

### Key reusable design patterns

The guide documents five patterns we extracted from this project that are worth carrying to future UE5 projects:

1. **One-shot animation actor with state change** — `PlayAnimation` + `FTimerHandle` for chests, doors, deaths.
2. **Subsystem as source of truth** — UGameInstanceSubsystem + delegate broadcast for global state.
3. **Polymorphic interactable base** — single overlap detection, infinite extensibility.
4. **Single input source for global actions** — one IA in IMC_Default, multiple consumers.
5. **Defensive `BeginPlay` for PIE** — force reset InputMode to survive viewport state contamination.

---

## Switch 2 deployment

The codebase is **architecturally ready** for the Nintendo Switch 2 SDK. All platform-specific code is gated behind `#if PLATFORM_SWITCH` with TODO comments referencing the expected API:

```cpp
EDPPlatformMode UDPPlatformModeSubsystem::QueryPlatformMode() const
{
#if PLATFORM_SWITCH
    // TODO Switch 2: replace with the real Nintendo SDK API
    // #include <nn/oe.h>
    // const nn::oe::OperationMode Mode = nn::oe::GetOperationMode();
    // return (Mode == nn::oe::OperationMode_Handheld)
    //     ? EDPPlatformMode::Handheld : EDPPlatformMode::Docked;
    return EDPPlatformMode::Docked;  // Fallback until SDK is available
#else
    return CurrentMode;
#endif
}
```

When the devkit is available, **only three changes** are needed to activate real detection:

1. Uncomment the SDK API call in `UDPPlatformModeSubsystem::QueryPlatformMode()`.
2. Hook the real JoyCon accelerometer/gyro into `UDPShakeDetectorSubsystem::FeedInputDelta` and `UDPGyroInputSubsystem::FeedTiltDelta`.
3. Run the BuildCookRun pipeline targeting `-platform=Switch`.

Everything else (camera switching, scalability adjustment, HUD feedback, marble physics, AI behavior) is platform-agnostic and runs untouched. See section 8 of the [Technical Guide](./Docs/ProtoDungeonSW2_Technical_Guide.md#8-switch-2-roadmap-deploy) for the full deployment checklist.

---

## Tech stack

| Layer | Technology |
|---|---|
| Engine | Unreal Engine 5.7 (source) |
| Language | C++20 + Blueprint |
| Rendering | Lumen (dynamic GI), Substrate materials |
| AI | Behavior Trees + Blackboard |
| Input | Enhanced Input |
| UI | UMG |
| Audio | Sound Cues with Random Without Replacement |
| Source control | Git + Git LFS |
| IDE | Visual Studio 2022 |
| AI-assisted dev | Claude (chat) + Claude Code (terminal) |

---

## Credits

### Visual assets

- **[Kenney Mini Dungeon Pack](https://kenney.nl/assets/mini-dungeon)** by Kenney Vleugels — characters, props, dungeon pieces (CC0).

### Fonts

- **[Cinzel](https://fonts.google.com/specimen/Cinzel)** by Natanael Gama — title typography (SIL Open Font License).
- **[Montserrat](https://fonts.google.com/specimen/Montserrat)** by Julieta Ulanovsky — body typography (SIL Open Font License).

### Engine & tools

- **[Unreal Engine](https://www.unrealengine.com/)** by Epic Games.
- **[Claude](https://www.anthropic.com)** by Anthropic — AI pair-programming throughout the entire project.

### Author

**Manuel Méndez** ([@ManuMendezMPG](https://github.com/ManuMendezMPG))

---

## License

Proprietary. All rights reserved.

The Kenney assets are CC0 — see the Kenney pack license for redistribution rights.

---

<div align="center">

*"The sphere rolled, the door opened, and somewhere a Joy-Con was shaken in vain."*

— `WBP_Victory`

</div>
