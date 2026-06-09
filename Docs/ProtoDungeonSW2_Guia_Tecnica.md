# ProtoDungeonSW2 — Guía Técnica del Proyecto

> Prototipo dungeon crawler isométrico para **Nintendo Switch 2**, desarrollado en Unreal Engine 5.7 source build. Combina combate cuerpo a cuerpo con un puzzle de bola controlado por giroscopio simulado, todo en dos niveles que demuestran el cambio entre modo **Docked** y **Handheld**.

---

## Tabla de contenidos

1. [Visión general](#1-visión-general)
2. [Setup del proyecto](#2-setup-del-proyecto)
3. [Niveles](#3-niveles)
4. [Sistemas C++](#4-sistemas-c)
5. [Sistemas Blueprint y assets](#5-sistemas-blueprint-y-assets)
6. [Patrones de diseño reutilizables](#6-patrones-de-diseño-reutilizables)
7. [Bugs documentados y soluciones](#7-bugs-documentados-y-soluciones)
8. [Roadmap Switch 2 (deploy)](#8-roadmap-switch-2-deploy)
9. [Workflow y filosofía](#9-workflow-y-filosofía)
10. [Referencia rápida](#10-referencia-rápida)

---

## 1. Visión general

### Concepto

ProtoDungeonSW2 es un prototipo de **dos niveles** que sirve como showcase de mecánicas características de Switch 2:

- **L_Combat**: dungeon clásico con combate cuerpo a cuerpo. Un jugador humanoide se enfrenta a un orco. El ataque básico se ejecuta con click izquierdo; el especial agitando el JoyCon (simulado en PC con click derecho + movimiento del ratón).
- **L_Puzzle**: el jugador encuentra una sala con un laberinto cenital. Al cambiar a modo Handheld (tecla M en PC, físicamente desacoplar JoyCons en Switch 2), la cámara cambia a cenital y el control pasa a una bola que se mueve mediante el giroscopio. Al resolver el puzzle aparece un cofre y luego una puerta hacia la victoria.
- **L_Victory**: pantalla final con créditos.

### Stack tecnológico

| Elemento | Versión / Detalles |
|---|---|
| Engine | Unreal Engine 5.7 (source build en `C:\UE57`) |
| IDE | Visual Studio 2022 |
| Lenguajes | C++ (sistemas) + Blueprint (UI y configuración) |
| Repositorio | GitHub con Git LFS para `.uasset` y `.umap` |
| Plataforma desarrollo | Windows 10/11 |
| Plataforma destino | Nintendo Switch 2 (devkit) |
| Assets visuales | Kenney mini-dungeon pack (GLB) |
| Asistente IA | Claude (chat para diseño + Claude Code para multi-file C++) |

### Alcance final

```
✅ Personaje jugable con cámara isométrica + Enhanced Input
✅ Combate cuerpo a cuerpo con animaciones, hit reactions direccionales y cooldowns
✅ Ataque especial activado por shake (mouse en PC, giroscopio en Switch)
✅ HUD shake feedback (icono JoyCon animado durante el especial)
✅ Enemigo orco con AI vía Behavior Tree
✅ Auto-fade walls con material Substrate
✅ Iluminación de dungeon con Lumen + antorchas dinámicas
✅ Audio con random selection en Sound Cues
✅ Sistema de Platform Mode (Docked/Handheld) con detección automática preparada para SDK
✅ Cambio de cámara + pawn al alternar modo (modo bola en L_Puzzle)
✅ Scalability automática (Medium en Handheld, Epic en Docked)
✅ Mensaje on-screen al cambiar de modo
✅ Puzzle con bola física manual y deslizamiento contra paredes
✅ Cofre animado one-shot (PlayAnimation)
✅ Puerta animada one-shot con condición de llave
✅ Sistema de mensajes HUD desacoplado
✅ Game Over con animación de muerte + Retry
✅ Transiciones entre niveles con fade
✅ Migración completa a assets Kenney (player, orco, props, niveles)
```

---

## 2. Setup del proyecto

### Requisitos

- **Unreal Engine 5.7** compilado desde source. Localización por defecto: `C:\UE57`.
- **Visual Studio 2022** con workload "Game development with C++" y "Desktop development with C++".
- **Git** + **Git LFS** configurado para `.uasset`, `.umap` y binarios.
- **GitHub CLI o SSH key** para acceso al repositorio privado.

### Estructura de carpetas

```
C:\dev\ProtoDungeonSW2\
├── Config\                     # .ini files del proyecto
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
│   ├── MPC\                    # MPC_Global con PlayerPosition
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

Notas clave:

- **`EnhancedInput`**: requerido para `UInputAction`, `UInputMappingContext`, `UEnhancedInputComponent`.
- **`AIModule` + `GameplayTasks` + `NavigationSystem`**: para `AAIController`, Behavior Trees, MoveTo.
- **`UMG`**: añadido tras el Game Over (necesario por `TSubclassOf<UUserWidget>` en el header del player). Sin esto, el linker falla con error LNK2019 en `UUserWidget`.

### Convenciones (CLAUDE.md)

El archivo `CLAUDE.md` en la raíz del proyecto define las convenciones:

| Aspecto | Regla |
|---|---|
| Prefijo de clases del proyecto | `DP` (ej. `ADPPlayerCharacter`, `UDPCombatComponent`) |
| Prefijos UE estándar | `A` para Actors, `U` para UObjects/Components, `F` para structs, `E` para enums |
| Idioma de comentarios | Español |
| Idioma de identificadores | Inglés |
| UPROPERTY editables | Siempre con Category específica |
| Punteros a UObject | `TObjectPtr<T>`, nunca `T*` raw |
| Includes cross-folder | Path relativo con `../`, p. ej. `"../Combat/DPCombatComponent.h"` |
| No shadowing | Nada de parámetros llamados `Instigator`, `Owner`, `Tags`, `Role` (chocan con miembros de `AActor`/`UObject`) |
| Commits | Conventional Commits (`feat:`, `fix:`, `refactor:`, `docs:`) |
| Engine | NO modificar `C:\UE57` salvo que se pida explícitamente |

### Comandos útiles

```powershell
# Regenerar archivos de Visual Studio tras cambiar Build.cs o añadir/quitar archivos
& "C:\UE57\Engine\Build\BatchFiles\GenerateProjectFiles.bat" -project="C:\dev\ProtoDungeonSW2\ProtoDungeonSW2.uproject" -game

# Lanzar Claude Code en el proyecto
cd C:\dev\ProtoDungeonSW2
claude

# Push de cambios
git status
git add .
git commit -m "feat(...): descripción"
git push
```

---

## 3. Niveles

### L_Combat

Sala con paredes Kenney en forma de L (entry + corredor + sala principal). El jugador spawnea en el entry, recorre el corredor hasta la sala donde le espera el orco. Al matar al orco, transición automática a L_Puzzle tras 2 segundos.

**Características técnicas:**

- **GameMode**: `BP_GameMode_Kenney` (Default Pawn Class = `BP_PlayerCharacter_Kenney`).
- **NavMesh**: cubre toda la zona transitable para que el AI del orco navegue.
- **Iluminación**: DirectionalLight tenue + SkyLight frío + ExponentialHeightFog para atmósfera. Antorchas (`BP_Torch`) con PointLight cálido (R=1, G=0.6, B=0.2) crean contraste.
- **Auto-fade walls**: las paredes frontales (orientadas hacia la cámara) usan `MI_Wall_Fade` y sus variantes. Se desvanecen cuando el jugador está cerca, usando `MPC_Global.PlayerPosition` + SphereMask.
- **Decoración**: banners Kenney en las paredes, barriles, antorchas, weapons y shields como props.

### L_Puzzle

Estructura **inusual**: dos sublevels físicos separados verticalmente en Z.

```
                  ┌─────────────────────────┐
                  │  Laberinto del puzzle   │   ← Z alto, cámara cenital
                  │  (visible solo en modo  │
                  │   Handheld con bola)    │
                  └─────────────────────────┘
                              ↓ separación vertical
                  ┌─────────────────────────┐
                  │  Sala Kenney principal  │   ← Z = 0, cámara isométrica
                  │  (entrada + cofre +     │
                  │   puerta)               │
                  └─────────────────────────┘
```

**Flujo del nivel:**

1. El jugador entra en la sala Kenney (modo Docked, cámara isométrica).
2. Ve una puerta cerrada y un mensaje de "necesitas la llave".
3. Pulsa M → cambia a Handheld → el `ADPPuzzlePlayerController` posee la bola, la cámara salta al `PuzzleCamera` (CameraActor cenital sobre el laberinto, Yaw -90).
4. Mueve el ratón → el giroscopio simulado inclina, la bola rueda por el laberinto. La bola **desliza** contra paredes (no se atasca) gracias a `FVector::VectorPlaneProject` sobre `Hit.ImpactNormal`.
5. Llega al hueco (`ADPHoleTrigger`) → la bola se destruye, modo cambia a Docked automáticamente.
6. De vuelta en la sala Kenney, aparece el cofre.
7. Interactúa con E → animación de apertura → `bPlayerHasKey = true` + mensaje "Has obtenido una llave".
8. Interactúa con la puerta con E → animación de apertura → tras la animación, fade out + transición a L_Victory.

**Iluminación dual:**

- Sala Kenney: PointLights cálidos en las paredes (antorchas).
- Laberinto del puzzle: 4 PointLights con `Cast Shadows: OFF`, color azul claro, intensidad 8000. Sin sombras para evitar artefactos de shadowmap dinámico con muchas luces solapadas.

**MPC fade fix crítico:** el `BP_PlayerCharacter` en su `Event Tick` escribe `GetActorLocation` al MPC solo si `ViewTarget == self`. Si la cámara está en otro lado (modo bola), escribe `(99999, 99999, 99999)` para que ninguna pared se desvanezca falsamente.

### L_Victory

Mapa minimalista con un único actor: un `BP_GameMode_Victory` con `DefaultPawnClass = nullptr`. El Level Blueprint crea el `WBP_Victory` y lo añade al viewport en BeginPlay.

**Por qué `DefaultPawnClass = nullptr`**: sin esto, el PlayerStart hace que UE intente spawnear un pawn, lo que en algunos casos arrastraba el HUD del player a la pantalla de victoria (bug del HUD residual).

**Estética del widget:** paleta amber/coral sobre marrón oscuro (`#2A1810`, `#EF9F27`, `#BA7517`, `#FAC775`). Fuentes Cinzel Bold (título) + Montserrat (subtitle/botones). Frase: *"The sphere rolled, the door opened, and somewhere a Joy-Con was shaken in vain."*

---

## 4. Sistemas C++

### 4.1 Characters

#### `ADPCharacterBase` (clase base)

Hereda de `ACharacter`. Define el contrato común para player y enemigos.

**Estado:**
- `MaxHealth` (default 100), `CurrentHealth`, `bIsDead`.
- `DeathAnimation` (`UAnimSequence*`): si está asignada, se reproduce en `OnDeath` vía `GetMesh()->PlayAnimation(DeathAnimation, false)` y mantiene el último frame.
- `bTriggersLevelTransitionOnDeath` + `NextLevelName`: si true, al morir solicita al `UDPLevelTransitionSubsystem` cargar el siguiente nivel (usado por el bull final de L_Combat).

**Combate:**
- `HitReactFrontMontage`, `HitReactBackMontage`: animaciones de reacción según dirección del atacante.
- `DamageSound`: gruñido al recibir daño no letal.
- Override de `TakeDamage` aplicado a la cadena: actualiza `CurrentHealth`, broadcast del delegate `OnHealthChanged`, reproduce hit reaction si sobrevive, llama a `OnDeath` si llega a 0.

**Detección de dirección del hit:**

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
1. Reproduce `DeathAnimation` (PlayAnimation, no Montage).
2. `Montage_Stop` para cancelar ataques en curso (evita que el orco te golpee post-muerte).
3. `SetActorEnableCollision(false)`.
4. `DisableMovement` en el `UCharacterMovementComponent`.
5. Si es controlado por AI, `Brain->StopLogic("Character died")`.
6. Si `bTriggersLevelTransitionOnDeath`, llama al subsystem de transición.

#### `ADPPlayerCharacter` (jugador)

Hereda de `ADPCharacterBase`. Añade cámara isométrica fija, Enhanced Input y el sistema de interacción/Game Over.

**Cámara:**
- `SpringArm`: `TargetArmLength = 2000`, rotación absoluta `(Pitch=-50, Yaw=-45, Roll=0)`. `bDoCollisionTest = false`, `bUseAbsoluteRotation = true`.
- `Camera`: anclada al socket del SpringArm. `FieldOfView = 60`.
- `bUseControllerRotationPitch/Yaw/Roll = false`. El personaje rota hacia el movimiento (`bOrientRotationToMovement = true`, `RotationRate = (0, 640, 0)`).

**Movimiento:**
- `MaxWalkSpeed = 500` (400 en variante Kenney por escala chibi).
- `Move(FInputActionValue)` proyecta el input 2D sobre el yaw de la cámara para que adelante/derecha sean coherentes con la vista isométrica.

**Interacción:**
- `InteractionSphere` (USphereComponent, radio 150): `WorldDynamic` + `Overlap WorldDynamic`. Mantiene un puntero `CurrentInteractable` al último `ADPInteractableBase` que entra en su rango.
- Al pulsar E (`IA_Interact`), si `CurrentInteractable != nullptr`, llama a `CurrentInteractable->Interact(this)` polimórficamente.

**Game Over:**
- `GameOverWidgetClass` (`TSubclassOf<UUserWidget>`): asignado a `WBP_GameOver` en el BP.
- `OnDeath` override: `Super::OnDeath()` → `DisableInput(PC)` → `SetTimer(GameOverTimerHandle, ShowGameOverScreen, DeathAnimation->GetPlayLength())`.
- `ShowGameOverScreen`: crea el widget, lo añade al viewport, `SetInputModeUIOnly` + `SetShowMouseCursor(true)` + `SetGamePaused(true)`.

**Defensa de input en `BeginPlay`:**

```cpp
if (APlayerController* PC = Cast<APlayerController>(GetController()))
{
    FInputModeGameOnly InputMode;
    PC->SetInputMode(InputMode);
    PC->SetShowMouseCursor(false);
    EnableInput(PC);
}
```

Esto previene un bug del viewport en PIE: tras un Retry, el InputMode del viewport queda contaminado en `UIOnly` y el nuevo player no recibe input. Defensiva en cada spawn.

#### `ADPEnemyBase`

Hereda de `ADPCharacterBase`. Configura:
- `AIControllerClass = ADPEnemyAIController::StaticClass()`.
- `bUseControllerRotationYaw = true` para que el AI rote al pawn al perseguir.
- `MaxWalkSpeed = 350` (orco un poco más lento que el player).
- Combat component con `BasicAttackMontage` apuntando a `AM_Attack_Basic_Orc`.

### 4.2 Combate

#### `UDPCombatComponent`

`UActorComponent` que añade combate a cualquier `ADPCharacterBase`. Tanto el player como el orco lo usan.

**Configuración expuesta:**
- `BasicAttackDamage` (25), `SpecialAttackDamage` (50).
- `BasicAttackRange` (80), `SpecialAttackRange` (95) — distancia desde el actor al centro de la esfera de daño.
- `AttackRadius` (45) — radio de la esfera de detección.
- `BasicAttackCooldown` (0.5), `SpecialAttackCooldown` (1.0).
- `BasicAttackMontage`, `SpecialAttackMontage`: si están asignados, el daño se aplica vía notify; si son nullptr, el daño es inmediato (flujo legacy).
- `HitImpactSound`: reproducido en la posición del enemigo impactado.
- `bDrawDebugAttacks`: muestra la esfera de detección durante 1s.

**Cooldowns inteligentes:**
- El básico está bloqueado si Now - LastBasic < BasicCD **o** si Now - LastSpecial < SpecialCD (el especial bloquea todo).
- El especial solo está bloqueado por su propio CD.

**Flujo de ataque con montage:**
1. `TryBasicAttack()` → verifica `CanBasicAttack()`, registra `LastBasicAttackTime`, reproduce montage en el character owner.
2. Marca `CurrentAttackType = Basic`.
3. El montage tiene un `AnimNotify_DamageMoment` en el frame del impacto visual.
4. Cuando el notify dispara, llama a `UDPCombatComponent::OnDamageNotify()` que ejecuta `PerformAttack(BasicDamage, BasicRange)`.
5. `PerformAttack` hace `SphereOverlapActors` filtrando por canal Pawn, ignora al owner, aplica `UGameplayStatics::ApplyDamage` a cada `ADPCharacterBase` encontrado, reproduce `HitImpactSound` en su posición.

**Daño:**

`UGameplayStatics::ApplyDamage` → llega a `ADPCharacterBase::TakeDamage` → decrementa vida → hit reaction o muerte.

### 4.3 AI

Behavior Tree clásico con Blackboard y servicios.

#### `ADPEnemyAIController`

Hereda de `AAIController`. En `OnPossess` activa el Behavior Tree:

```cpp
UBlackboardComponent* BBComp = nullptr;
if (UseBlackboard(BehaviorTreeAsset->BlackboardAsset, BBComp))
{
    RunBehaviorTree(BehaviorTreeAsset);
}
```

`BehaviorTreeAsset` es un `UPROPERTY(EditDefaultsOnly)` asignado a `BT_Enemy` desde `BP_EnemyAIController`.

#### `BB_Enemy` (Blackboard)

Variables:
- `TargetActor` (Object, Base Class Actor): referencia al jugador detectado.
- `bIsInAttackRange` (Bool): true si el target está a `AttackRange` o menos.
- `SelfActor` (auto, no borrar): puntero al pawn poseído.

#### `BT_Enemy` (Behavior Tree)

Estructura mínima del árbol:

```
Root
└── Selector
    ├── Sequence (atacar)
    │   ├── Decorator: bIsInAttackRange == true
    │   ├── Decorator: TargetActor != null
    │   └── BTTask_AttackTarget
    └── Sequence (perseguir)
        ├── Decorator: TargetActor != null
        └── MoveTo: TargetActor
```

Con un `BTService_UpdateTarget` corriendo en el Selector (tick 0.2s) que actualiza `TargetActor` y `bIsInAttackRange` constantemente.

#### `UDPBTTask_AttackTarget`

Custom Task que invoca `CombatComponent->TryBasicAttack()` del enemigo. Devuelve `Failed` si:
- No hay AIController o pawn.
- No hay `CombatComponent` en el pawn (`FindComponentByClass`).
- No hay `TargetActor` en el Blackboard.
- `CanBasicAttack()` devuelve false (cooldown activo).

Devuelve `Succeeded` tras llamar a `TryBasicAttack`.

#### `UDPBTService_UpdateTarget`

Custom Service que cada 0.2s:
1. Busca el `ADPPlayerCharacter` más cercano dentro del radio de detección.
2. Si lo encuentra: setea `TargetActor` en el Blackboard.
3. Calcula distancia al target y setea `bIsInAttackRange` si está dentro de `AttackRange`.
4. Si no hay target a la vista: setea `TargetActor` a null.

### 4.4 Input

Tres subsystems abstraen el input físico del juego, dejando preparado el camino para Switch 2.

#### `UDPShakeDetectorSubsystem`

`UGameInstanceSubsystem` que detecta agitación acumulada en una ventana temporal. Plataform-agnostic: PC le alimenta el delta del ratón, Switch le alimentará el delta del giroscopio.

**API:**
- `FeedInputDelta(FVector Delta)`: punto de entrada.
- `OnShakeDetected` (delegate): broadcast cuando supera threshold.
- Tunables: `ShakeWindowSeconds` (0.3), `ShakeThreshold` (50), `ShakeCooldown` (0.8).

**Algoritmo:**
1. Cada llamada a `FeedInputDelta` añade un sample `{timestamp, magnitude}` al historial.
2. Se limpian samples más antiguos que `ShakeWindowSeconds`.
3. Si la suma de magnitudes supera `ShakeThreshold`, dispara `OnShakeDetected` y arranca cooldown.
4. Durante el cooldown, los samples se ignoran.

En PC, `BP_PlayerCharacter` en su `Event Tick`, mientras `bIsTriggeringShake = true` (mantener click derecho), alimenta el subsystem con `GetInputMouseDelta`.

#### `UDPGyroInputSubsystem`

`UGameInstanceSubsystem` que mantiene un tilt 2D con auto-centrado pasivo. Usado por la bola del puzzle para calcular la dirección de "gravedad".

**API:**
- `FeedTiltDelta(FVector2D Delta)`: incremento de tilt.
- `ResetTilt()`: vuelve a (0, 0).
- `SetGyroActive(bool)`: el subsystem solo procesa input cuando activo.
- `CurrentTilt` (FVector2D, BlueprintReadOnly).
- `OnTiltChanged` (delegate).

**Tunables:**
- `TiltSensitivity` (0.01): multiplicador del delta entrante.
- `CenteringSpeed` (2.0): velocidad de auto-centrado.
- `MaxTilt` (1.0): clamp por componente.

**Auto-centrado pasivo:** un `FTSTicker` global ejecuta `TickCentering` cada frame. Mueve `CurrentTilt` hacia `(0,0)` con velocidad `CenteringSpeed * DeltaTime`. Sin esto, el tilt acumulado nunca volvería a cero al dejar de mover el ratón.

En PC, `ADPPuzzlePlayerController::PlayerTick` alimenta el subsystem con el delta del ratón mientras está en modo Handheld.

#### `UDPPlatformModeSubsystem`

`UGameInstanceSubsystem` que es la **fuente de verdad** del modo Docked/Handheld global.

**API:**
- `GetCurrentMode()`, `IsDocked()`, `IsHandheld()`.
- `SetPlatformMode(EDPPlatformMode)`: cambia el modo.
- `TogglePlatformMode()`: alterna.
- `OnPlatformModeChanged` (delegate): broadcast a todos los listeners.

**Comportamiento al cambiar de modo (orden importante):**
1. Actualiza `CurrentMode`.
2. **Aplica Scalability** según modo: `level 1` (Medium) en Handheld, `level 3` (Epic) en Docked. Persiste con `Scalability::SaveState(GGameUserSettingsIni)`.
3. **Solicita mensaje on-screen** vía `UDPMessageSubsystem`: "Docked mode" o "Handheld mode" durante 2s.
4. **Broadcast** del delegate.

Este orden garantiza que los listeners (cámara, gyro, etc.) ya operen bajo el nuevo perf budget.

**Detección automática (preparada para SDK):**

```cpp
EDPPlatformMode UDPPlatformModeSubsystem::QueryPlatformMode() const
{
#if PLATFORM_SWITCH
    // TODO Switch 2: reemplazar con la API real del SDK de Nintendo
    // #include <nn/oe.h>
    // const nn::oe::OperationMode Mode = nn::oe::GetOperationMode();
    // return (Mode == nn::oe::OperationMode_Handheld)
    //     ? EDPPlatformMode::Handheld
    //     : EDPPlatformMode::Docked;
    return EDPPlatformMode::Docked;  // Fallback hasta SDK disponible
#else
    return CurrentMode;
#endif
}
```

En builds de Switch, un `FTimerHandle` polling cada 0.5s ejecuta `QueryPlatformMode()` y si difiere del cached, llama a `SetPlatformMode()`. En PC el polling no se activa (toggle manual con tecla M).

#### Tecla M (toggle global)

`IA_TogglePlatform` está en `IMC_Default` (mapeado a tecla M). Ambos:
- `ADPPlayerCharacter::OnToggleModePressed` (en L_Combat) → `TogglePlatformMode()`.
- `ADPPuzzlePlayerController::HandleToggleMode` (en L_Puzzle) → `TogglePlatformMode()`.

Esto hace que la tecla M funcione **en ambos niveles**. En L_Combat el cambio solo afecta scalability + mensaje HUD (el gameplay sigue igual). En L_Puzzle, además, cambia cámara y pawn.

### 4.5 Puzzle

#### `ADPPuzzleBall`

`APawn` (no Character — no necesita movement component) con física manual. Vive en el laberinto del puzzle.

**Componentes:**
- `CollisionSphere` (USphereComponent): root, radio 50.
- `BallMesh` (UStaticMeshComponent): visual, attached al CollisionSphere.

**Tunables:**
- `Acceleration = 16000` (alto para feel ágil).
- `MaxSpeed = 500`.
- `Friction = 0.95` (multiplicador por segundo).
- `CurrentTilt` (FVector2D): asignado externamente desde el PlayerController.

**Tick (física manual):**

```cpp
// 1. Aplicar aceleración según tilt
FVector TiltAcceleration(CurrentTilt.X * Acceleration, CurrentTilt.Y * Acceleration, 0.f);
CurrentVelocity += TiltAcceleration * DeltaTime;

// 2. Fricción
CurrentVelocity *= FMath::Pow(Friction, DeltaTime);

// 3. Clamp a MaxSpeed
if (CurrentVelocity.SizeSquared() > MaxSpeed * MaxSpeed)
{
    CurrentVelocity = CurrentVelocity.GetSafeNormal() * MaxSpeed;
}

// 4. Mover con sweep para detectar colisión
FVector NewLocation = GetActorLocation() + CurrentVelocity * DeltaTime;
FHitResult Hit;
SetActorLocation(NewLocation, true, &Hit);

// 5. Slide contra paredes (no atascarse)
if (Hit.bBlockingHit)
{
    CurrentVelocity = FVector::VectorPlaneProject(CurrentVelocity, Hit.ImpactNormal);
}
```

`VectorPlaneProject` proyecta la velocidad sobre el plano perpendicular a la normal del impacto, permitiendo que la bola deslice contra paredes en lugar de quedar atascada.

#### `ADPPuzzlePlayerController`

PlayerController custom usado solo en L_Puzzle. Gestiona la posesión Docked/Handheld y alimenta el gyro.

**`BeginPlay`:**
1. `FindPawnsInLevel()`: busca el `ADPPlayerCharacter` y el `ADPPuzzleBall` en el nivel.
2. `FindPuzzleCameraInLevel()`: busca un `ACameraActor` con tag `"PuzzleCamera"`.
3. Registra el `InputMappingContext` (sobrevive al cambio de pawn).
4. Suscribe al `OnPlatformModeChanged`.
5. Llama a `OnPlatformModeChanged(SubSystem->CurrentMode)` para aplicar el modo inicial.

**`OnPlatformModeChanged(NewMode)`:**
- Activa/desactiva el gyro (`SetGyroActive(NewMode == Handheld)`).
- Si Handheld: `Possess(BallPawn)` + `SetViewTargetWithBlend(PuzzleCameraActor)`.
- Si Docked: `Possess(PlayerCharacterPawn)` + `SetViewTargetWithBlend(PlayerCharacterPawn)`.

**`PlayerTick`:**

```cpp
if (CachedGyroSubsystem == nullptr || !CachedGyroSubsystem->bGyroActive)
{
    return;
}

float MouseDeltaX = 0.f, MouseDeltaY = 0.f;
GetInputMouseDelta(MouseDeltaX, MouseDeltaY);

// Iteración para Camera Actor con Yaw=-90: mapeo directo Mouse X→X, Mouse Y→Y invertida
const FVector2D Delta(MouseDeltaX, -MouseDeltaY);
CachedGyroSubsystem->FeedTiltDelta(Delta);
```

**Por qué un PlayerController custom y no manejar todo desde `ADPPlayerCharacter`:** los input bindings en el pawn no sobreviven al cambio de posesión. El toggle modal (tecla M) debe estar en el controller para sobrevivir.

#### `ADPHoleTrigger`

Actor con `UBoxComponent` (root). Solo overlap, no bloquea.

`OnTriggerBeginOverlap`:
1. Verifica que el actor que entra es `ADPPuzzleBall`.
2. Notifica al `UDPPuzzleStateSubsystem`: `NotifyBallReachedGoal()`.
3. **Antes** de destruir la bola, cambia modo a Docked. Esto es crítico: el PlayerController necesita que **ambos** pawns sigan vivos para hacer `Possess(PlayerCharacterPawn)` correctamente.
4. `Ball->Destroy()`.

#### `UDPPuzzleStateSubsystem`

`UGameInstanceSubsystem`. Mantiene el estado del puzzle:

- `bBallReachedGoal`, `bPlayerHasKey`.
- `OnBallReachedGoal` (delegate broadcast cuando la bola llega al hueco).
- `SetPlayerHasKey(bool)`.

**Patrón importante**: los actores que tienen que reaccionar a "la bola llegó" (como el cofre o la llave original) se suscriben a `OnBallReachedGoal` en su `BeginPlay`. Tienen además un check defensivo "si ya disparó antes de mi spawn, dispara ahora" para soportar level streaming o spawn diferido.

#### `ADPInteractableBase`

Clase abstracta. Define el contrato:

```cpp
virtual void Interact(AActor* InteractingActor);
```

Todas las cosas con las que el player puede interactuar pulsando E (llaves, cofres, puertas) heredan de aquí.

**Detección polimórfica desde el player:**

```cpp
void ADPPlayerCharacter::OnInteractionSphereBeginOverlap(...)
{
    if (ADPInteractableBase* Interactable = Cast<ADPInteractableBase>(OtherActor))
    {
        CurrentInteractable = Interactable;
    }
}
```

Sin esto polimórfico (si en su lugar fuera `Cast<ADPPuzzleKey>`), añadir nuevos interactables como el cofre o la puerta requeriría tocar el código del player. Con la clase base, **basta con heredar y override Interact()**.

#### `ADPPuzzleChest`

Hereda de `ADPInteractableBase`. Cofre animado que aparece al resolver el puzzle.

**Componentes:**
- `ChestMesh` (`USkeletalMeshComponent`): root component. Configurado en el constructor con `QueryAndPhysics`, `WorldDynamic`, `Pawn=Block`, `WorldDynamic=Overlap`.

**Configurable en BP:**
- `OpenAnimation` (`UAnimSequence*`): la animación `chestopen` Kenney.
- `OnOpenMessage` ("Has obtenido una llave"), `OnOpenMessageDuration` (3.0s).

**Flujo:**
1. `BeginPlay`: oculta (`SetActorHiddenInGame(true)` + `SetActorEnableCollision(false)`) + suscribe a `OnBallReachedGoal`.
2. Cuando el delegate dispara: muestra + reactiva collision.
3. Al pulsar E (`Interact`): early-return si `bHasBeenOpened`, marca el flag, `ChestMesh->PlayAnimation(OpenAnimation, false)`, programa `FTimerHandle` con `OpenAnimation->GetPlayLength()`.
4. Cuando el timer dispara `OnOpenAnimationEnded`: `SetPlayerHasKey(true)` en el state subsystem + `RequestMessage` al `UDPMessageSubsystem`.

**Por qué `PlayAnimation` y no `Montage_Play`:** los montages devuelven el slot del AnimBP a la bind pose al terminar (cofre se cerraría visualmente). `PlayAnimation` con `Loop=false` mantiene el último frame. El cofre no necesita AnimBP — está siempre en estado A o B.

#### `ADPPuzzleDoor`

Hereda de `ADPInteractableBase`. Mismo patrón que el cofre con diferencias.

**Componentes:**
- `DoorMesh` (`USkeletalMeshComponent`): root, `QueryAndPhysics + WorldStatic + Block all`. Sólida hasta que se anima.
- `InteractionSphere` separada (`USphereComponent` radio 150): porque el mesh es Block y necesitamos un overlap aparte para detectar al player.

**Configurable en BP:**
- `OpenAnimation`: la animación `gateopen` Kenney.
- `NoKeyMessage` ("You need a key. Maybe you'll find it taking the bull by the joycons... I mean, horns."), `NoKeyMessageDuration` (6.0s).
- `bTriggersLevelTransitionOnOpen`, `NextLevelName` (`L_Victory`), `TransitionDelay` (0.5s).

**Flujo de `Interact`:**
- Si `bIsOpen`: return.
- Si `bPlayerHasKey == false`: `RequestMessage(NoKeyMessage, NoKeyMessageDuration)`.
- Si `bPlayerHasKey == true`: llama `OpenDoor()`.

**`OpenDoor`:**
1. `bIsOpen = true` PRIMERO (gates re-interacción durante la animación).
2. `DoorMesh->PlayAnimation(OpenAnimation, false)`.
3. `SetTimer(OpenAnimTimerHandle, OnOpenAnimationEnded, OpenAnimation->GetPlayLength())`.

**`OnOpenAnimationEnded`:**
1. `DoorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision)` (ahora el player puede atravesar).
2. Si `bTriggersLevelTransitionOnOpen`: `TransitionToLevel(NextLevelName, TransitionDelay, 0.5)`.

Diferir el cambio de collision y la transición a `OnAnimationEnded` (no a `OpenDoor`) asegura sincronización cinematográfica: el jugador no puede atravesar la puerta hasta que visualmente esté abierta.

### 4.6 Transiciones

#### `UDPLevelTransitionSubsystem`

`UGameInstanceSubsystem` que centraliza fade + delay + OpenLevel.

**API:**
```cpp
void TransitionToLevel(FName LevelName, float DelayBeforeFade = 0.f, float FadeDuration = 0.5f);
```

**Implementación:**
1. Guard `bTransitionInProgress` para evitar transiciones solapadas.
2. Si `DelayBeforeFade > 0`, `FTimerHandle` con delay.
3. `StartFadeAndLoad`: `PC->PlayerCameraManager->StartCameraFade(0, 1, FadeDuration, FLinearColor::Black, false, true)` (hold when finished).
4. Otro `FTimerHandle` para `ExecuteOpenLevel` tras `FadeDuration`.
5. `ExecuteOpenLevel`: limpia widgets del viewport (`Viewport->RemoveAllViewportWidgets()`) y `UGameplayStatics::OpenLevel(this, PendingLevelName)`.

**Quién dispara las transiciones:**
- L_Combat → L_Puzzle: `ADPCharacterBase::OnDeath` del orco bull con `bTriggersLevelTransitionOnDeath = true` (delay 2.0s).
- L_Puzzle → L_Victory: `ADPPuzzleDoor::OnOpenAnimationEnded` con `bTriggersLevelTransitionOnOpen = true` (delay 0.5s).
- Game Over Retry: `WBP_GameOver` botón Retry → recarga el level actual.

### 4.7 Mensajes HUD

#### `UDPMessageSubsystem`

`UGameInstanceSubsystem` (persiste entre niveles) con un único delegate y una función:

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMessageRequested, FString, Message, float, Duration);

UPROPERTY(BlueprintAssignable)
FOnMessageRequested OnMessageRequested;

UFUNCTION(BlueprintCallable)
void RequestMessage(const FString& Message, float Duration);
```

`RequestMessage` simplemente hace `OnMessageRequested.Broadcast(Message, Duration)`.

**El `WBP_PlayerHUD` se suscribe** a este delegate en su `Construct`. Cuando recibe, muestra el texto con fade in/out durante la duración.

**Quién usa el subsystem:**
- `ADPPuzzleChest`: "Has obtenido una llave" al abrir.
- `ADPPuzzleDoor`: "You need a key..." al intentar sin llave.
- `UDPPlatformModeSubsystem`: "Docked mode" / "Handheld mode" al cambiar.

Cualquier sistema futuro que necesite enseñar texto al jugador solo tiene que pedírselo al subsystem. Sin acoplamiento al HUD ni al player.

---

## 5. Sistemas Blueprint y assets

### 5.1 HUD

#### `WBP_PlayerHUD`

Widget UMG creado en `BP_PlayerCharacter::BeginPlay` y añadido al viewport.

**Elementos:**
- `HealthBar` (Progress Bar anclado top-left): color modulado según vida (verde 100%, amarillo 50%, rojo <25%). Se actualiza vía suscripción al delegate `OnHealthChanged` del player.
- `TextMessage` (TextBlock anclado center-bottom, fuente Cinzel): muestra mensajes del `UDPMessageSubsystem`. Tiene animaciones UMG `MessageFadeIn` y `MessageFadeOut` que reproducen al recibir/quitar mensaje.
- `Image_JoyConShake` (Image anclado center-bottom, encima del mensaje): icono de JoyCon que aparece durante el especial. Inicialmente Collapsed. Animación UMG `ShakeIconAnim` con keyframes de rotación + translation lateral (loop infinito mientras visible).

**Funciones públicas (llamadas desde BP_PlayerCharacter):**
- `UpdateHealth(Current, Max)`: actualiza la barra.
- `HandleMessage(Message, Duration)`: muestra el mensaje (interno, conectado al delegate).
- `ShowShakeIcon` / `HideShakeIcon`: gestiona el icono JoyCon.

#### `WBP_GameOver`

Widget de pantalla de muerte.

**Elementos:**
- `BackgroundOverlay` (Image fill, color negro 70% opacidad).
- `VerticalBox` centrado:
  - TextBlock "GAME OVER" (Cinzel Bold, size 96-128, rojo intenso).
  - Spacer.
  - Button `RetryButton` (300x80) con TextBlock "RETRY" dentro.

**`On Clicked (RetryButton)`:**
```
Set Game Paused (false)
→ Remove from Parent
→ Get Game Instance → Get Subsystem (DPLevelTransitionSubsystem)
→ Get Current Level Name → To Name
→ Transition To Level (Target: subsystem, LevelName, DelayBeforeFade: 0.5, FadeDuration: 0.5)
```

#### `WBP_Victory`

Widget de pantalla final.

**Diseño:**
- Fondo marrón oscuro `#2A1810`.
- Divisor superior dorado.
- Título "ProtoDungeon Switch 2" en Cinzel Bold 56, color `#EF9F27`.
- Subtitle "DEMO COMPLETED" en Montserrat 14, color `#BA7517`.
- Divisor central.
- Quote en Cinzel Italic 16, color `#FAC775`: *"The sphere rolled, the door opened, and somewhere a Joy-Con was shaken in vain."*
- Dos botones: "Play Again" (OpenLevel L_Combat) y "Quit" (Quit Game).

### 5.2 Materiales

#### `M_FadeOutWall` y `M_KenneyFadeWall`

Material Substrate (sistema moderno de UE5) con SphereMask para auto-fade cerca del jugador.

**Estructura del grafo (Substrate):**

```
[BaseColorTexture] ──► [Diffuse Albedo del SubstrateSlabBSDF]
                          │
[Constant 0.8] ─────────► [Roughness]
                          │
[WorldPosition] ─┐        │
                 ├──► [SphereMask] ──► [OneMinus] ──► [Substrate Coverage Weight] ──► [Front Material del Result]
[CollectionParameter] ─┘  (Radius, Hardness)
(MPC_Global.PlayerPosition)
```

**Material Instances** para texturas distintas:
- `MI_Wall_Fade`: textura `colormap` Kenney en la wall normal.
- `MI_WallHalf_Fade`, `MI_WallNarrow_Fade`, `MI_WallOpening_Fade`: variantes.

**Configuración del material:**
- `Blend Mode`: `TranslucentColoredTransmittance` (Substrate).
- `Lighting Mode`: `Surface TranslucencyVolume` para que reciba luz dinámica correctamente.
- `Two Sided`: **OFF** (importante: con Two Sided ON, paredes huecas Kenney mostraban artefactos visuales).

#### `MPC_Global`

Material Parameter Collection con:
- `PlayerPosition` (Vector parameter, default 0,0,0).

`BP_PlayerCharacter::Event Tick` lo escribe cada frame:

```
Branch: GetPlayerController.GetViewTarget == self ?
  True:  PlayerPosition = GetActorLocation
  False: PlayerPosition = (99999, 99999, 99999)
```

El else case previene fades falsos cuando la cámara está en otro pawn (modo bola).

### 5.3 Iluminación

**L_Combat:**
- `DirectionalLight`: rotación apuntando ligeramente desde arriba-detrás. Intensidad tenue. Color frío.
- `SkyLight`: ambient frío.
- `ExponentialHeightFog`: atmósfera oscura.
- 5+ `BP_Torch` posicionados estratégicamente: `PointLight` cálido `(R=1, G=0.6, B=0.2)`, `AttenuationRadius ≈ 1800`, Movable.

**L_Puzzle (zona Kenney superior):**
- Antorchas Kenney decorativas + algunos `PointLight` cálidos en las paredes para iluminación uniforme.

**L_Puzzle (zona laberinto inferior):**
- 4 `PointLight` con `Cast Shadows: OFF`, color azul claro, `Intensity 8000`, Movable. Sin sombras dinámicas para evitar artefactos cuando varias luces se solapan.

**Sistema:** Lumen (GI dinámico) — no requiere baking. Todas las luces marcadas como `Mobility = Movable`.

**Post-process volume** en L_Combat con `Min EV100 = 5` y `Max EV100 = 7` para fijar la auto-exposure (descubierto empíricamente cuando UE iluminaba en exceso).

### 5.4 Audio

Tres Sound Cues con Random Without Replacement:

| Sound Cue | Fuentes | Trigger |
|---|---|---|
| `SC_Swing` | `S_Swing_01`, `S_Swing_02` | Notify `Play Sound` al inicio del montage de ataque |
| `SC_Hit` | `S_Hit_01..04` | `UDPCombatComponent::PerformAttack` cuando hay impacto |
| `SC_Grunt` | `S_Grunt_01`, `S_Grunt_02` | `ADPCharacterBase::TakeDamage` con `DamageSound` |

**Patrón:** cada Sound Cue tiene los Sound Waves conectados a un nodo `Random` con `Without Replacement: true` (no repite el último). Output del Random al Output del Cue.

### 5.5 Animaciones

#### Montages del player Kenney (`character-human` skeleton)

| Montage | Anim base | Notifies |
|---|---|---|
| `AM_Attack_Basic_Kenney` | `humanattack-melee-right` (RateScale 0.6) | `DamageMoment` + `SC_Swing` |
| `AM_Attack_Special_Kenney` | `humanattack-kick-right` (RateScale 0.6) | `DamageMoment` + `SC_Swing` |
| `AM_HitReact_Kenney` | `humanfall` (BlendIn 0.05, BlendOut 0.1) | — |
| `AM_Die_Kenney` | `humandie` | — (no usado, se usa la AnimSequence directa) |

**Blend In/Out cortos** (0.05/0.1) en HitReact: la animación `humanfall` es muy corta, blends largos por defecto la "comían" antes de verla.

#### Montages del orco (`character-orc` skeleton)

| Montage | Anim base |
|---|---|
| `AM_Attack_Basic_Orc` | `orcattack-melee-right` |
| `AM_HitReact_Orc` | `orcfall` |
| `AM_Die_Orc` | `orcdie` (no usado, AnimSequence directa) |

#### AnimBPs

- `ABP_KenneyHuman`: State Machine con Idle/Walk transitioning por `Speed > 10`. `Slot 'DefaultSlot'` sobre el state machine para reproducir montages. Handler de `AnimNotify_DamageMoment` que casts el owner a `ADPCharacterBase` y llama al `UDPCombatComponent::OnDamageNotify`.
- `ABP_KenneyOrc`: similar al humano pero con la skeleton del orco.
- `ABP_KenneyChest`: solo Slot 'DefaultSlot' → Output Pose. Sin state machine (el cofre no necesita locomotion).

**Patrón importante**: el threshold de transición Idle/Walk es `Speed > 10` (no `> 0`) porque el CharacterMovement tiene ruido residual de velocidad que dispararía falsa transición.

### 5.6 Assets Kenney

Pack `mini-dungeon` importado como **GLB** (formato preferido para preservar materials y skeletons).

**Estructura tras importar:**
- `Content/Kenney/MiniDungeon/Meshes/<asset>/`: cada asset en su subcarpeta con SkeletalMesh, Skeleton, AnimSequences y Textures.
- `colormap` PNG compartido entre la mayoría de assets (atlas de colores).

**Material base de import:** `M_Default` (creado por el GLB importer) es Substrate-compatible y se usa como referencia para `M_KenneyFadeWall`.

---

## 6. Patrones de diseño reutilizables

### 6.1 Actor con animación one-shot que cambia de estado

**Cuándo usar:** actor que pasa de estado A a estado B persistente mediante una animación visual (cofre, puerta, muerte).

**Estructura:**

1. **RootComponent**: `USkeletalMeshComponent`, no `UStaticMeshComponent`.
2. **Animación**: `UAnimSequence*` (no `UAnimMontage`). Asignado vía UPROPERTY EditAnywhere.
3. **AnimBP del SkeletalMesh**: no se necesita. `PlayAnimation` bypasea el AnimGraph.
4. **Bool de estado**: `bIsOpen` / `bHasBeenOpened`, fuente de verdad para "ya se activó".
5. **`FTimerHandle`** para detectar el fin de la animación.

**Flujo de `Interact()`:**

```cpp
void AMyActor::Interact(AActor* InteractingActor)
{
    if (bIsOpen) return;       // Idempotencia
    bIsOpen = true;            // Marcar PRIMERO

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

**Handler del fin de animación**: todos los efectos persistentes (collision toggle, transición, subsystem updates, mensajes HUD) van **aquí**, no en `Interact()`. Sincronización feedback visual ↔ mecánica.

**Reglas de collision:**
- Constructor: `Block` contra Pawn para que el actor sea sólido en estado A.
- **No llamar `SetActorEnableCollision(false)` en Interact()**: el bool de estado ya previene re-interacción.
- Si el estado B requiere atravesar: `Mesh->SetCollisionEnabled(NoCollision)` en `OnAnimationEnded()`.

**Por qué AnimSequence y no AnimMontage:** los montages devuelven el slot a la bind pose al terminar. `PlayAnimation(Loop=false)` mantiene el último frame.

**Variante para Character con AnimBP:** mismo patrón pero `GetMesh()->PlayAnimation(...)` bypasea el AnimBP. Tras esto el actor no necesita volver al AnimBP (caso de muerte: al hacer Retry se recarga el nivel).

### 6.2 Subsystem como fuente de verdad

Para cualquier estado global del juego (puzzle state, platform mode, level transition, mensajes HUD), un `UGameInstanceSubsystem` centraliza:

- **Estado** (variables internas).
- **API** (funciones para mutar el estado).
- **Delegate** (broadcast a los listeners).

Los sistemas que reaccionan **se suscriben al delegate en su BeginPlay** y se desuscriben en EndPlay. No conocen a quien dispara el cambio, solo qué hacer cuando ocurre.

**Ejemplos en el proyecto:**

| Subsystem | Estado | Delegate |
|---|---|---|
| `UDPPlatformModeSubsystem` | `CurrentMode` | `OnPlatformModeChanged` |
| `UDPPuzzleStateSubsystem` | `bBallReachedGoal`, `bPlayerHasKey` | `OnBallReachedGoal` |
| `UDPMessageSubsystem` | (stateless, solo retransmite) | `OnMessageRequested` |
| `UDPLevelTransitionSubsystem` | `bTransitionInProgress` | (sin delegate, usa timers internos) |
| `UDPShakeDetectorSubsystem` | Historial de samples | `OnShakeDetected` |

**Ventaja:** añadir nuevos sistemas que reaccionan al estado no requiere modificar quien dispara el cambio. Bajo acoplamiento.

### 6.3 Interactable polimórfico

Una clase base `ADPInteractableBase` con `virtual void Interact(AActor* InteractingActor)`. El player detecta cualquier interactable por `Cast<ADPInteractableBase>(OtherActor)` en el overlap de su `InteractionSphere`.

**Para añadir un nuevo interactable:**
1. Heredar de `ADPInteractableBase`.
2. Override `Interact()`.

El player **no necesita modificarse**. El sistema crece sin tocar código existente.

### 6.4 Single source of input para acciones globales

La tecla M está en `IMC_Default` (un único Input Action `IA_TogglePlatform`). Ambos:
- `ADPPlayerCharacter` la bindea para `TogglePlatformMode()`.
- `ADPPuzzlePlayerController` la bindea para lo mismo.

El subsystem es la única fuente de verdad. Cualquiera puede llamar a `TogglePlatformMode()` y el sistema reacciona consistentemente.

### 6.5 Defensive `BeginPlay` para PIE

Tras un level reload (Retry, OpenLevel), el viewport del editor puede mantener estado contaminado (InputMode, cursor visibility, pause state). El `BeginPlay` del player reset defensivamente:

```cpp
if (APlayerController* PC = Cast<APlayerController>(GetController()))
{
    FInputModeGameOnly InputMode;
    PC->SetInputMode(InputMode);
    PC->SetShowMouseCursor(false);
    EnableInput(PC);
}
```

Sin coste si ya está bien. Salva si está mal.

---

## 7. Bugs documentados y soluciones

A lo largo del proyecto se documentaron **16 bugs principales** con su causa raíz y solución. Listados aquí para referencia futura.

### Bugs 1-5: AnimBP y animaciones

1. **`Speed > 0` en AnimBP dispara constantemente.** Causa: `UCharacterMovementComponent` tiene ruido residual de velocidad incluso parado. Fix: usar threshold `Speed > 10`.

2. **Falta nodo `Slot 'DefaultSlot'` en AnimGraph.** Síntoma: montages se reproducen pero no se ven. Fix: añadir `Slot 'DefaultSlot'` en el AnimGraph entre el state machine y `Output Pose`.

3. **`AnimNotify_DamageMoment` necesita handler en cada AnimBP.** Patrón: `Try Get Pawn Owner → Cast to DPCharacterBase → Get Component DPCombatComponent → OnDamageNotify`. Compartido entre player y enemigo.

4. **Blend In/Out 0.25 default eats short animations.** `humanfall` (~0.3s) se ve cortada. Fix: en el montage de hit react, `Blend In Time = 0.05`, `Blend Out Time = 0.1`.

5. **Anim Class no asignado al Mesh en child BP.** Síntoma: idle pose correcta pero `Try Get Pawn Owner` devuelve referencia distinta, velocidad lee como 0. Diagnóstico: `Event Tick + Print String` comparando velocity real (350) vs lectura del AnimBP (0).

### Bugs 6-8: Material y MPC

6. **Two Sided ON en `M_KenneyFadeWall` causa artefactos en paredes huecas.** Los muros Kenney son decorativos interiores (huecos), no sólidos como SM_Cube. Fix: desactivar Two Sided.

7. **`PlayerPosition` MPC necesita escribirse a (99999) en modo bola.** Sin esto, la última posición conocida del player queda fija y produce fades falsos en el laberinto. Fix: branch en Event Tick.

8. **Laberinto del puzzle en mismo Z que sala Kenney = cámara cenital ve a través.** Fix: separar verticalmente en Z. Originalmente el puzzle estaba arriba, luego se movió abajo. Con luces dedicadas (`Cast Shadows: OFF`) en el laberinto.

### Bugs 9-13: Cofre y puerta Kenney

9. **`USkeletalMeshComponent` arranca con `NoCollision` por defecto.** Al crear actor C++ con SkeletalMesh root, no genera overlap events ni recibe interacción. Fix: configurar collision en el constructor explícitamente (`QueryAndPhysics`, `WorldDynamic`, Pawn=Block, WorldDynamic=Overlap).

10. **Cofre se cierra visualmente tras `Montage_Play`.** Causa: el slot del AnimBP vuelve a bind pose. Fix: usar `USkeletalMeshComponent::PlayAnimation(AnimSequence, false)` que mantiene el último frame.

11. **Cofre intangible tras abrirse.** Causa: `SetActorEnableCollision(false)` en `Interact()` para "evitar re-interacción". Pero el flag `bHasBeenOpened` ya lo previene y desactivar collision rompe el block contra Pawn. Fix: eliminar esa línea.

12. **Pivote del SkeletalMesh Kenney no coincide con cubo placeholder.** Al migrar `DoorMesh` de StaticMesh (pivote centrado) a SkeletalMesh (pivote en base), la puerta queda desplazada. Fix: reajustar Transform manualmente en el nivel.

13. **Transición de nivel arranca a la vez que la animación de apertura.** Si llamas a `TransitionToLevel` desde `OpenDoor` directamente, el fade empieza antes de que la animación termine. Fix: diferir la transición a `OnOpenAnimationEnded` con el `FTimerHandle`.

### Bugs 14-16: Game Over y UI

14. **`Build.cs` sin UMG → CreateWidget falla al enlazar.** Hasta ahora UMG solo se usaba desde Blueprint. Al añadir `TSubclassOf<UUserWidget>` en el header del player, el linker no encuentra `UUserWidget::CreateWidgetInstance` ni `AddToViewport`. Fix: añadir `"UMG"` a `PublicDependencyModuleNames` + regenerar VS project files.

15. **Player vuelve sin input tras Retry en PIE.** Tras `SetInputModeUIOnly` antes del Game Over, el viewport del editor queda contaminado. Tras `OpenLevel`, el nuevo PC arranca con `FInputModeGameAndUI` pero el viewport sigue en UI mode. Fix: en `BeginPlay` del player, forzar `FInputModeGameOnly` + `EnableInput(PC)` defensivamente.

16. **DeathAnimation no se reproduce aunque el campo está asignado.** Causa: el campo es `UAnimSequence*` pero por error se asignó un `UAnimMontage` (UE lo permite visualmente). `PlayAnimation()` falla silenciosamente. Fix: usar siempre la AnimSequence raw (icono verde/azul), no el montage (icono rojo).

### Otros bugs menores documentados

- **Render Transform Scale del HUD escala desde pivot (0.5, 0.5)** y elementos anclados a esquinas se salen del viewport. No es realmente un bug, sino un uso incorrecto. Decisión: no escalar el HUD entre Docked/Handheld en este prototipo (sistema de DPI Scaling de UE ya escala según resolución de salida en builds reales).

- **`OpenLevel` no resetea pausa automáticamente en PIE.** El Retry del Game Over llama a `Set Game Paused(false)` explícitamente antes de transitar.

- **NuGet `coverlet.collector` 6.0.4 rompe `GenerateProjectFiles`** en algunos setups. Fix: `dotnet nuget locals all --clear` y regenerar.

- **Includes de UE necesitan path relativo con `../` entre subcarpetas.** UBT NO añade cada subcarpeta como include path. `#include "../Combat/DPCombatComponent.h"` desde `Characters/`.

---

## 8. Roadmap Switch 2 (deploy)

### Lo que ya está preparado

El proyecto está arquitectónicamente listo para Switch 2:

- **`#if PLATFORM_SWITCH` placeholders** en `UDPPlatformModeSubsystem::QueryPlatformMode()` con comentarios TODO referenciando la API esperada `nn::oe::GetOperationMode()`.
- **Polling cada 0.5s** en builds de Switch (timer ya configurado, solo polling actualmente devuelve fallback Docked).
- **Scalability automática** que reduce calidad en Handheld (level 1 Medium) y la sube en Docked (level 3 Epic).
- **Abstracción de input** vía subsystems (`UDPShakeDetectorSubsystem`, `UDPGyroInputSubsystem`) que separan "qué hacer con un shake/tilt" de "de dónde viene el input".

### Pasos para activar Switch 2 real

#### 1. Acceso al SDK de Nintendo

- Requiere ser developer registrado en el portal de Nintendo (https://developer.nintendo.com).
- Descargar el plugin de Switch 2 para UE 5.7 (NDA bajo developer agreement).
- Descomprimir en `Engine/Platforms/Switch/` o similar (la estructura exacta la documenta el SDK).

#### 2. Configurar el proyecto

- Project Settings → Platforms → Switch: marcar como soportado.
- `ProtoDungeonSW2.Target.cs`: añadir `Switch` a `SupportedPlatforms` si no está implícito.

#### 3. Activar la detección real de modo

En `UDPPlatformModeSubsystem::QueryPlatformMode()`, descomentar el bloque del SDK:

```cpp
#if PLATFORM_SWITCH
    #include <nn/oe.h>  // verificar nombre exacto en SDK
    const nn::oe::OperationMode Mode = nn::oe::GetOperationMode();
    return (Mode == nn::oe::OperationMode_Handheld)
        ? EDPPlatformMode::Handheld
        : EDPPlatformMode::Docked;
#else
    return CurrentMode;
#endif
```

**Verificar antes:**
- Nombre exacto del header (`<nn/oe.h>` o `<nn/oe2.h>` para Switch 2).
- Nombre exacto del enum (puede ser `nn::oe2::OperationMode` con valores ligeramente distintos).
- Si requiere `#include` adicional para el namespace en el módulo Build.cs.

#### 4. Build para Switch

```powershell
# Desde la raíz de UE:
.\Engine\Build\BatchFiles\RunUAT.bat BuildCookRun -project="C:\dev\ProtoDungeonSW2\ProtoDungeonSW2.uproject" -platform=Switch -clientconfig=Development -cook -allmaps -build -stage -pak
```

(Comando exacto puede variar según versión del SDK)

#### 5. Conectar JoyCons al sistema de shake/gyro

En PC, el `UDPShakeDetectorSubsystem` recibe el delta del ratón vía `FeedInputDelta(GetInputMouseDelta)` desde el Tick. En Switch:

```cpp
#if PLATFORM_SWITCH
    // En el Tick del player o un subsystem dedicado:
    const FVector2D AccelDelta = JoyConSDK::GetAccelerometerDelta();
    ShakeDetector->FeedInputDelta(FVector(AccelDelta.X, AccelDelta.Y, 0));
#endif
```

Lo mismo para el gyro:

```cpp
#if PLATFORM_SWITCH
    const FVector2D TiltDelta = JoyConSDK::GetGyroDelta();
    GyroSubsystem->FeedTiltDelta(TiltDelta);
#endif
```

**No hay que cambiar `ADPPuzzleBall` ni el resto del juego.** El subsystem se encarga de procesar el input, sea cual sea su fuente.

#### 6. Test en emulador

El compañero de oficina tiene un emulador de Switch 2 que simula docked/handheld. Plan:

1. Build el proyecto en modo Switch.
2. Cargar el `.nsp` (o formato que use el emulador) en el emulador.
3. Probar el juego completo:
   - L_Combat: combate con orco usando el control real.
   - Pulsar el botón de "desacoplar JoyCons" → debe activarse automáticamente Handheld → la HUD muestra "Handheld mode" → scalability baja.
   - En L_Puzzle, hacer lo mismo → además, el `ADPPuzzlePlayerController` cambia a la bola y la cámara cenital.
   - El gyro de los JoyCons reales debe mover la bola.

#### 7. Polish para Switch 2

Cosas que tendrán sentido revisar tras el primer build funcional:

- **HUD scaling**: si el HUD se ve pequeño en modo portátil (pantalla 7.9" LCD), aplicar Render Scale 1.15 al `WBP_PlayerHUD` con un `Scale Box` que respete las anchors. (No se hizo en PC porque `Set Render Scale` simple rompía las anchors).
- **Joy-Con rumble**: añadir feedback háptico a impactos de combate, shake del especial, cofre abriendo, puerta abriendo.
- **Touch input** opcional en handheld (interactuar con cofres tocando la pantalla).
- **Iconos de JoyCon reales** para el HUD shake feedback (sustituir el placeholder generado).
- **Profile de rendimiento**: medir el frame time en Handheld y ajustar scalability si es necesario.

### Información que necesitamos del SDK de Switch 2

Cuando llegues al devkit, anota:

1. Nombre exacto de la macro de preprocesador para SW2 (`PLATFORM_SWITCH` o `PLATFORM_SWITCH2`).
2. Header del SDK para detección de modo (`<nn/oe.h>`, `<nn/oe2.h>`, etc.).
3. Nombre exacto del enum y sus valores (`nn::oe::OperationMode_Handheld` vs alternativas).
4. API de input para JoyCons (gyro, acelerómetro, rumble).
5. ¿Requiere algún módulo Build.cs adicional?

---

## 9. Workflow y filosofía

### C++ vs Blueprint

**C++ para:**
- Subsystems (estado global, lifecycle gestionado).
- Clases base (`ADPCharacterBase`, `ADPInteractableBase`).
- Componentes reutilizables (`UDPCombatComponent`).
- GameModes.
- Lógica de física manual (`ADPPuzzleBall`).
- AI Tasks/Services custom.

**Blueprint para:**
- UI (Widgets UMG).
- Configuración de instancias (asignar AnimSequence, Widget Classes, Montages).
- Level Blueprints.
- Materiales (Material Editor visual).
- Lógica que requiera iterar rápido sin recompilar.

**Patrón "tunear en BP, trasladar a C++":**
1. Itera valores en BP en runtime (sin recompilar).
2. Cuando los valores se confirman, trasládalos al constructor de C++ (la fuente de verdad).
3. Resetea overrides en BP (botón "Reset to Default" en el property) para que herede del C++ correctamente.

Ejemplo: cámara isométrica. Se tuneó en BP (`Pitch -50`, `ArmLength 2000`), luego se trasladó al constructor de `ADPPlayerCharacter::ADPPlayerCharacter()`.

### Workflow IA

**Chat (claude.ai):**
- Diseño arquitectónico, dudas conceptuales.
- Revisión de código aislado.
- Debugging con stack trace.
- Generación de clase nueva desde cero.
- Conversación, exploración.

**Claude Code (terminal):**
- Cambios multi-file.
- Refactors.
- Builds + leer errores del compilador automáticamente.
- Scaffolding completo (header + cpp).
- Modificación de archivos existentes con cambios precisos.

**Patrón de prompts a Claude Code (heredado de este proyecto):**
- Descripción de tarea + reglas (no compiles, muéstrame diff, no toques X).
- Convenciones (DP prefix, Spanish comments, English identifiers).
- Restricciones explícitas (no tocar Build.cs salvo que se indique, no tocar otros archivos).
- Verificaciones a hacer antes (leer X.cpp para entender la API actual).

### Iteración: probar → tunear → consolidar

Filosofía repetida en cada bloque:

1. **Implementa lo mínimo funcional.**
2. **Pruébalo en runtime.**
3. **Tunea por sensación, no por números teóricos.**
4. **Cuando se siente bien, consolida** (trasladar valor a C++, hacer commit, documentar).

Ejemplos:
- Cámara: pitch -30 → probar → no se ve el corredor → -50 → bien → trasladar a C++.
- Bola: Acceleration 980 → muy lenta → 16000 → bien.
- ShakeThreshold: 80 → exige demasiado → 50 → bien.

### Commits Conventional Commits

Patrón:

```
<type>(<scope>): <subject>

<body opcional con detalles>
```

Tipos usados en el proyecto:
- `feat`: nueva funcionalidad.
- `fix`: arreglo de bug.
- `refactor`: cambio de código sin alterar comportamiento.
- `docs`: solo documentación.
- `chore`: cambios de tooling.

Ejemplos del proyecto:
- `feat(puzzle): add ADPPuzzleChest with Kenney chest mesh and animation`
- `feat(player): add game over flow with death animation and retry`
- `feat(platform): wire UDPPlatformModeSubsystem into L_Combat and add auto-scalability`
- `refactor(camera): move tuned isometric camera values to C++ constructor`
- `docs(claude): add one-shot animation actor pattern to CLAUDE.md`

### Sesiones cerradas

Cada sesión de trabajo termina con:

1. **Commit del trabajo realizado**.
2. **Push a GitHub**.
3. **Resumen del estado** (qué se logró, qué falta).
4. **Anticipación del próximo paso**.

Si una sesión queda a medias, no commitear código que no compila. Mejor `git stash` o terminar el bloque pequeño antes de cerrar.

---

## 10. Referencia rápida

### Comandos clave

```powershell
# Regenerar archivos de VS tras cambiar Build.cs
& "C:\UE57\Engine\Build\BatchFiles\GenerateProjectFiles.bat" -project="C:\dev\ProtoDungeonSW2\ProtoDungeonSW2.uproject" -game

# Lanzar Claude Code
cd C:\dev\ProtoDungeonSW2 && claude

# Build desde terminal (alternativa a VS)
& "C:\UE57\Engine\Build\BatchFiles\Build.bat" ProtoDungeonSW2Editor Win64 Development -project="C:\dev\ProtoDungeonSW2\ProtoDungeonSW2.uproject"

# Limpiar caché de NuGet si GenerateProjectFiles falla
dotnet nuget locals all --clear
```

### Hotkeys del juego

| Tecla | Acción |
|---|---|
| WASD | Movimiento del player |
| Click izquierdo | Ataque básico |
| Click derecho (mantener) + mover ratón | Ataque especial (simulando shake JoyCon) |
| E | Interactuar (cofre, puerta) |
| **M** | Toggle Docked / Handheld mode |

### Clases principales y sus responsabilidades

| Clase | Responsabilidad |
|---|---|
| `ADPCharacterBase` | Vida, daño, hit reactions, muerte con animación. Base para player y enemigos. |
| `ADPPlayerCharacter` | Player con cámara isométrica, Enhanced Input, interacción, Game Over. |
| `ADPEnemyBase` | Enemigo controlado por AI. |
| `ADPPuzzleBall` | Pawn de la bola con física manual. |
| `UDPCombatComponent` | Lógica de combate (ataques, cooldowns, daño). |
| `ADPEnemyAIController` | AI controller que ejecuta el Behavior Tree. |
| `ADPInteractableBase` | Clase base abstracta para cosas con las que interactuar (E). |
| `ADPPuzzleChest` | Cofre animado one-shot. |
| `ADPPuzzleDoor` | Puerta animada one-shot con condición de llave. |
| `ADPHoleTrigger` | Trigger que dispara al caer la bola. |
| `ADPPuzzlePlayerController` | PlayerController custom del L_Puzzle (gestiona posesión Docked/Handheld). |
| `UDPShakeDetectorSubsystem` | Detecta agitación (mouse en PC, gyro en Switch). |
| `UDPGyroInputSubsystem` | Tilt 2D con auto-centering. |
| `UDPPlatformModeSubsystem` | Fuente de verdad del modo Docked/Handheld. |
| `UDPPuzzleStateSubsystem` | Estado del puzzle (bola en hueco, llave obtenida). |
| `UDPLevelTransitionSubsystem` | Fade + delay + OpenLevel centralizado. |
| `UDPMessageSubsystem` | Mensajes HUD desacoplados. |

### Tunables clave por sistema

| Sistema | Valor | Por qué |
|---|---|---|
| Cámara isométrica | Pitch -50, ArmLength 2000, FOV 60 | Vista clara del dungeon Kenney |
| Player walk speed | 500 (400 Kenney) | Sensación de combate ágil |
| Basic attack damage | 25 | 4 golpes para matar al orco |
| Special attack damage | 50 | El doble que el básico, justifica el cooldown |
| Basic attack cooldown | 0.5s | Combos sin spam |
| Special attack cooldown | 1.0s | Bloquea TODO durante ese segundo |
| Attack range / radius | 80 / 45 (Kenney chibi) | Hitbox razonable para escala chibi |
| ShakeWindow / Threshold | 0.3s / 50 | Movimiento claro sin ser punitivo |
| Ball Acceleration | 16000 | Feel ágil tuneado por sensación |
| Ball MaxSpeed | 500 | No sale del laberinto a velocidades absurdas |
| Ball Friction | 0.95 | Frenado natural sin sentirse pegajoso |
| Transition fade | 0.5s | Cinematográfico sin ser lento |
| Death animation delay | `DeathAnimation->GetPlayLength()` | UI aparece sincronizada con la animación |

### Workflow por adquisición de funcionalidad nueva

Cuando añadas un sistema nuevo al proyecto, este patrón ha funcionado bien:

1. **Diseño en chat**: discutir arquitectura, decidir clase base, definir API pública.
2. **Decisiones explícitas**: ¿C++ o Blueprint? ¿Subsystem o componente? ¿Delegate o callback directo?
3. **Prompt para Claude Code**: con todas las decisiones, generar el código en una pasada.
4. **Revisar diff**: verificar que sigue las convenciones (DP prefix, Spanish comments, includes con `../`).
5. **Compilar**: si falla, leer el error con calma. Suele ser `Build.cs` o un include faltante.
6. **Configurar en editor**: asignar UPROPERTIES en BPs, conectar a niveles.
7. **Test en runtime**: tunear valores por sensación.
8. **Consolidar**: trasladar valores tuneados a C++.
9. **Commit** con Conventional Commits.
10. **Documentar** si emerge un patrón nuevo (en CLAUDE.md).

---

## Cierre

ProtoDungeonSW2 es un prototipo completo end-to-end. Demuestra:

- **Combate** con animaciones, AI, audio, hit reactions, muerte cinematográfica.
- **Mecánica única de Switch 2**: cambio físico Docked/Handheld con cambio de cámara, control y pawn.
- **Polish técnico**: auto-fade walls Substrate, scalability automática, HUD desacoplado, transiciones cinemáticas.
- **Arquitectura sólida**: subsystems como fuente de verdad, patrones reutilizables (one-shot animation, interactable polimórfico), código preparado para SDK.

El proyecto es la base sobre la que se puede iterar para un juego completo o como showcase técnico para Nintendo. Cada decisión está documentada (este archivo, los CLAUDE.md, los 16 bugs y sus fixes) para que el siguiente developer (o tú mismo en 6 meses) pueda retomar sin perder contexto.

**Próximos pasos opcionales:**
- Deploy en devkit Switch 2 real.
- Rumble en JoyCons reales.
- Más niveles (combat + puzzle alternating).
- Audio ambient en los niveles.
- Música.
- Más enemigos.
- Sistema de progresión.

---

*Documento generado al cierre del proyecto. Última actualización: junio 2026.*
