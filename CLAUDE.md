\# ProtoDungeonSW2 — Context for Claude



\## Project

Isometric dungeon crawler, 2-level prototype, target Nintendo Switch 2.

Developed on PC (Windows) with Unreal Engine 5.7 built from source.



\## Planned levels

\- \*\*Level 1 (L\_Combat):\*\* combat with stick movement, basic attack (button) and 

&#x20; special attack by shaking the JoyCon (simulated with a mouse click on PC).

\- \*\*Level 2 (L\_Puzzle):\*\* puzzle that toggles dock mode (isometric camera) and 

&#x20; handheld mode (top-down camera + gyroscope that moves a ball through a maze).



\## Engine

\- UE 5.7 source build at C:\\UE57 (junction).

\- Do NOT modify engine code unless explicitly requested.

\- Standard build config: Development Editor | Win64.



\## Project on disk

\- Path: C:\\dev\\ProtoDungeonSW2

\- Module name: ProtoDungeonSW2

\- Module API macro: PROTODUNGEONSW2\_API



\## Repository

\- GitHub (https://github.com/ManuMendezMPG/ProtoDungeonSW2), main.

\- Git LFS configured for .uasset, .umap, and the usual binaries.

\- Ignore: Binaries/, Intermediate/, DerivedDataCache/, Saved/, .vs/



\## Code conventions

\- Project class prefix: DP (e.g.: ADPPlayerCharacter, UDPCombatComponent).

\- Standard UE prefixes: A for Actors, U for UObjects/Components, F for structs, E for enums.

\- Every editable UPROPERTY must have a specific Category.

\- Use TObjectPtr<T> instead of T\* for UObject members.

\- Logs with their own categories: DECLARE\_LOG\_CATEGORY\_EXTERN + DEFINE\_LOG\_CATEGORY.

\- Comments and code (identifiers, strings) in English.



\## Modules

\- Main module: ProtoDungeonSW2

\- Current dependencies: Core, CoreUObject, Engine, InputCore

\- Dependencies we'll add as needed: EnhancedInput, PhysicsCore, UMG



\## Input

\- We use Enhanced Input (NOT the old system).

\- IMCs split by context (Combat, Puzzle) plus one global (Default).



\## Platform

\- Dock/handheld and gyroscope are simulated on PC for development:

&#x20; - Toggle dock/handheld: key (TBD).

&#x20; - Gyroscope: mouse or WASD alternative.

\- Abstraction via Subsystems so gameplay code is platform-agnostic.

\- The Switch 2 SDK lives at C:\\UE57\\Engine\\Platforms\\Switch2 (present but not active in PC development).



\## Acceptance criteria for generated code

\- Compiles without warnings in Development Editor.

\- Correct headers and GENERATED\_BODY() included.

\- UPROPERTY/UFUNCTION with correct specifiers (EditDefaultsOnly vs EditAnywhere, 

&#x20; BlueprintReadOnly vs BlueprintReadWrite, etc.).

\- Clear separation between header (.h) and implementation (.cpp).

\- Forward declarations in headers where possible to minimize includes.



\## AI workflow

\- For new classes: request header + cpp together, include Build.cs if it changes.

\- For multi-file changes: use Claude Code; for conceptual questions: chat.

\- Frequent, small commits whenever something compiles.


## Includes in UE (empirically verified rule)

Includes within the module use paths relative to the source file:

- File in the SAME folder: #include "File.h"
- File in a DIFFERENT folder: use ../ to go up and down
  Example: from GameModes/Something.cpp include Characters/Other.h:
  #include "../Characters/Other.h"

For external modules (engine), paths are relative to that module's public 
root, and do not need ../:
  #include "GameFramework/Character.h"
  #include "Camera/CameraComponent.h"

Note: If in the future we want "absolute" paths from the module root without 
relative ../, we can add PublicIncludePaths in ProtoDungeonSW2.Build.cs.

## Naming Conventions

### Parameter shadowing of UE base class members

Avoid parameter names in function signatures of classes inheriting from AActor or UObject that shadow members of those base classes. UE prohibits shadowing and the compiler will fail with an error like "Function parameter: 'X' cannot be defined ... as it is already defined in scope 'AActor' (shadowing is not allowed)".

Members of AActor to avoid as parameter names:
- Instigator (use InteractingActor, Caller, OtherActor)
- Owner (use OwningActor, OwningPawn)
- Tags (use ActorTags)
- Role (use NetworkRole)

Members of UObject to avoid as parameter names:
- Name (use ActorName, ComponentName)
- Outer (use OuterContext)

When in doubt, prefix with a descriptor (Interacting, Owning, Source, Target).

Example - bad:
    virtual void Interact(AActor* Instigator);  // Compilation error

Example - good:
    virtual void Interact(AActor* InteractingActor);

## Pattern: actor with a one-shot animation that changes state

When a puzzle actor (or any other system) needs to play 
an animation that takes it from state A to a persistent 
state B — for example chest closed → open, door closed → 
open —, follow this pattern.

Code references: `ADPPuzzleChest`, `ADPPuzzleDoor`.

### Structure

1. **RootComponent**: `USkeletalMeshComponent`, not `UStaticMeshComponent`. 
   The skeletal is required to play animations even when the actor 
   doesn't need an AnimBP.

2. **Animation**: `UAnimSequence*` assigned via `UPROPERTY EditAnywhere`. 
   Don't use `UAnimMontage` for this case: montages return the slot 
   to bind pose when they end, which visually reverts the 
   state.

3. **SkeletalMesh AnimBP**: not needed. `PlayAnimation()` puts 
   the `SkeletalMeshComponent` into `AnimationSingleNode` mode and bypasses 
   the AnimGraph.

4. **State bool**: `bool bIsOpen` / `bool bHasBeenOpened`, 
   `UPROPERTY BlueprintReadOnly`. This is the source of truth for "already 
   activated", not collision or visibility.

5. **FTimerHandle**: private, to detect the end of the animation. 
   `PlayAnimation` does not fire `OnMontageEnded` (it doesn't use montages); 
   we use a manual timer with the animation's actual duration.

### Interact() flow

```cpp
void AMyActor::Interact(AActor* InteractingActor)
{
    if (bIsOpen) return;       // Idempotency: don't re-activate
    bIsOpen = true;            // Mark FIRST to block re-entry

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
        // Fallback: if the animation or the mesh is missing, we don't 
        // want the mechanic to get stuck. Call the animation-end handler 
        // directly.
        OnAnimationEnded();
    }
}
```

### Animation-end handler

All **persistent** effects (collision changes, level transitions, 
setting a flag on a subsystem, broadcasting a message, etc.) go here, 
**never directly inside Interact()**. This way the visual feedback 
and the mechanic stay synchronized.

```cpp
void AMyActor::OnAnimationEnded()
{
    // Persistent effects: collision toggle, transition, subsystem, 
    // HUD message, etc.
}
```

### Collision rules

- **Constructor**: set the mesh to Block against Pawn 
  (`WorldStatic` or `WorldDynamic` as appropriate) so the 
  player can't pass through the actor while it's in state A.
- **Don't call `SetActorEnableCollision(false)` in Interact()**: the 
  state bool already prevents re-interaction. Disabling all 
  collision breaks physical blocking.
- **If state B requires the player to be able to pass through the actor** 
  (the door case): do `Mesh->SetCollisionEnabled(NoCollision)` 
  in `OnAnimationEnded()`, not before.
- **If the actor must be hidden until a reveal event** 
  (the chest with `OnBallReachedGoal` case): in `BeginPlay()` do 
  `SetActorHiddenInGame(true)` + `SetActorEnableCollision(false)`. In 
  the reveal delegate handler, re-enable both.

### Connection to UDPMessageSubsystem

If the actor needs to show a message to the player (on opening, on failing 
a condition, etc.), request it from the subsystem in Interact() or in 
OnAnimationEnded() depending on the case:

```cpp
if (UGameInstance* GI = GetGameInstance())
{
    if (UDPMessageSubsystem* MessageSubsystem = 
        GI->GetSubsystem<UDPMessageSubsystem>())
    {
        MessageSubsystem->RequestMessage(Message, Duration);
    }
}
```

Don't use `GEngine->AddOnScreenDebugMessage` in gameplay logic — 
that channel is for temporary debug only.

### Variant for a Character with AnimBP

When the actor is an ACharacter with a full AnimBP (state machine 
locomotion + montages) but also has a one-shot animation that 
must persist (player death, enemy death), apply the 
same pattern:

- The animation goes as a UAnimSequence (not UAnimMontage). PlayAnimation 
  bypasses the AnimBP and holds the last frame.
- We assume the actor doesn't need to return to the AnimBP after this 
  animation. If it had to be reanimated, you'd restore the mode 
  with SetAnimationMode(EAnimationMode::AnimationBlueprint) and reassign 
  the AnimClass.

Reference: ADPCharacterBase::OnDeath() plays DeathAnimation 
via GetMesh()->PlayAnimation(DeathAnimation, false).

### Combining with UI after the animation

When the animation culminates in UI (Game Over, victory, modal 
message):

1. Schedule the widget with an FTimerHandle and Delay = 
   AnimSequence->GetPlayLength() to synchronize.
2. When showing the widget: SetInputModeUIOnly + SetShowMouseCursor(true) 
   + SetGamePaused(true).
3. When closing the widget (Retry, Continue, etc. button): SetGamePaused(false) 
   + RemoveFromParent + next action.
4. Defensively, in BeginPlay of the Character spawned after a reload: 
   SetInputModeGameOnly + SetShowMouseCursor(false) + EnableInput(PC). 
   The editor viewport (PIE) can retain the previous InputMode and 
   block input to the new player.

Reference: ADPPlayerCharacter::OnDeath() / ShowGameOverScreen() / 
BeginPlay().