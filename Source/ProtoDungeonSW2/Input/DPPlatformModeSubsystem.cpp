#include "DPPlatformModeSubsystem.h"

void UDPPlatformModeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CurrentMode = EDPPlatformMode::Docked;
}

void UDPPlatformModeSubsystem::Deinitialize()
{
	OnPlatformModeChanged.Clear();
	Super::Deinitialize();
}

void UDPPlatformModeSubsystem::TogglePlatformMode()
{
	const EDPPlatformMode Next = (CurrentMode == EDPPlatformMode::Docked)
		? EDPPlatformMode::Handheld
		: EDPPlatformMode::Docked;

	SetPlatformMode(Next);
}

void UDPPlatformModeSubsystem::SetPlatformMode(EDPPlatformMode NewMode)
{
	if (NewMode == CurrentMode)
	{
		return;
	}

	CurrentMode = NewMode;
	OnPlatformModeChanged.Broadcast(CurrentMode);
}
