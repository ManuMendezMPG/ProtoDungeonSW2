#include "DPPuzzleChest.h"
#include "DPPuzzleStateSubsystem.h"
#include "../UI/DPMessageSubsystem.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"

ADPPuzzleChest::ADPPuzzleChest()
{
	PrimaryActorTick.bCanEverTick = false;

	// Skeletal mesh as root: replaces the default scene root.
	ChestMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ChestMesh"));
	RootComponent = ChestMesh;
}

void ADPPuzzleChest::BeginPlay()
{
	Super::BeginPlay();

	// Initial state: hidden and with no collision until the subsystem activates us.
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);

	if (UDPPuzzleStateSubsystem* PuzzleState = GetGameInstance()->GetSubsystem<UDPPuzzleStateSubsystem>())
	{
		PuzzleState->OnBallReachedGoal.AddDynamic(this, &ADPPuzzleChest::OnBallReachedGoalHandler);

		// Defensive: if the event already fired before this chest existed, show it directly.
		if (PuzzleState->bBallReachedGoal)
		{
			ShowChest();
		}
	}
}

void ADPPuzzleChest::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unsubscribe to avoid dangling pointers if the subsystem outlives the actor.
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UDPPuzzleStateSubsystem* PuzzleState = GI->GetSubsystem<UDPPuzzleStateSubsystem>())
		{
			PuzzleState->OnBallReachedGoal.RemoveDynamic(this, &ADPPuzzleChest::OnBallReachedGoalHandler);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ADPPuzzleChest::OnBallReachedGoalHandler()
{
	ShowChest();
}

void ADPPuzzleChest::ShowChest()
{
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
}

void ADPPuzzleChest::Interact(AActor* InteractingActor)
{
	// You can't open a chest twice. The flag is the only source of truth;
	// we don't touch collision so the chest keeps blocking the player after opening.
	if (bHasBeenOpened)
	{
		return;
	}

	bHasBeenOpened = true;

	// PlayAnimation with Loop=false holds the last frame: the chest stays open on screen.
	// We schedule a timer because PlayAnimation does not fire OnMontageEnded.
	if (OpenAnimation && ChestMesh)
	{
		ChestMesh->PlayAnimation(OpenAnimation, false);
		const float Duration = OpenAnimation->GetPlayLength();
		GetWorldTimerManager().SetTimer(
			OpenAnimTimerHandle, this,
			&ADPPuzzleChest::OnOpenAnimationEnded,
			Duration, false);
	}
	else
	{
		// Fallback: if no animation is assigned, run the final logic
		// directly so the puzzle doesn't get stuck
		OnOpenAnimationEnded();
	}
}

void ADPPuzzleChest::OnOpenAnimationEnded()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UDPPuzzleStateSubsystem* PuzzleState = GI->GetSubsystem<UDPPuzzleStateSubsystem>())
		{
			PuzzleState->SetPlayerHasKey(true);
		}

		if (UDPMessageSubsystem* MessageSubsystem = GI->GetSubsystem<UDPMessageSubsystem>())
		{
			MessageSubsystem->RequestMessage(OnOpenMessage, OnOpenMessageDuration);
		}
	}
}
