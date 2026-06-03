#include "DPPuzzleDoor.h"
#include "DPPuzzleStateSubsystem.h"
#include "../GameModes/DPLevelTransitionSubsystem.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "TimerManager.h"
#include <ProtoDungeonSW2/UI/DPMessageSubsystem.h>

ADPPuzzleDoor::ADPPuzzleDoor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Skeletal mesh sólido como root: bloquea al player hasta que la puerta se abre.
	// El mesh y la animación (gate / gateopen de Kenney) se asignan en el BP.
	DoorMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("DoorMesh"));
	RootComponent = DoorMesh;

	DoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	DoorMesh->SetCollisionObjectType(ECC_WorldStatic);
	DoorMesh->SetCollisionResponseToAllChannels(ECR_Block);

	// Esfera de interacción: solo overlap, no bloquea
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(RootComponent);
	InteractionSphere->InitSphereRadius(150.f);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	// CRÍTICO: WorldDynamic también debe responder para que la InteractionSphere
	// del player (WorldDynamic) detecte esta esfera. Sin esto el overlap no dispara.
	InteractionSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	InteractionSphere->SetGenerateOverlapEvents(true);
}

void ADPPuzzleDoor::Interact(AActor* InteractingActor)
{
	if (bIsOpen)
	{
		return;
	}

	if (UDPPuzzleStateSubsystem* PuzzleState = GetGameInstance()->GetSubsystem<UDPPuzzleStateSubsystem>())
	{
		if (PuzzleState->bPlayerHasKey)
		{
			OpenDoor();
		}
		else
		{
			if (UGameInstance* GI = GetGameInstance())
			{
				if (UDPMessageSubsystem* MessageSubsystem = GI->GetSubsystem<UDPMessageSubsystem>())
				{
					MessageSubsystem->RequestMessage(NoKeyMessage, NoKeyMessageDuration);
				}
			}
		}
	}
}

void ADPPuzzleDoor::OpenDoor()
{
	// Marcar abierta primero para evitar re-interacción durante la animación.
	bIsOpen = true;

	// PlayAnimation con Loop=false mantiene el último frame: la puerta se queda abierta en pantalla.
	// Programamos un timer porque PlayAnimation no dispara OnMontageEnded.
	if (OpenAnimation && DoorMesh)
	{
		DoorMesh->PlayAnimation(OpenAnimation, false);
		const float Duration = OpenAnimation->GetPlayLength();
		GetWorldTimerManager().SetTimer(
			OpenAnimTimerHandle, this,
			&ADPPuzzleDoor::OnOpenAnimationEnded,
			Duration, false);
	}
	else
	{
		// Fallback: si no hay animación asignada, ejecuta el final
		// directamente para que la mecánica no se quede colgada
		OnOpenAnimationEnded();
	}
}

void ADPPuzzleDoor::OnOpenAnimationEnded()
{
	// Desactivar colisión sólo ahora: el player no podría atravesar la puerta mientras
	// se está abriendo, pero sí puede hacerlo una vez está totalmente abierta.
	if (DoorMesh)
	{
		DoorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Disparar transición de nivel si esta puerta es la salida del nivel.
	if (bTriggersLevelTransitionOnOpen && NextLevelName != NAME_None)
	{
		if (UDPLevelTransitionSubsystem* TransitionSubsystem = GetGameInstance()->GetSubsystem<UDPLevelTransitionSubsystem>())
		{
			TransitionSubsystem->TransitionToLevel(NextLevelName, TransitionDelay, 0.5f);
		}
	}
}
