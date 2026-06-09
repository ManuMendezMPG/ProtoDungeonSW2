# ProtoDungeonSW2 — Technical Guide

> Isometric dungeon crawler prototype for **Nintendo Switch 2**, developed in Unreal Engine 5.7 source build. Combines melee combat with a marble puzzle controlled by simulated gyroscope, all in two levels that showcase the switch between **Docked** and **Handheld** mode.

---

## Table of contents

1. [Overview](#1-overview)
2. [Project setup](#2-project-setup)
3. [Levels](#3-levels)
4. [C++ systems](#4-c-systems)
5. [Blueprint systems and assets](#5-blueprint-systems-and-assets)
6. [Reusable design patterns](#6-reusable-design-patterns)
7. [Documented bugs and fixes](#7-documented-bugs-and-fixes)
8. [Switch 2 roadmap (deploy)](#8-switch-2-roadmap-deploy)
9. [Workflow and philosophy](#9-workflow-and-philosophy)
10. [Quick reference](#10-quick-reference)

---

## 1. Overview

### Concept

ProtoDungeonSW2 is a **two-level** prototype designed as a showcase of Switch 2 hallmark mechanics:

- **L_Combat**: classic dungeon with melee combat. A humanoid player faces an orc. Basic attack is bound to left-click; the special attack triggers when the JoyCon is shaken (simulated on PC with right-click + mouse movement).
- **L_Puzzle**: the player enters a room with a top-down maze. Switching to Handheld mode (M key on PC, or physically detaching the JoyCons on Switch 2) flips the camera to a top-down angle and hands control to a marble that rolls via the gyroscope. Solving the puzzle reveals a chest and then a door leading to victory.
- **L_Victory**: end-game screen with credits.

### Technology stack

| Component | Version / Details |
|---|---|
| Engine | Unreal Engine 5.7 (source build at `C:\UE57`) |
| IDE | Visual Studio 2022 |
| Languages | C++ (systems) + Blueprint (UI and configuration) |
| Repository | GitHub with Git LFS for `.uasset` and `.umap` |
| Dev platform | Windows 10/11 |
| Target platform | Nintendo Switch 2 (devkit) |
| Visual assets | Kenney mini-dungeon pack (GLB) |
| AI assistant | Claude (chat for design + Claude Code for multi-file C++) |

### Final scope

```
✅ Playable character with isometric camera + Enhanced Input
✅ Melee combat with animations, directional hit reactions, and cooldowns
✅ Special attack triggered by shake (mouse on PC, gyroscope on Switch)
✅ Shake-feedback HUD (animated JoyCon icon during the special)
✅ Orc enemy driven by Behavior Tree AI
✅ Auto-fade walls with Substrate material
✅ Dungeon lighting with Lumen + dynamic torches
✅ Audio with random selection in Sound Cues
✅ Platform Mode system (Docked/Handheld) with auto-detection ready for SDK
✅ Camera + pawn swap when toggling mode (marble mode in L_Puzzle)
✅ Automatic scalability (Medium in Handheld, Epic in Docked)
✅ On-screen message when changing mode
✅ Puzzle with manually-simulated ball physics and wall sliding
✅ One-shot animated chest (PlayAnimation)
✅ One-shot animated door with key gate
✅ Decoupled HUD message system
✅ Game Over with death animation + Retry
✅ Level transitions with fade
✅ Full migration to Kenney assets (player, orc, props, levels)
```

---

## 2. Project setup

### Requirements

- **Unreal Engine 5.7** built from source. Default location: `C:\UE57`.
- **Visual Studio 2022** with the "Game development with C++" and "Desktop development with C++" workloads.
- **Git** + **Git LFS** configured for `.uasset`, `.umap`, and binary files.
- **GitHub CLI or SSH key** for private repository access.

### Folder structure

```
C:\dev\ProtoDungeonSW2\
├── Config\                     # Project .ini files
├── Content\
│   ├── AI\                     # BB_Enemy, BT_Enemy
│   ├── Audio\
│   │   ├── Grunt\              # S_Grunt_01, _02
│   │   ├── Hit\                # S_Hit_01..04
│   │   └── Swoosh\             # S_Swing_01, _02
│   ├── Blueprints\
│   │   ├── AI\                 # BP_EnemyAIController
│   │   ├── Characters\         # BP_PlayerCharacter, BP_PlayerCharacter_Kenney, etc.
│   │   ├── GameModes\          # BP_GameMode_Kenney, BP_GameModePuzzle_Kenney
│   │   └── Puzzle\             # BP_PuzzleChest_Kenney, BP_PuzzleDoor, BP_PuzzleKey
│   ├── Input\                  # IA_*, IMC_Default
│   ├── Kenney\
│   │   └── MiniDungeon\
│   │       ├── Meshes\         # character-human, character-orc, chest, gate, wall, etc.
│   │       └── Montages\       # AM_Attack_Basic_Kenney, AM_Die_Kenney, etc.
│   ├── Maps\
│   │   ├── L_Combat.umap
│   │   ├── L_Puzzle.umap
│   │   ├── L_Victory.umap
│   │   └── _Backups\
│   ├── Materials\              # M_FadeOutWall, M_KenneyFadeWall + Material Instances
│   ├── MPC\                    # MPC_Global with PlayerPosition
│   └── UI\
│       ├── Fonts\              # Cinzel, Montserrat
│       ├── Textures\           # T_JoyCon_Shake
│       ├── WBP_PlayerHUD
│       ├── WBP_GameOver
│       └── WBP_Victory
├── Source\
│   └── ProtoDungeonSW2\
│       ├── AI\                 # DPEnemyAIController, DPBTTask_AttackTarget, DPBTService_UpdateTarget
│       ├── Characters\         # DPCharacterBase, DPPlayerCharacter, DPEnemyBase, DPPuzzleBall
│       ├── Combat\             # DPCombatComponent
│       ├── GameModes\          # DPGameModeBase, DPGameModePuzzle, DPLevelTransitionSubsystem
│       ├── Input\              # DPShakeDetectorSubsystem, DPGyroInputSubsystem, DPPlatformModeSubsystem
│       ├── Puzzle\             # DPInteractableBase, DPPuzzleKey, DPPuzzleChest, DPPuzzleDoor, DPHoleTrigger, DPPuzzleStateSubsystem, DPPuzzlePlayerController
│       ├── UI\                 # DPMessageSubsystem
│       ├── ProtoDungeonSW2.Build.cs
│       └── ProtoDungeonSW2.Target.cs
├── ProtoDungeonSW2.uproject
└── CLAUDE.md
```

### Build.cs

`Source/ProtoDungeonSW2/ProtoDungeonSW2.Build.cs`:

```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "Core", "CoreUObject", "Engine", "InputCore",
    "EnhancedInput", "AIModule", "GameplayTasks",
    "NavigationSystem", "UMG"
});
```

Key notes:

- **`EnhancedInput`**: required for `UInputAction`, `UInputMappingContext`, `UEnhancedInputComponent`.
- **`AIModule` + `GameplayTasks` + `NavigationSystem`**: for `AAIController`, Behavior Trees, MoveTo.
- **`UMG`**: added after Game Over (required by `TSubclassOf<UUserWidget>` in the player header). Without this, the linker fails with LNK2019 errors on `UUserWidget`.

### Conventions (CLAUDE.md)

The `CLAUDE.md` file at the project root defines conventions:

| Aspect | Rule |
|---|---|
| Project class prefix | `DP` (e.g., `ADPPlayerCharacter`, `UDPCombatComponent`) |
| UE standard prefixes | `A` for Actors, `U` for UObjects/Components, `F` for structs, `E` for enums |
| Comment language | English (after final translation pass) |
| Identifier language | English |
| Editable UPROPERTY | Always with a specific Category |
| UObject pointers | `TObjectPtr<T>`, never raw `T*` |
| Cross-folder includes | Relative path with `../`, e.g., `"../Combat/DPCombatComponent.h"` |
| No shadowing | No parameters named `Instigator`, `Owner`, `Tags`, `Role` (collide with `AActor`/`UObject` members) |
| Commits | Conventional Commits (`feat:`, `fix:`, `refactor:`, `docs:`) |
| Engine | DO NOT modify `C:\UE57` unless explicitly requested |

### Useful commands

```powershell
# Regenerate Visual Studio files after changing Build.cs or adding/removing files
& "C:\UE57\Engine\Build\BatchFiles\GenerateProjectFiles.bat" -project="C:\dev\ProtoDungeonSW2\ProtoDungeonSW2.uproject" -game

# Launch Claude Code in the project
cd C:\dev\ProtoDungeonSW2
claude

# Push changes
git status
git add .
git commit -m "feat(...): description"
git push
```

---

## 3. Levels

### L_Combat

L-shaped room with Kenney walls (entry + corridor + main room). The player spawns in the entry, walks through the corridor to the main room where the orc waits. Killing the orc triggers an automatic transition to L_Puzzle after 2 seconds.

**Technical features:**

- **GameMode**: `BP_GameMode_Kenney` (Default Pawn Class = `BP_PlayerCharacter_Kenney`).
- **NavMesh**: covers the entire walkable area so the orc AI can navigate.
- **Lighting**: subtle DirectionalLight + cold SkyLight + ExponentialHeightFog for atmosphere. Torches (`BP_Torch`) with warm PointLight `(R=1, G=0.6, B=0.2)` create contrast.
- **Auto-fade walls**: front-facing walls (towards the camera) use `MI_Wall_Fade` and variants. They fade out when the player gets close, using `MPC_Global.PlayerPosition` + SphereMask.
- **Decoration**: Kenney banners on walls, barrels, torches, weapons and shields as props.

### L_Puzzle

**Unusual** structure: two sublevels physically separated vertically in Z.

```
                  ┌─────────────────────────┐
                  │  Puzzle maze            │   ← High Z, top-down camera
                  │  (visible only in       │
                  │   Handheld mode with    │
                  │   marble)               │
                  └─────────────────────────┘
                              ↓ vertical separation
                  ┌─────────────────────────┐
                  │  Main Kenney room       │   ← Z = 0, isometric camera
                  │  (entrance + chest +    │
                  │   door)                 │
                  └─────────────────────────┘
```

**Level flow:**

1. The player enters the Kenney room (Docked mode, isometric camera).
2. Sees a closed door and a message saying "you need a key".
3. Presses M → switches to Handheld → `ADPPuzzlePlayerController` possesses the marble, the camera jumps to `PuzzleCamera` (CameraActor positioned top-down over the maze, Yaw -90).
4. Moves the mouse → simulated gyroscope tilts, the marble rolls through the maze. The marble **slides** along walls (doesn't get stuck) thanks to `FVector::VectorPlaneProject` on `Hit.ImpactNormal`.
5. Reaches the hole (`ADPHoleTrigger`) → the marble is destroyed, mode automatically switches back to Docked.
6. Back in the Kenney room, the chest appears.
7. Press E → opening animation → `bPlayerHasKey = true` + "Key obtained" message.
8. Press E on the door → opening animation → after the animation, fade out + transition to L_Victory.

**Dual lighting:**

- Kenney room: warm PointLights on walls (torches).
- Puzzle maze: 4 PointLights with `Cast Shadows: OFF`, light blue, intensity 8000. No shadows to avoid shadowmap artifacts when multiple dynamic lights overlap.

**Critical MPC fade fix:** `BP_PlayerCharacter`'s `Event Tick` writes `GetActorLocation` to the MPC only if `ViewTarget == self`. When the camera is on a different pawn (marble mode), it writes `(99999, 99999, 99999)` so no wall fades falsely.

### L_Victory

Minimal map with a single actor: a `BP_GameMode_Victory` with `DefaultPawnClass = nullptr`. The Level Blueprint creates `WBP_Victory` and adds it to the viewport on BeginPlay.

**Why `DefaultPawnClass = nullptr`:** without this, the PlayerStart causes UE to spawn a pawn, which in some cases dragged the player HUD into the victory screen (residual HUD bug).

**Widget aesthetics:** amber/coral palette on dark brown (`#2A1810`, `#EF9F27`, `#BA7517`, `#FAC775`). Fonts: Cinzel Bold (title) + Montserrat (subtitle/buttons). Tagline: *"The sphere rolled, the door opened, and somewhere a Joy-Con was shaken in vain."*

---

## 4. C++ systems

### 4.1 Characters

#### `ADPCharacterBase` (base class)

Inherits from `ACharacter`. Defines the shared contract for player and enemies.

**State:**
- `MaxHealth` (default 100), `CurrentHealth`, `bIsDead`.
- `DeathAnimation` (`UAnimSequence*`): if assigned, plays in `OnDeath` via `GetMesh()->PlayAnimation(DeathAnimation, false)` and holds the last frame.
- `bTriggersLevelTransitionOnDeath` + `NextLevelName`: if true, on death requests `UDPLevelTransitionSubsystem` to load the next level (used by the final bull in L_Combat).

**Combat:**
- `HitReactFrontMontage`, `HitReactBackMontage`: directional reaction animations based on attacker direction.
- `DamageSound`: grunt played when taking non-lethal damage.
- Override of `TakeDamage` applied to the chain: updates `CurrentHealth`, broadcasts `OnHealthChanged`, plays hit reaction if survives, calls `OnDeath` if reaches 0.

**Hit direction detection:**

```cpp
const FVector ToAttacker = (DamageCauser->GetActorLocation() - GetActorLocation()).GetSafeNormal();
const FVector Forward = GetActorForwardVector();
const float DotProduct = FVector::DotProduct(Forward, ToAttacker);
if (DotProduct < 0.f && HitReactBackMontage != nullptr)
{
    MontageToPlay = HitReactBackMontage;
}
```

**OnDeath (virtual):**
1. Plays `DeathAnimation` (PlayAnimation, not Montage).
2. `Montage_Stop` to cancel in-flight attacks (prevents the orc from hitting you post-death).
3. `SetActorEnableCollision(false)`.
4. `DisableMovement` on the `UCharacterMovementComponent`.
5. If AI-controlled, `Brain->StopLogic("Character died")`.
6. If `bTriggersLevelTransitionOnDeath`, calls the transition subsystem.

#### `ADPPlayerCharacter` (player)

Inherits from `ADPCharacterBase`. Adds fixed isometric camera, Enhanced Input, and the interaction/Game Over system.

**Camera:**
- `SpringArm`: `TargetArmLength = 2000`, absolute rotation `(Pitch=-50, Yaw=-45, Roll=0)`. `bDoCollisionTest = false`, `bUseAbsoluteRotation = true`.
- `Camera`: attached to SpringArm socket. `FieldOfView = 60`.
- `bUseControllerRotationPitch/Yaw/Roll = false`. The character orients towards movement (`bOrientRotationToMovement = true`, `RotationRate = (0, 640, 0)`).

**Movement:**
- `MaxWalkSpeed = 500` (400 in Kenney variant due to chibi scale).
- `Move(FInputActionValue)` projects the 2D input onto the camera yaw so forward/right are consistent with the isometric view.

**Interaction:**
- `InteractionSphere` (USphereComponent, radius 150): `WorldDynamic` + `Overlap WorldDynamic`. Keeps a `CurrentInteractable` pointer to the latest `ADPInteractableBase` that enters range.
- On E press (`IA_Interact`), if `CurrentInteractable != nullptr`, calls `CurrentInteractable->Interact(this)` polymorphically.

**Game Over:**
- `GameOverWidgetClass` (`TSubclassOf<UUserWidget>`): set to `WBP_GameOver` in the BP.
- `OnDeath` override: `Super::OnDeath()` → `DisableInput(PC)` → `SetTimer(GameOverTimerHandle, ShowGameOverScreen, DeathAnimation->GetPlayLength())`.
- `ShowGameOverScreen`: creates the widget, adds to viewport, `SetInputModeUIOnly` + `SetShowMouseCursor(true)` + `SetGamePaused(true)`.

**Defensive `BeginPlay` input reset:**

```cpp
if (APlayerController* PC = Cast<APlayerController>(GetController()))
{
    FInputModeGameOnly InputMode;
    PC->SetInputMode(InputMode);
    PC->SetShowMouseCursor(false);
    EnableInput(PC);
}
```

This prevents a PIE viewport bug: after a Retry, the viewport's InputMode remains contaminated as `UIOnly` and the new player won't receive input. Defensive on every spawn.

#### `ADPEnemyBase`

Inherits from `ADPCharacterBase`. Configures:
- `AIControllerClass = ADPEnemyAIController::StaticClass()`.
- `bUseControllerRotationYaw = true` so the AI can rotate the pawn while chasing.
- `MaxWalkSpeed = 350` (orc slightly slower than the player).
- Combat component with `BasicAttackMontage` set to `AM_Attack_Basic_Orc`.

### 4.2 Combat

#### `UDPCombatComponent`

`UActorComponent` that adds combat to any `ADPCharacterBase`. Both the player and the orc use it.

**Exposed configuration:**
- `BasicAttackDamage` (25), `SpecialAttackDamage` (50).
- `BasicAttackRange` (80), `SpecialAttackRange` (95) — distance from the actor to the center of the damage sphere.
- `AttackRadius` (45) — detection sphere radius.
- `BasicAttackCooldown` (0.5), `SpecialAttackCooldown` (1.0).
- `BasicAttackMontage`, `SpecialAttackMontage`: if assigned, damage applies via notify; if null, damage is instant (legacy path).
- `HitImpactSound`: played at each impacted target's location.
- `bDrawDebugAttacks`: shows the detection sphere for 1s.

**Smart cooldowns:**
- Basic is blocked if Now - LastBasic < BasicCD **or** if Now - LastSpecial < SpecialCD (the special blocks everything).
- Special is only blocked by its own CD.

**Attack flow with montage:**
1. `TryBasicAttack()` → checks `CanBasicAttack()`, registers `LastBasicAttackTime`, plays the montage on the character owner.
2. Sets `CurrentAttackType = Basic`.
3. The montage has an `AnimNotify_DamageMoment` on the impact frame.
4. When the notify fires, calls `UDPCombatComponent::OnDamageNotify()` which runs `PerformAttack(BasicDamage, BasicRange)`.
5. `PerformAttack` does `SphereOverlapActors` filtering by Pawn channel, ignores the owner, applies `UGameplayStatics::ApplyDamage` to each found `ADPCharacterBase`, plays `HitImpactSound` at their location.

**Damage:**

`UGameplayStatics::ApplyDamage` → reaches `ADPCharacterBase::TakeDamage` → decrements health → hit reaction or death.

### 4.3 AI

Classic Behavior Tree with Blackboard and services.

#### `ADPEnemyAIController`

Inherits from `AAIController`. In `OnPossess` it activates the Behavior Tree:

```cpp
UBlackboardComponent* BBComp = nullptr;
if (UseBlackboard(BehaviorTreeAsset->BlackboardAsset, BBComp))
{
    RunBehaviorTree(BehaviorTreeAsset);
}
```

`BehaviorTreeAsset` is a `UPROPERTY(EditDefaultsOnly)` set to `BT_Enemy` from `BP_EnemyAIController`.

#### `BB_Enemy` (Blackboard)

Variables:
- `TargetActor` (Object, Base Class Actor): reference to the detected player.
- `bIsInAttackRange` (Bool): true if the target is within `AttackRange`.
- `SelfActor` (auto, don't remove): pointer to the possessed pawn.

#### `BT_Enemy` (Behavior Tree)

Minimal tree structure:

```
Root
└── Selector
    ├── Sequence (attack)
    │   ├── Decorator: bIsInAttackRange == true
    │   ├── Decorator: TargetActor != null
    │   └── BTTask_AttackTarget
    └── Sequence (chase)
        ├── Decorator: TargetActor != null
        └── MoveTo: TargetActor
```

With a `BTService_UpdateTarget` running on the Selector (tick 0.2s) that constantly refreshes `TargetActor` and `bIsInAttackRange`.

#### `UDPBTTask_AttackTarget`

Custom Task that invokes `CombatComponent->TryBasicAttack()` of the enemy. Returns `Failed` if:
- No AIController or pawn.
- No `CombatComponent` on the pawn (`FindComponentByClass`).
- No `TargetActor` in the Blackboard.
- `CanBasicAttack()` returns false (cooldown active).

Returns `Succeeded` after calling `TryBasicAttack`.

#### `UDPBTService_UpdateTarget`

Custom Service that every 0.2s:
1. Finds the nearest `ADPPlayerCharacter` within detection radius.
2. If found: sets `TargetActor` in the Blackboard.
3. Computes distance to the target and sets `bIsInAttackRange` if within `AttackRange`.
4. If no target in sight: sets `TargetActor` to null.

### 4.4 Input

Three subsystems abstract physical input from game logic, paving the way for Switch 2.

#### `UDPShakeDetectorSubsystem`

`UGameInstanceSubsystem` that detects accumulated agitation over a time window. Platform-agnostic: PC feeds mouse delta, Switch will feed gyro delta.

**API:**
- `FeedInputDelta(FVector Delta)`: entry point.
- `OnShakeDetected` (delegate): broadcasts when threshold is exceeded.
- Tunables: `ShakeWindowSeconds` (0.3), `ShakeThreshold` (50), `ShakeCooldown` (0.8).

**Algorithm:**
1. Each `FeedInputDelta` call adds a sample `{timestamp, magnitude}` to the history.
2. Samples older than `ShakeWindowSeconds` are cleaned up.
3. If the sum of magnitudes exceeds `ShakeThreshold`, fires `OnShakeDetected` and starts cooldown.
4. During cooldown, samples are ignored.

On PC, `BP_PlayerCharacter`'s `Event Tick`, while `bIsTriggeringShake = true` (right-click held), feeds the subsystem with `GetInputMouseDelta`.

#### `UDPGyroInputSubsystem`

`UGameInstanceSubsystem` that maintains a 2D tilt with passive auto-centering. Used by the puzzle marble to compute the "gravity" direction.

**API:**
- `FeedTiltDelta(FVector2D Delta)`: tilt increment.
- `ResetTilt()`: returns to (0, 0).
- `SetGyroActive(bool)`: subsystem only processes input when active.
- `CurrentTilt` (FVector2D, BlueprintReadOnly).
- `OnTiltChanged` (delegate).

**Tunables:**
- `TiltSensitivity` (0.01): multiplier of the incoming delta.
- `CenteringSpeed` (2.0): auto-centering speed.
- `MaxTilt` (1.0): per-component clamp.

**Passive auto-centering:** a global `FTSTicker` runs `TickCentering` every frame. It moves `CurrentTilt` towards `(0,0)` at `CenteringSpeed * DeltaTime`. Without this, accumulated tilt would never return to zero when the mouse stops moving.

On PC, `ADPPuzzlePlayerController::PlayerTick` feeds the subsystem with mouse delta while in Handheld mode.

#### `UDPPlatformModeSubsystem`

`UGameInstanceSubsystem` that is the **source of truth** for the global Docked/Handheld mode.

**API:**
- `GetCurrentMode()`, `IsDocked()`, `IsHandheld()`.
- `SetPlatformMode(EDPPlatformMode)`: changes the mode.
- `TogglePlatformMode()`: toggles.
- `OnPlatformModeChanged` (delegate): broadcasts to all listeners.

**Behavior on mode change (order matters):**
1. Updates `CurrentMode`.
2. **Applies Scalability** by mode: `level 1` (Medium) for Handheld, `level 3` (Epic) for Docked. Persists with `Scalability::SaveState(GGameUserSettingsIni)`.
3. **Requests on-screen message** via `UDPMessageSubsystem`: "Docked mode" or "Handheld mode" for 2s.
4. **Broadcasts** the delegate.

This order ensures listeners (camera, gyro, etc.) operate under the new perf budget.

**Auto-detection (SDK-ready):**

```cpp
EDPPlatformMode UDPPlatformModeSubsystem::QueryPlatformMode() const
{
#if PLATFORM_SWITCH
    // TODO Switch 2: replace with the real Nintendo SDK API
    // #include <nn/oe.h>
    // const nn::oe::OperationMode Mode = nn::oe::GetOperationMode();
    // return (Mode == nn::oe::OperationMode_Handheld)
    //     ? EDPPlatformMode::Handheld
    //     : EDPPlatformMode::Docked;
    return EDPPlatformMode::Docked;  // Fallback until SDK is available
#else
    return CurrentMode;
#endif
}
```

In Switch builds, an `FTimerHandle` polling every 0.5s runs `QueryPlatformMode()` and if it differs from the cached mode, calls `SetPlatformMode()`. On PC the polling doesn't activate (manual toggle with M key).

#### M key (global toggle)

`IA_TogglePlatform` is in `IMC_Default` (mapped to M key). Both:
- `ADPPlayerCharacter::OnToggleModePressed` (in L_Combat) → `TogglePlatformMode()`.
- `ADPPuzzlePlayerController::HandleToggleMode` (in L_Puzzle) → `TogglePlatformMode()`.

This makes the M key work **in both levels**. In L_Combat the change only affects scalability + HUD message (gameplay stays the same). In L_Puzzle, additionally, camera and pawn swap.

### 4.5 Puzzle

#### `ADPPuzzleBall`

`APawn` (not Character — doesn't need a movement component) with manually-simulated physics. Lives in the puzzle maze.

**Components:**
- `CollisionSphere` (USphereComponent): root, radius 50.
- `BallMesh` (UStaticMeshComponent): visual, attached to the CollisionSphere.

**Tunables:**
- `Acceleration = 16000` (high for snappy feel).
- `MaxSpeed = 500`.
- `Friction = 0.95` (per-second multiplier).
- `CurrentTilt` (FVector2D): assigned externally from the PlayerController.

**Tick (manual physics):**

```cpp
// 1. Apply acceleration by tilt
FVector TiltAcceleration(CurrentTilt.X * Acceleration, CurrentTilt.Y * Acceleration, 0.f);
CurrentVelocity += TiltAcceleration * DeltaTime;

// 2. Friction
CurrentVelocity *= FMath::Pow(Friction, DeltaTime);

// 3. Clamp to MaxSpeed
if (CurrentVelocity.SizeSquared() > MaxSpeed * MaxSpeed)
{
    CurrentVelocity = CurrentVelocity.GetSafeNormal() * MaxSpeed;
}

// 4. Move with sweep to detect collision
FVector NewLocation = GetActorLocation() + CurrentVelocity * DeltaTime;
FHitResult Hit;
SetActorLocation(NewLocation, true, &Hit);

// 5. Slide against walls (don't stick)
if (Hit.bBlockingHit)
{
    CurrentVelocity = FVector::VectorPlaneProject(CurrentVelocity, Hit.ImpactNormal);
}
```

`VectorPlaneProject` projects velocity onto the plane perpendicular to the impact normal, allowing the marble to slide along walls instead of getting stuck.

#### `ADPPuzzlePlayerController`

Custom PlayerController used only in L_Puzzle. Manages Docked/Handheld possession and feeds the gyro.

**`BeginPlay`:**
1. `FindPawnsInLevel()`: finds the `ADPPlayerCharacter` and `ADPPuzzleBall` in the level.
2. `FindPuzzleCameraInLevel()`: finds an `ACameraActor` tagged `"PuzzleCamera"`.
3. Registers the `InputMappingContext` (survives pawn swap).
4. Subscribes to `OnPlatformModeChanged`.
5. Calls `OnPlatformModeChanged(SubSystem->CurrentMode)` to apply the initial mode.

**`OnPlatformModeChanged(NewMode)`:**
- Activates/deactivates the gyro (`SetGyroActive(NewMode == Handheld)`).
- If Handheld: `Possess(BallPawn)` + `SetViewTargetWithBlend(PuzzleCameraActor)`.
- If Docked: `Possess(PlayerCharacterPawn)` + `SetViewTargetWithBlend(PlayerCharacterPawn)`.

**`PlayerTick`:**

```cpp
if (CachedGyroSubsystem == nullptr || !CachedGyroSubsystem->bGyroActive)
{
    return;
}

float MouseDeltaX = 0.f, MouseDeltaY = 0.f;
GetInputMouseDelta(MouseDeltaX, MouseDeltaY);

// For Camera Actor with Yaw=-90: direct Mouse X→X mapping, Mouse Y→Y inverted
const FVector2D Delta(MouseDeltaX, -MouseDeltaY);
CachedGyroSubsystem->FeedTiltDelta(Delta);
```

**Why a custom PlayerController instead of doing everything from `ADPPlayerCharacter`:** input bindings on the pawn don't survive possession changes. The modal toggle (M key) must live on the controller to survive.

#### `ADPHoleTrigger`

Actor with `UBoxComponent` (root). Overlap only, doesn't block.

`OnTriggerBeginOverlap`:
1. Verifies the incoming actor is `ADPPuzzleBall`.
2. Notifies `UDPPuzzleStateSubsystem`: `NotifyBallReachedGoal()`.
3. **Before** destroying the marble, switches mode to Docked. This is critical: the PlayerController needs **both** pawns alive to correctly `Possess(PlayerCharacterPawn)`.
4. `Ball->Destroy()`.

#### `UDPPuzzleStateSubsystem`

`UGameInstanceSubsystem`. Tracks puzzle state:

- `bBallReachedGoal`, `bPlayerHasKey`.
- `OnBallReachedGoal` (delegate broadcast when the marble reaches the hole).
- `SetPlayerHasKey(bool)`.

**Important pattern:** actors that need to react to "marble reached goal" (like the chest or the original key) subscribe to `OnBallReachedGoal` in their `BeginPlay`. They also have a defensive "if already fired before my spawn, fire now" check to support level streaming or deferred spawn.

#### `ADPInteractableBase`

Abstract class. Defines the contract:

```cpp
virtual void Interact(AActor* InteractingActor);
```

Everything the player can interact with by pressing E (keys, chests, doors) inherits from here.

**Polymorphic detection from the player:**

```cpp
void ADPPlayerCharacter::OnInteractionSphereBeginOverlap(...)
{
    if (ADPInteractableBase* Interactable = Cast<ADPInteractableBase>(OtherActor))
    {
        CurrentInteractable = Interactable;
    }
}
```

Without polymorphic detection (if instead it was `Cast<ADPPuzzleKey>`), adding new interactables like the chest or door would require modifying the player code. With the base class, **all you do is inherit and override Interact()**.

#### `ADPPuzzleChest`

Inherits from `ADPInteractableBase`. Animated chest that appears when the puzzle is solved.

**Components:**
- `ChestMesh` (`USkeletalMeshComponent`): root component. Configured in constructor with `QueryAndPhysics`, `WorldDynamic`, `Pawn=Block`, `WorldDynamic=Overlap`.

**BP-configurable:**
- `OpenAnimation` (`UAnimSequence*`): the Kenney `chestopen` animation.
- `OnOpenMessage` ("Key obtained"), `OnOpenMessageDuration` (3.0s).

**Flow:**
1. `BeginPlay`: hides (`SetActorHiddenInGame(true)` + `SetActorEnableCollision(false)`) + subscribes to `OnBallReachedGoal`.
2. When the delegate fires: shows + re-enables collision.
3. On E press (`Interact`): early-return if `bHasBeenOpened`, marks the flag, `ChestMesh->PlayAnimation(OpenAnimation, false)`, schedules `FTimerHandle` with `OpenAnimation->GetPlayLength()`.
4. When the timer fires `OnOpenAnimationEnded`: `SetPlayerHasKey(true)` on the state subsystem + `RequestMessage` to `UDPMessageSubsystem`.

**Why `PlayAnimation` and not `Montage_Play`:** montages return the AnimBP slot to bind pose when finished (chest would visually close again). `PlayAnimation` with `Loop=false` holds the last frame. The chest doesn't need an AnimBP — it's always in state A or B.

#### `ADPPuzzleDoor`

Inherits from `ADPInteractableBase`. Same pattern as the chest with key differences.

**Components:**
- `DoorMesh` (`USkeletalMeshComponent`): root, `QueryAndPhysics + WorldStatic + Block all`. Solid until animated.
- Separate `InteractionSphere` (`USphereComponent` radius 150): because the mesh blocks, we need a separate overlap to detect the player.

**BP-configurable:**
- `OpenAnimation`: the Kenney `gateopen` animation.
- `NoKeyMessage` ("You need a key. Maybe you'll find it taking the bull by the joycons... I mean, horns."), `NoKeyMessageDuration` (6.0s).
- `bTriggersLevelTransitionOnOpen`, `NextLevelName` (`L_Victory`), `TransitionDelay` (0.5s).

**`Interact` flow:**
- If `bIsOpen`: return.
- If `bPlayerHasKey == false`: `RequestMessage(NoKeyMessage, NoKeyMessageDuration)`.
- If `bPlayerHasKey == true`: calls `OpenDoor()`.

**`OpenDoor`:**
1. `bIsOpen = true` FIRST (gates re-interaction during the animation).
2. `DoorMesh->PlayAnimation(OpenAnimation, false)`.
3. `SetTimer(OpenAnimTimerHandle, OnOpenAnimationEnded, OpenAnimation->GetPlayLength())`.

**`OnOpenAnimationEnded`:**
1. `DoorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision)` (now the player can pass through).
2. If `bTriggersLevelTransitionOnOpen`: `TransitionToLevel(NextLevelName, TransitionDelay, 0.5)`.

Deferring the collision change and transition to `OnAnimationEnded` (not `OpenDoor`) ensures cinematic synchronization: the player can't pass through the door until it's visually open.

### 4.6 Transitions

#### `UDPLevelTransitionSubsystem`

`UGameInstanceSubsystem` that centralizes fade + delay + OpenLevel.

**API:**
```cpp
void TransitionToLevel(FName LevelName, float DelayBeforeFade = 0.f, float FadeDuration = 0.5f);
```

**Implementation:**
1. `bTransitionInProgress` guard to prevent overlapping transitions.
2. If `DelayBeforeFade > 0`, `FTimerHandle` for delay.
3. `StartFadeAndLoad`: `PC->PlayerCameraManager->StartCameraFade(0, 1, FadeDuration, FLinearColor::Black, false, true)` (hold when finished).
4. Another `FTimerHandle` for `ExecuteOpenLevel` after `FadeDuration`.
5. `ExecuteOpenLevel`: clears viewport widgets (`Viewport->RemoveAllViewportWidgets()`) and `UGameplayStatics::OpenLevel(this, PendingLevelName)`.

**Who fires the transitions:**
- L_Combat → L_Puzzle: `ADPCharacterBase::OnDeath` of the bull orc with `bTriggersLevelTransitionOnDeath = true` (delay 2.0s).
- L_Puzzle → L_Victory: `ADPPuzzleDoor::OnOpenAnimationEnded` with `bTriggersLevelTransitionOnOpen = true` (delay 0.5s).
- Game Over Retry: `WBP_GameOver` Retry button → reloads the current level.

### 4.7 HUD messages

#### `UDPMessageSubsystem`

`UGameInstanceSubsystem` (persists across levels) with a single delegate and one function:

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMessageRequested, FString, Message, float, Duration);

UPROPERTY(BlueprintAssignable)
FOnMessageRequested OnMessageRequested;

UFUNCTION(BlueprintCallable)
void RequestMessage(const FString& Message, float Duration);
```

`RequestMessage` simply does `OnMessageRequested.Broadcast(Message, Duration)`.

**`WBP_PlayerHUD` subscribes** to this delegate on its `Construct`. When it receives, it shows the text with fade in/out for the duration.

**Subsystem consumers:**
- `ADPPuzzleChest`: "Key obtained" when opened.
- `ADPPuzzleDoor`: "You need a key..." when interacting without it.
- `UDPPlatformModeSubsystem`: "Docked mode" / "Handheld mode" on change.

Any future system that needs to show text only has to ask the subsystem. No coupling to the HUD or the player.

---

## 5. Blueprint systems and assets

### 5.1 HUD

#### `WBP_PlayerHUD`

UMG widget created in `BP_PlayerCharacter::BeginPlay` and added to the viewport.

**Elements:**
- `HealthBar` (Progress Bar anchored top-left): color modulated by health (green 100%, yellow 50%, red <25%). Updates via subscription to the player's `OnHealthChanged` delegate.
- `TextMessage` (TextBlock anchored center-bottom, Cinzel font): shows messages from `UDPMessageSubsystem`. Has UMG animations `MessageFadeIn` and `MessageFadeOut` that play on receive/remove.
- `Image_JoyConShake` (Image anchored center-bottom, above the message): JoyCon icon shown during the special. Initially Collapsed. UMG animation `ShakeIconAnim` with rotation + lateral translation keyframes (infinite loop while visible).

**Public functions (called from BP_PlayerCharacter):**
- `UpdateHealth(Current, Max)`: updates the bar.
- `HandleMessage(Message, Duration)`: shows the message (internal, wired to the delegate).
- `ShowShakeIcon` / `HideShakeIcon`: manages the JoyCon icon.

#### `WBP_GameOver`

Death screen widget.

**Elements:**
- `BackgroundOverlay` (Image fill, black 70% opacity).
- Centered `VerticalBox`:
  - TextBlock "GAME OVER" (Cinzel Bold, size 96-128, intense red).
  - Spacer.
  - Button `RetryButton` (300x80) with a TextBlock "RETRY" inside.

**`On Clicked (RetryButton)`:**
```
Set Game Paused (false)
→ Remove from Parent
→ Get Game Instance → Get Subsystem (DPLevelTransitionSubsystem)
→ Get Current Level Name → To Name
→ Transition To Level (Target: subsystem, LevelName, DelayBeforeFade: 0.5, FadeDuration: 0.5)
```

#### `WBP_Victory`

End-screen widget.

**Design:**
- Dark brown background `#2A1810`.
- Top gold divider.
- Title "ProtoDungeon Switch 2" in Cinzel Bold 56, color `#EF9F27`.
- Subtitle "DEMO COMPLETED" in Montserrat 14, color `#BA7517`.
- Middle divider.
- Quote in Cinzel Italic 16, color `#FAC775`: *"The sphere rolled, the door opened, and somewhere a Joy-Con was shaken in vain."*
- Two buttons: "Play Again" (OpenLevel L_Combat) and "Quit" (Quit Game).

### 5.2 Materials

#### `M_FadeOutWall` and `M_KenneyFadeWall`

Substrate material (modern UE5 system) with SphereMask for auto-fade near the player.

**Graph structure (Substrate):**

```
[BaseColorTexture] ──► [Diffuse Albedo of SubstrateSlabBSDF]
                          │
[Constant 0.8] ─────────► [Roughness]
                          │
[WorldPosition] ─┐        │
                 ├──► [SphereMask] ──► [OneMinus] ──► [Substrate Coverage Weight] ──► [Result Front Material]
[CollectionParameter] ─┘  (Radius, Hardness)
(MPC_Global.PlayerPosition)
```

**Material Instances** for different textures:
- `MI_Wall_Fade`: Kenney `colormap` texture on the normal wall.
- `MI_WallHalf_Fade`, `MI_WallNarrow_Fade`, `MI_WallOpening_Fade`: variants.

**Material configuration:**
- `Blend Mode`: `TranslucentColoredTransmittance` (Substrate).
- `Lighting Mode`: `Surface TranslucencyVolume` for correct dynamic light reception.
- `Two Sided`: **OFF** (important: with Two Sided ON, hollow Kenney walls showed visual artifacts).

#### `MPC_Global`

Material Parameter Collection with:
- `PlayerPosition` (Vector parameter, default 0,0,0).

`BP_PlayerCharacter::Event Tick` writes to it every frame:

```
Branch: GetPlayerController.GetViewTarget == self ?
  True:  PlayerPosition = GetActorLocation
  False: PlayerPosition = (99999, 99999, 99999)
```

The else case prevents false fades when the camera is on another pawn (marble mode).

### 5.3 Lighting

**L_Combat:**
- `DirectionalLight`: rotation pointing slightly from behind-above. Subtle intensity. Cool color.
- `SkyLight`: cool ambient.
- `ExponentialHeightFog`: dark atmosphere.
- 5+ `BP_Torch` strategically placed: warm `PointLight` `(R=1, G=0.6, B=0.2)`, `AttenuationRadius ≈ 1800`, Movable.

**L_Puzzle (upper Kenney area):**
- Decorative Kenney torches + a few warm `PointLight`s on walls for uniform illumination.

**L_Puzzle (lower maze area):**
- 4 `PointLight` with `Cast Shadows: OFF`, light blue, `Intensity 8000`, Movable. No dynamic shadows to avoid artifacts when multiple lights overlap.

**System:** Lumen (dynamic GI) — no baking required. All lights marked `Mobility = Movable`.

**Post-process volume** in L_Combat with `Min EV100 = 5` and `Max EV100 = 7` to clamp auto-exposure (discovered empirically when UE was over-exposing).

### 5.4 Audio

Three Sound Cues with Random Without Replacement:

| Sound Cue | Sources | Trigger |
|---|---|---|
| `SC_Swing` | `S_Swing_01`, `S_Swing_02` | `Play Sound` notify at attack montage start |
| `SC_Hit` | `S_Hit_01..04` | `UDPCombatComponent::PerformAttack` on impact |
| `SC_Grunt` | `S_Grunt_01`, `S_Grunt_02` | `ADPCharacterBase::TakeDamage` via `DamageSound` |

**Pattern:** each Sound Cue has the Sound Waves wired to a `Random` node with `Without Replacement: true` (doesn't repeat the last). Random output to Cue output.

### 5.5 Animations

#### Kenney player montages (`character-human` skeleton)

| Montage | Base animation | Notifies |
|---|---|---|
| `AM_Attack_Basic_Kenney` | `humanattack-melee-right` (RateScale 0.6) | `DamageMoment` + `SC_Swing` |
| `AM_Attack_Special_Kenney` | `humanattack-kick-right` (RateScale 0.6) | `DamageMoment` + `SC_Swing` |
| `AM_HitReact_Kenney` | `humanfall` (BlendIn 0.05, BlendOut 0.1) | — |
| `AM_Die_Kenney` | `humandie` | — (unused, the AnimSequence is used directly) |

**Short Blend In/Out** (0.05/0.1) in HitReact: the `humanfall` animation is very short, default long blends would "eat" it before being seen.

#### Orc montages (`character-orc` skeleton)

| Montage | Base animation |
|---|---|
| `AM_Attack_Basic_Orc` | `orcattack-melee-right` |
| `AM_HitReact_Orc` | `orcfall` |
| `AM_Die_Orc` | `orcdie` (unused, AnimSequence directly) |

#### AnimBPs

- `ABP_KenneyHuman`: State Machine with Idle/Walk transitioning on `Speed > 10`. `Slot 'DefaultSlot'` over the state machine to play montages. Handler for `AnimNotify_DamageMoment` that casts the owner to `ADPCharacterBase` and calls `UDPCombatComponent::OnDamageNotify`.
- `ABP_KenneyOrc`: similar to human but with the orc skeleton.
- `ABP_KenneyChest`: only Slot 'DefaultSlot' → Output Pose. No state machine (the chest doesn't need locomotion).

**Important pattern**: the Idle/Walk transition threshold is `Speed > 10` (not `> 0`) because CharacterMovement has residual velocity noise that would trigger false transitions.

### 5.6 Kenney assets

`mini-dungeon` pack imported as **GLB** (preferred format to preserve materials and skeletons).

**Structure after import:**
- `Content/Kenney/MiniDungeon/Meshes/<asset>/`: each asset in its subfolder with SkeletalMesh, Skeleton, AnimSequences, and Textures.
- Shared `colormap` PNG across most assets (color atlas).

**Base import material:** `M_Default` (created by the GLB importer) is Substrate-compatible and used as reference for `M_KenneyFadeWall`.

---

## 6. Reusable design patterns

### 6.1 One-shot animation actor with state change

**When to use:** an actor that transitions from state A to persistent state B via a visual animation (chest, door, death).

**Structure:**

1. **RootComponent**: `USkeletalMeshComponent`, not `UStaticMeshComponent`.
2. **Animation**: `UAnimSequence*` (not `UAnimMontage`). Assigned via UPROPERTY EditAnywhere.
3. **SkeletalMesh AnimBP**: not needed. `PlayAnimation` bypasses the AnimGraph.
4. **State bool**: `bIsOpen` / `bHasBeenOpened`, source of truth for "already triggered".
5. **`FTimerHandle`** to detect animation end.

**`Interact()` flow:**

```cpp
void AMyActor::Interact(AActor* InteractingActor)
{
    if (bIsOpen) return;       // Idempotency
    bIsOpen = true;            // Mark FIRST

    if (OpenAnimation && Mesh)
    {
        Mesh->PlayAnimation(OpenAnimation, false);
        const float Duration = OpenAnimation->GetPlayLength();
        GetWorldTimerManager().SetTimer(
            AnimTimerHandle, this,
            &AMyActor::OnAnimationEnded,
            Duration, false);
    }
    else
    {
        OnAnimationEnded();    // Fallback
    }
}
```

**Animation end handler**: all persistent effects (collision toggle, transition, subsystem updates, HUD messages) go **here**, not in `Interact()`. Visual feedback ↔ mechanic sync.

**Collision rules:**
- Constructor: `Block` against Pawn so the actor is solid in state A.
- **Don't call `SetActorEnableCollision(false)` in Interact()**: the state bool already prevents re-interaction.
- If state B requires walk-through: `Mesh->SetCollisionEnabled(NoCollision)` in `OnAnimationEnded()`.

**Why AnimSequence and not AnimMontage:** montages return the slot to bind pose when finished. `PlayAnimation(Loop=false)` holds the last frame.

**Variant for Character with AnimBP:** same pattern but `GetMesh()->PlayAnimation(...)` bypasses the AnimBP. After this the actor doesn't need to return to the AnimBP (death case: on Retry the level reloads).

### 6.2 Subsystem as source of truth

For any global game state (puzzle state, platform mode, level transition, HUD messages), a `UGameInstanceSubsystem` centralizes:

- **State** (internal variables).
- **API** (functions to mutate the state).
- **Delegate** (broadcast to listeners).

Systems that react **subscribe to the delegate in their BeginPlay** and unsubscribe in EndPlay. They don't know who fires the change, only what to do when it occurs.

**Project examples:**

| Subsystem | State | Delegate |
|---|---|---|
| `UDPPlatformModeSubsystem` | `CurrentMode` | `OnPlatformModeChanged` |
| `UDPPuzzleStateSubsystem` | `bBallReachedGoal`, `bPlayerHasKey` | `OnBallReachedGoal` |
| `UDPMessageSubsystem` | (stateless, just relays) | `OnMessageRequested` |
| `UDPLevelTransitionSubsystem` | `bTransitionInProgress` | (no delegate, uses internal timers) |
| `UDPShakeDetectorSubsystem` | Sample history | `OnShakeDetected` |

**Benefit:** adding new systems that react to the state doesn't require modifying whoever fires the change. Low coupling.

### 6.3 Polymorphic interactable

A base class `ADPInteractableBase` with `virtual void Interact(AActor* InteractingActor)`. The player detects any interactable via `Cast<ADPInteractableBase>(OtherActor)` in their `InteractionSphere` overlap.

**To add a new interactable:**
1. Inherit from `ADPInteractableBase`.
2. Override `Interact()`.

The player **doesn't need modification**. The system grows without touching existing code.

### 6.4 Single source of input for global actions

The M key is in `IMC_Default` (a single Input Action `IA_TogglePlatform`). Both:
- `ADPPlayerCharacter` binds it for `TogglePlatformMode()`.
- `ADPPuzzlePlayerController` binds it likewise.

The subsystem is the only source of truth. Anyone can call `TogglePlatformMode()` and the system reacts consistently.

### 6.5 Defensive `BeginPlay` for PIE

After a level reload (Retry, OpenLevel), the editor viewport may retain contaminated state (InputMode, cursor visibility, pause). The player's `BeginPlay` defensively resets:

```cpp
if (APlayerController* PC = Cast<APlayerController>(GetController()))
{
    FInputModeGameOnly InputMode;
    PC->SetInputMode(InputMode);
    PC->SetShowMouseCursor(false);
    EnableInput(PC);
}
```

Free if already correct. Saves you if broken.

---

## 7. Documented bugs and fixes

**16 major bugs** documented throughout the project with root cause and fix. Listed here for future reference.

### Bugs 1-5: AnimBP and animations

1. **`Speed > 0` in AnimBP triggers constantly.** Cause: `UCharacterMovementComponent` has residual velocity noise even when stationary. Fix: use threshold `Speed > 10`.

2. **Missing `Slot 'DefaultSlot'` node in AnimGraph.** Symptom: montages play but don't render. Fix: add `Slot 'DefaultSlot'` in the AnimGraph between the state machine and `Output Pose`.

3. **`AnimNotify_DamageMoment` requires a handler in each AnimBP.** Pattern: `Try Get Pawn Owner → Cast to DPCharacterBase → Get Component DPCombatComponent → OnDamageNotify`. Shared between player and enemy.

4. **Default Blend In/Out 0.25 eats short animations.** `humanfall` (~0.3s) gets cut. Fix: in the hit react montage, `Blend In Time = 0.05`, `Blend Out Time = 0.1`.

5. **Anim Class not assigned to Mesh in child BP.** Symptom: idle pose correct but `Try Get Pawn Owner` returns wrong reference, velocity reads as 0. Diagnosis: `Event Tick + Print String` comparing real velocity (350) vs AnimBP-read (0).

### Bugs 6-8: Material and MPC

6. **Two Sided ON in `M_KenneyFadeWall` causes artifacts on hollow walls.** Kenney walls are decorative interior pieces (hollow), not solid like SM_Cube. Fix: disable Two Sided.

7. **`PlayerPosition` MPC must be written as (99999) in marble mode.** Without this, the last known player position stays fixed and produces false fades in the maze. Fix: branch in Event Tick.

8. **Puzzle maze in same Z as Kenney room = top-down camera sees through.** Fix: separate vertically in Z. Originally the puzzle was above, then moved below. With dedicated lights (`Cast Shadows: OFF`) in the maze.

### Bugs 9-13: Kenney chest and door

9. **`USkeletalMeshComponent` starts with `NoCollision` by default.** When creating a C++ actor with SkeletalMesh root, it doesn't generate overlap events nor receive interaction. Fix: configure collision in the constructor explicitly (`QueryAndPhysics`, `WorldDynamic`, Pawn=Block, WorldDynamic=Overlap).

10. **Chest visually closes after `Montage_Play`.** Cause: the AnimBP slot returns to bind pose. Fix: use `USkeletalMeshComponent::PlayAnimation(AnimSequence, false)` which holds the last frame.

11. **Chest intangible after opening.** Cause: `SetActorEnableCollision(false)` in `Interact()` to "prevent re-interaction". But the `bHasBeenOpened` flag already prevents that and disabling collision breaks the Pawn block. Fix: remove that line.

12. **Kenney SkeletalMesh pivot doesn't match cube placeholder.** When migrating `DoorMesh` from StaticMesh (centered pivot) to SkeletalMesh (base pivot), the door becomes misaligned. Fix: manually re-adjust Transform in the level.

13. **Level transition starts at the same time as opening animation.** If you call `TransitionToLevel` from `OpenDoor` directly, fade starts before the animation ends. Fix: defer transition to `OnOpenAnimationEnded` with the `FTimerHandle`.

### Bugs 14-16: Game Over and UI

14. **`Build.cs` without UMG → CreateWidget fails to link.** Until now UMG was only used from Blueprint. Adding `TSubclassOf<UUserWidget>` to the player header makes the linker fail to find `UUserWidget::CreateWidgetInstance` and `AddToViewport`. Fix: add `"UMG"` to `PublicDependencyModuleNames` + regenerate VS project files.

15. **Player returns without input after Retry in PIE.** After `SetInputModeUIOnly` before Game Over, the editor viewport gets contaminated. After `OpenLevel`, the new PC starts with `FInputModeGameAndUI` but the viewport stays in UI mode. Fix: in player's `BeginPlay`, defensively force `FInputModeGameOnly` + `EnableInput(PC)`.

16. **DeathAnimation doesn't play even though the field is assigned.** Cause: the field is `UAnimSequence*` but a `UAnimMontage` was assigned by mistake (UE visually allows it). `PlayAnimation()` fails silently. Fix: always use the raw AnimSequence (green/blue icon), not the montage (red icon).

### Other minor bugs documented

- **HUD Render Transform Scale scales from pivot (0.5, 0.5)** and corner-anchored elements fall outside the viewport. Not really a bug, but incorrect usage. Decision: don't scale HUD between Docked/Handheld in this prototype (UE's DPI Scaling already scales by output resolution in real builds).

- **`OpenLevel` doesn't reset pause automatically in PIE.** Game Over Retry calls `Set Game Paused(false)` explicitly before transitioning.

- **NuGet `coverlet.collector` 6.0.4 breaks `GenerateProjectFiles`** in some setups. Fix: `dotnet nuget locals all --clear` and regenerate.

- **UE includes require relative path with `../` between subfolders.** UBT does NOT add each subfolder as include path. `#include "../Combat/DPCombatComponent.h"` from `Characters/`.

---

## 8. Switch 2 roadmap (deploy)

### What's already prepared

The project is architecturally ready for Switch 2:

- **`#if PLATFORM_SWITCH` placeholders** in `UDPPlatformModeSubsystem::QueryPlatformMode()` with TODO comments referencing the expected `nn::oe::GetOperationMode()` API.
- **Polling every 0.5s** in Switch builds (timer already configured, polling currently returns the Docked fallback).
- **Automatic scalability** that lowers quality in Handheld (level 1 Medium) and raises in Docked (level 3 Epic).
- **Input abstraction** via subsystems (`UDPShakeDetectorSubsystem`, `UDPGyroInputSubsystem`) that separate "what to do with a shake/tilt" from "where the input comes from".

### Steps to activate real Switch 2

#### 1. Nintendo SDK access

- Requires being a registered developer at the Nintendo portal (https://developer.nintendo.com).
- Download the Switch 2 plugin for UE 5.7 (NDA under developer agreement).
- Extract into `Engine/Platforms/Switch/` or similar (exact structure documented by the SDK).

#### 2. Project configuration

- Project Settings → Platforms → Switch: mark as supported.
- `ProtoDungeonSW2.Target.cs`: add `Switch` to `SupportedPlatforms` if not implicit.

#### 3. Enable real mode detection

In `UDPPlatformModeSubsystem::QueryPlatformMode()`, uncomment the SDK block:

```cpp
#if PLATFORM_SWITCH
    #include <nn/oe.h>  // verify exact name in SDK
    const nn::oe::OperationMode Mode = nn::oe::GetOperationMode();
    return (Mode == nn::oe::OperationMode_Handheld)
        ? EDPPlatformMode::Handheld
        : EDPPlatformMode::Docked;
#else
    return CurrentMode;
#endif
```

**Verify beforehand:**
- Exact header name (`<nn/oe.h>` or `<nn/oe2.h>` for Switch 2).
- Exact enum name (may be `nn::oe2::OperationMode` with slightly different values).
- Whether additional `#include` or Build.cs module is required.

#### 4. Switch build

```powershell
# From the UE root:
.\Engine\Build\BatchFiles\RunUAT.bat BuildCookRun -project="C:\dev\ProtoDungeonSW2\ProtoDungeonSW2.uproject" -platform=Switch -clientconfig=Development -cook -allmaps -build -stage -pak
```

(Exact command may vary by SDK version)

#### 5. Hook JoyCons to the shake/gyro system

On PC, `UDPShakeDetectorSubsystem` receives mouse delta via `FeedInputDelta(GetInputMouseDelta)` from the Tick. On Switch:

```cpp
#if PLATFORM_SWITCH
    // In the player's Tick or a dedicated subsystem:
    const FVector2D AccelDelta = JoyConSDK::GetAccelerometerDelta();
    ShakeDetector->FeedInputDelta(FVector(AccelDelta.X, AccelDelta.Y, 0));
#endif
```

Same for the gyro:

```cpp
#if PLATFORM_SWITCH
    const FVector2D TiltDelta = JoyConSDK::GetGyroDelta();
    GyroSubsystem->FeedTiltDelta(TiltDelta);
#endif
```

**You don't need to change `ADPPuzzleBall` or the rest of the game.** The subsystem handles processing the input, whatever its source.

#### 6. Emulator testing

Your office colleague has a Switch 2 emulator that simulates docked/handheld. Plan:

1. Build the project in Switch mode.
2. Load the `.nsp` (or whatever format the emulator uses).
3. Test the full game:
   - L_Combat: combat against the orc using real input.
   - Press the "detach JoyCons" button → Handheld should activate automatically → HUD shows "Handheld mode" → scalability lowers.
   - In L_Puzzle, same thing → additionally, `ADPPuzzlePlayerController` switches to the marble and top-down camera.
   - Real JoyCon gyro should move the marble.

#### 7. Polish for Switch 2

Things worth revisiting after the first working build:

- **HUD scaling**: if the HUD looks small in portable mode (7.9" LCD screen), apply Render Scale 1.15 to `WBP_PlayerHUD` with a `Scale Box` that respects anchors. (Not done on PC because plain `Set Render Scale` broke anchors).
- **Joy-Con rumble**: add haptic feedback for combat impacts, special shake, chest opening, door opening.
- **Optional touch input** in handheld (interact with chests by touching the screen).
- **Real JoyCon icons** for the HUD shake feedback (replace the generated placeholder).
- **Performance profile**: measure frame time in Handheld and tune scalability if needed.

### SDK info we need from Switch 2

When you get to the devkit, note:

1. Exact preprocessor macro name for SW2 (`PLATFORM_SWITCH` or `PLATFORM_SWITCH2`).
2. SDK header for mode detection (`<nn/oe.h>`, `<nn/oe2.h>`, etc.).
3. Exact enum name and values (`nn::oe::OperationMode_Handheld` vs alternatives).
4. JoyCon input API (gyro, accelerometer, rumble).
5. Any additional Build.cs module required?

---

## 9. Workflow and philosophy

### C++ vs Blueprint

**C++ for:**
- Subsystems (global state, managed lifecycle).
- Base classes (`ADPCharacterBase`, `ADPInteractableBase`).
- Reusable components (`UDPCombatComponent`).
- GameModes.
- Manual physics logic (`ADPPuzzleBall`).
- Custom AI Tasks/Services.

**Blueprint for:**
- UI (UMG Widgets).
- Instance configuration (assigning AnimSequence, Widget Classes, Montages).
- Level Blueprints.
- Materials (visual Material Editor).
- Logic that needs fast iteration without recompiling.

**"Tune in BP, move to C++" pattern:**
1. Iterate values in BP at runtime (no recompile).
2. When values are confirmed, move them to the C++ constructor (source of truth).
3. Reset BP overrides (the property's "Reset to Default" button) so it inherits from C++ correctly.

Example: isometric camera. Tuned in BP (`Pitch -50`, `ArmLength 2000`), then moved to `ADPPlayerCharacter::ADPPlayerCharacter()` constructor.

### AI workflow

**Chat (claude.ai):**
- Architectural design, conceptual questions.
- Isolated code review.
- Stack trace debugging.
- Generating a new class from scratch.
- Conversation, exploration.

**Claude Code (terminal):**
- Multi-file changes.
- Refactors.
- Builds + reading compiler errors automatically.
- Full scaffolding (header + cpp).
- Modifying existing files with precise changes.

**Claude Code prompt pattern (inherited from this project):**
- Task description + rules (don't compile, show me diff, don't touch X).
- Conventions (DP prefix, English comments, English identifiers).
- Explicit restrictions (don't touch Build.cs unless instructed, don't touch other files).
- Pre-checks to perform (read X.cpp to understand current API).

### Iteration: test → tune → consolidate

Philosophy repeated in every block:

1. **Implement the minimal working version.**
2. **Test it at runtime.**
3. **Tune by feel, not by theoretical numbers.**
4. **When it feels right, consolidate** (move value to C++, commit, document).

Examples:
- Camera: pitch -30 → test → corridor not visible → -50 → good → move to C++.
- Marble: Acceleration 980 → too slow → 16000 → good.
- ShakeThreshold: 80 → too demanding → 50 → good.

### Conventional Commits

Pattern:

```
<type>(<scope>): <subject>

<optional body with details>
```

Types used in the project:
- `feat`: new functionality.
- `fix`: bug fix.
- `refactor`: code change without changing behavior.
- `docs`: documentation only.
- `chore`: tooling changes.

Project examples:
- `feat(puzzle): add ADPPuzzleChest with Kenney chest mesh and animation`
- `feat(player): add game over flow with death animation and retry`
- `feat(platform): wire UDPPlatformModeSubsystem into L_Combat and add auto-scalability`
- `refactor(camera): move tuned isometric camera values to C++ constructor`
- `docs(claude): add one-shot animation actor pattern to CLAUDE.md`

### Closed sessions

Each work session ends with:

1. **Commit of completed work**.
2. **Push to GitHub**.
3. **Status summary** (what was accomplished, what remains).
4. **Next step anticipated**.

If a session is left half-done, don't commit code that doesn't compile. Better `git stash` or finish the small block before closing.

---

## 10. Quick reference

### Key commands

```powershell
# Regenerate VS files after changing Build.cs
& "C:\UE57\Engine\Build\BatchFiles\GenerateProjectFiles.bat" -project="C:\dev\ProtoDungeonSW2\ProtoDungeonSW2.uproject" -game

# Launch Claude Code
cd C:\dev\ProtoDungeonSW2 && claude

# Build from terminal (alternative to VS)
& "C:\UE57\Engine\Build\BatchFiles\Build.bat" ProtoDungeonSW2Editor Win64 Development -project="C:\dev\ProtoDungeonSW2\ProtoDungeonSW2.uproject"

# Clear NuGet cache if GenerateProjectFiles fails
dotnet nuget locals all --clear
```

### Game hotkeys

| Key | Action |
|---|---|
| WASD | Player movement |
| Left-click | Basic attack |
| Right-click (hold) + mouse movement | Special attack (simulating JoyCon shake) |
| E | Interact (chest, door) |
| **M** | Toggle Docked / Handheld mode |

### Main classes and responsibilities

| Class | Responsibility |
|---|---|
| `ADPCharacterBase` | Health, damage, hit reactions, death with animation. Base for player and enemies. |
| `ADPPlayerCharacter` | Player with isometric camera, Enhanced Input, interaction, Game Over. |
| `ADPEnemyBase` | AI-controlled enemy. |
| `ADPPuzzleBall` | Marble pawn with manual physics. |
| `UDPCombatComponent` | Combat logic (attacks, cooldowns, damage). |
| `ADPEnemyAIController` | AI controller that runs the Behavior Tree. |
| `ADPInteractableBase` | Abstract base class for interactables (E key). |
| `ADPPuzzleChest` | One-shot animated chest. |
| `ADPPuzzleDoor` | One-shot animated door with key gate. |
| `ADPHoleTrigger` | Trigger that fires when the marble falls in. |
| `ADPPuzzlePlayerController` | Custom PlayerController for L_Puzzle (Docked/Handheld possession). |
| `UDPShakeDetectorSubsystem` | Detects shaking (mouse on PC, gyro on Switch). |
| `UDPGyroInputSubsystem` | 2D tilt with auto-centering. |
| `UDPPlatformModeSubsystem` | Source of truth for Docked/Handheld mode. |
| `UDPPuzzleStateSubsystem` | Puzzle state (marble in hole, key obtained). |
| `UDPLevelTransitionSubsystem` | Centralized fade + delay + OpenLevel. |
| `UDPMessageSubsystem` | Decoupled HUD messages. |

### Key tunables per system

| System | Value | Why |
|---|---|---|
| Isometric camera | Pitch -50, ArmLength 2000, FOV 60 | Clear view of the Kenney dungeon |
| Player walk speed | 500 (400 Kenney) | Snappy combat feel |
| Basic attack damage | 25 | 4 hits to kill the orc |
| Special attack damage | 50 | Double the basic, justifies cooldown |
| Basic attack cooldown | 0.5s | No spam combos |
| Special attack cooldown | 1.0s | Blocks EVERYTHING during that second |
| Attack range / radius | 80 / 45 (Kenney chibi) | Reasonable hitbox for chibi scale |
| ShakeWindow / Threshold | 0.3s / 50 | Clear movement without being punishing |
| Ball Acceleration | 16000 | Snappy feel tuned by hand |
| Ball MaxSpeed | 500 | Doesn't fly out of the maze |
| Ball Friction | 0.95 | Natural deceleration without feeling sticky |
| Transition fade | 0.5s | Cinematic without being slow |
| Death animation delay | `DeathAnimation->GetPlayLength()` | UI appears synced with animation |

### Workflow for adding new functionality

When adding a new system to the project, this pattern has worked well:

1. **Design in chat**: discuss architecture, decide base class, define public API.
2. **Explicit decisions**: C++ or Blueprint? Subsystem or component? Delegate or direct callback?
3. **Claude Code prompt**: with all decisions, generate code in one pass.
4. **Review diff**: verify it follows conventions (DP prefix, English comments, `../` includes).
5. **Compile**: if it fails, read the error calmly. Usually `Build.cs` or a missing include.
6. **Configure in editor**: assign UPROPERTIES in BPs, wire to levels.
7. **Runtime test**: tune values by feel.
8. **Consolidate**: move tuned values to C++.
9. **Commit** with Conventional Commits.
10. **Document** if a new pattern emerges (in CLAUDE.md).

---

## Closing

ProtoDungeonSW2 is a complete end-to-end prototype. It demonstrates:

- **Combat** with animations, AI, audio, hit reactions, cinematic death.
- **Switch 2 unique mechanic**: physical Docked/Handheld swap with camera, control and pawn changes.
- **Technical polish**: Substrate auto-fade walls, automatic scalability, decoupled HUD, cinematic transitions.
- **Solid architecture**: subsystems as source of truth, reusable patterns (one-shot animation, polymorphic interactable), SDK-ready code.

The project is the foundation to iterate towards a full game or as a technical showcase for Nintendo. Every decision is documented (this file, the CLAUDE.md files, the 16 bugs and fixes) so the next developer (or yourself in 6 months) can resume without losing context.

**Optional next steps:**
- Real Switch 2 devkit deployment.
- Rumble on real JoyCons.
- More levels (combat + puzzle alternating).
- Ambient audio in levels.
- Music.
- More enemies.
- Progression system.

---

*Document generated at project close. Last updated: June 2026.*
