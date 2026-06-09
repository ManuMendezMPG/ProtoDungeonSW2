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

	// Solid skeletal mesh as root: blocks the player until the door opens.
	// The mesh and animation (Kenney's gate / gateopen) are assigned in the BP.
	DoorMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("DoorMesh"));
	RootComponent = DoorMesh;

	DoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	DoorMesh->SetCollisionObjectType(ECC_WorldStatic);
	DoorMesh->SetCollisionResponseToAllChannels(ECR_Block);

	// Interaction sphere: overlap only, doesn't block
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(RootComponent);
	InteractionSphere->InitSphereRadius(150.f);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	// CRITICAL: WorldDynamic must respond too so the player's InteractionSphere
	// (WorldDynamic) detects this sphere. Without this the overlap doesn't fire.
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
	// Mark as open first to prevent re-interaction during the animation.
	bIsOpen = true;

	// PlayAnimation with Loop=false holds the last frame: the door stays open on screen.
	// We schedule a timer because PlayAnimation does not fire OnMontageEnded.
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
		// Fallback: if no animation is assigned, run the final logic
		// directly so the mechanic doesn't get stuck
		OnOpenAnimationEnded();
	}
}

void ADPPuzzleDoor::OnOpenAnimationEnded()
{
	// Disable collision only now: the player couldn't cross the door while
	// it's opening, but they can once it's fully open.
	if (DoorMesh)
	{
		DoorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Trigger the level transition if this door is the level exit.
	if (bTriggersLevelTransitionOnOpen && NextLevelName != NAME_None)
	{
		if (UDPLevelTransitionSubsystem* TransitionSubsystem = GetGameInstance()->GetSubsystem<UDPLevelTransitionSubsystem>())
		{
			TransitionSubsystem->TransitionToLevel(NextLevelName, TransitionDelay, 0.5f);
		}
	}
}
