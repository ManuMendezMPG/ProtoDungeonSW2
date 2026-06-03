\# ProtoDungeonSW2 — Contexto para Claude



\## Proyecto

Dungeon crawler isométrico, prototipo de 2 niveles, target Nintendo Switch 2.

Desarrollado en PC (Windows) con Unreal Engine 5.7 compilado desde source.



\## Niveles planificados

\- \*\*Nivel 1 (L\_Combat):\*\* combate con movimiento por stick, ataque básico (botón) y 

&#x20; ataque especial agitando JoyCon (simulado con click de ratón en PC).

\- \*\*Nivel 2 (L\_Puzzle):\*\* puzzle que conmuta modo dock (cámara isométrica) y modo 

&#x20; portátil (cámara cenital + giroscopio que mueve una bola por un laberinto).



\## Engine

\- UE 5.7 source build en C:\\UE57 (junction).

\- NO modificar código del engine salvo que se pida explícitamente.

\- Build config habitual: Development Editor | Win64.



\## Proyecto en disco

\- Ruta: C:\\dev\\ProtoDungeonSW2

\- Nombre del módulo: ProtoDungeonSW2

\- Macro API del módulo: PROTODUNGEONSW2\_API



\## Repositorio

\- GitHub (https://github.com/ManuMendezMPG/ProtoDungeonSW2), main.

\- Git LFS configurado para .uasset, .umap y binarios habituales.

\- Ignorar: Binaries/, Intermediate/, DerivedDataCache/, Saved/, .vs/



\## Convenciones de código

\- Prefijo de clases del proyecto: DP (ej: ADPPlayerCharacter, UDPCombatComponent).

\- Prefijos estándar UE: A para Actors, U para UObjects/Components, F para structs, E para enums.

\- Todos los UPROPERTY editables deben tener Category específica.

\- Usar TObjectPtr<T> en lugar de T\* en miembros UObject.

\- Logs con categorías propias: DECLARE\_LOG\_CATEGORY\_EXTERN + DEFINE\_LOG\_CATEGORY.

\- Comentarios en español, código (identificadores, strings) en inglés.



\## Módulos

\- Módulo principal: ProtoDungeonSW2

\- Dependencias actuales: Core, CoreUObject, Engine, InputCore

\- Dependencias que añadiremos según necesidad: EnhancedInput, PhysicsCore, UMG



\## Input

\- Usamos Enhanced Input (NO el sistema viejo).

\- IMCs separados por contexto (Combat, Puzzle) y uno global (Default).



\## Plataforma

\- Dock/portátil y giroscopio se simulan en PC para desarrollo:

&#x20; - Toggle dock/portátil: tecla (por definir).

&#x20; - Giroscopio: ratón o WASD alternativo.

\- Abstracción vía Subsystems para que el código gameplay no sepa de plataforma.

\- El SDK de Switch 2 está en C:\\UE57\\Engine\\Platforms\\Switch2 (presente pero no activo en desarrollo PC).



\## Criterios de aceptación de código generado

\- Compila sin warnings en Development Editor.

\- Incluye headers correctos y GENERATED\_BODY().

\- UPROPERTY/UFUNCTION con specifiers correctos (EditDefaultsOnly vs EditAnywhere, 

&#x20; BlueprintReadOnly vs BlueprintReadWrite, etc.).

\- Separación clara entre header (.h) y implementación (.cpp).

\- Forward declarations en headers cuando sea posible para minimizar includes.



\## Workflow con IA

\- Para clases nuevas: pedir header + cpp juntos, incluir Build.cs si cambia.

\- Para cambios multi-archivo: usar Claude Code; para dudas conceptuales: chat.

\- Commits frecuentes y pequeños cuando algo compila.


## Includes en UE (regla verificada empíricamente)

Los includes dentro del módulo usan rutas relativas al archivo fuente:

- Archivo en MISMA carpeta: #include "Archivo.h"
- Archivo en carpeta DISTINTA: usar ../ para subir y bajar
  Ejemplo: desde GameModes/Algo.cpp incluir Characters/Otro.h:
  #include "../Characters/Otro.h"

Para módulos externos (engine), los paths son relativos a la raíz pública 
de ese módulo, no requieren ../:
  #include "GameFramework/Character.h"
  #include "Camera/CameraComponent.h"

Nota: Si en el futuro queremos paths "absolutos" desde raíz del módulo sin 
../ relativos, se puede añadir PublicIncludePaths en ProtoDungeonSW2.Build.cs.

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

## Patrón: actor con animación one-shot que cambia de estado

Cuando un actor del puzzle (o cualquier otro sistema) necesita 
reproducir una animación que lo lleva de un estado A a un estado B 
persistente — por ejemplo cofre cerrado → abierto, puerta cerrada → 
abierta —, seguir este patrón.

Referencias en el código: `ADPPuzzleChest`, `ADPPuzzleDoor`.

### Estructura

1. **RootComponent**: `USkeletalMeshComponent`, no `UStaticMeshComponent`. 
   El skeletal es necesario para reproducir animaciones aunque el actor 
   no necesite AnimBP.

2. **Animación**: `UAnimSequence*` asignado vía `UPROPERTY EditAnywhere`. 
   No usar `UAnimMontage` para este caso: los montages devuelven el slot 
   a la bind pose cuando terminan, lo cual revierte visualmente el 
   estado.

3. **AnimBP del SkeletalMesh**: no se necesita. `PlayAnimation()` pone 
   el `SkeletalMeshComponent` en modo `AnimationSingleNode` y bypasea 
   el AnimGraph.

4. **Bool de estado**: `bool bIsOpen` / `bool bHasBeenOpened`, 
   `UPROPERTY BlueprintReadOnly`. Es la fuente de verdad para "ya se 
   activó", no la collision ni la visibilidad.

5. **FTimerHandle**: privado, para detectar el fin de la animación. 
   `PlayAnimation` no dispara `OnMontageEnded` (no usa montages); 
   usamos un timer manual con la duración real de la animación.

### Flujo de Interact()

```cpp
void AMyActor::Interact(AActor* InteractingActor)
{
    if (bIsOpen) return;       // Idempotencia: no re-activar
    bIsOpen = true;            // Marcar PRIMERO para bloquear re-entradas

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
        // Fallback: si falta la animación o el mesh, no queremos que 
        // la mecánica se quede colgada. Llamamos al handler de fin de 
        // animación directamente.
        OnAnimationEnded();
    }
}
```

### Handler del fin de animación

Todo el efecto **persistente** (cambio de collision, transición de 
nivel, set de flag en subsystem, broadcast de mensaje, etc.) va aquí, 
**nunca dentro de Interact() directamente**. Así el feedback visual 
y la mecánica están sincronizados.

```cpp
void AMyActor::OnAnimationEnded()
{
    // Efectos persistentes: collision toggle, transición, subsystem, 
    // mensaje HUD, etc.
}
```

### Reglas de collision

- **Constructor**: configurar el mesh como Block contra Pawn 
  (`WorldStatic` u `WorldDynamic` según corresponda) para que el 
  player no atraviese el actor mientras está en estado A.
- **No llamar `SetActorEnableCollision(false)` en Interact()**: el 
  bool de estado ya previene re-interacción. Desactivar toda la 
  collision rompe el bloqueo físico.
- **Si el estado B requiere que el player atraviese el actor** 
  (caso de la puerta): hacer `Mesh->SetCollisionEnabled(NoCollision)` 
  en `OnAnimationEnded()`, no antes.
- **Si el actor debe estar oculto antes de un evento de revelación** 
  (caso del cofre con `OnBallReachedGoal`): en `BeginPlay()` hacer 
  `SetActorHiddenInGame(true)` + `SetActorEnableCollision(false)`. En 
  el handler del delegate de revelación, volver a activar ambos.

### Conexión con UDPMessageSubsystem

Si el actor debe mostrar un mensaje al jugador (al abrirse, al fallar 
una condición, etc.), pedirlo al subsystem desde Interact() o desde 
OnAnimationEnded() según el caso:

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

No usar `GEngine->AddOnScreenDebugMessage` en lógica de gameplay — 
ese canal es solo para debug temporal.

### Variante para Character con AnimBP

Cuando el actor es un ACharacter con AnimBP completo (state machine 
locomotion + montages) pero también tiene una animación one-shot que 
debe persistir (muerte del player, muerte de enemigos), aplicar el 
mismo patrón:

- La animación va como UAnimSequence (no UAnimMontage). PlayAnimation 
  bypasea el AnimBP y mantiene el último frame.
- Se asume que tras esta animación el actor no necesita volver al 
  AnimBP. Si tuviera que reanimarse, habría que restaurar el modo 
  con SetAnimationMode(EAnimationMode::AnimationBlueprint) + reasignar 
  el AnimClass.

Referencia: ADPCharacterBase::OnDeath() reproduce DeathAnimation 
mediante GetMesh()->PlayAnimation(DeathAnimation, false).

### Combinación con UI tras la animación

Cuando la animación culmina con UI (Game Over, victoria, mensaje 
modal):

1. Programar el widget con FTimerHandle con Delay = 
   AnimSequence->GetPlayLength() para sincronizar.
2. Al mostrar el widget: SetInputModeUIOnly + SetShowMouseCursor(true) 
   + SetGamePaused(true).
3. Al cerrar el widget (botón Retry, Continue, etc.): SetGamePaused(false) 
   + RemoveFromParent + acción siguiente.
4. En BeginPlay del Character spawneado tras un reload defensivamente: 
   SetInputModeGameOnly + SetShowMouseCursor(false) + EnableInput(PC). 
   El viewport del editor (PIE) puede mantener el InputMode anterior y 
   bloquear input al nuevo player.

Referencia: ADPPlayerCharacter::OnDeath() / ShowGameOverScreen() / 
BeginPlay().