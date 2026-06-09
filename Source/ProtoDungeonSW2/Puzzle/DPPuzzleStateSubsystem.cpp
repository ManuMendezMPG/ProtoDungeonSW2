#include "DPPuzzleStateSubsystem.h"

void UDPPuzzleStateSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	bBallReachedGoal = false;
	bPlayerHasKey    = false;
}

void UDPPuzzleStateSubsystem::Deinitialize()
{
	OnBallReachedGoal.Clear();
	Super::Deinitialize();
}

void UDPPuzzleStateSubsystem::NotifyBallReachedGoal()
{
	// Avoid double firing if the trigger activates more than once
	if (bBallReachedGoal)
	{
		return;
	}

	bBallReachedGoal = true;
	OnBallReachedGoal.Broadcast();
}

void UDPPuzzleStateSubsystem::SetPlayerHasKey(bool bHasKey)
{
	bPlayerHasKey = bHasKey;
}
