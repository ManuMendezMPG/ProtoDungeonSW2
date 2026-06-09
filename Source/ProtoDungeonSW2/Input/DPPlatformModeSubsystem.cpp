#include "DPPlatformModeSubsystem.h"
#include "../UI/DPMessageSubsystem.h"
#include "CoreGlobals.h"
#include "Engine/World.h"
#include "Scalability.h"
#include "TimerManager.h"

void UDPPlatformModeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initial state: query the SDK on Switch, or Docked by default on PC
	CurrentMode = QueryPlatformMode();

#if PLATFORM_SWITCH
	// On Switch we start polling every 0.5s to detect when the
	// user physically undocks/docks the console. On PC this is not
	// needed because toggling is manual via TogglePlatformMode().
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			PollingTimerHandle, this,
			&UDPPlatformModeSubsystem::PollPlatformMode,
			0.5f, true);
	}
#endif
}

void UDPPlatformModeSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PollingTimerHandle);
	}

	OnPlatformModeChanged.Clear();
	Super::Deinitialize();
}

void UDPPlatformModeSubsystem::SetPlatformMode(EDPPlatformMode NewMode)
{
	if (NewMode == CurrentMode)
	{
		return;
	}

	CurrentMode = NewMode;

	// Apply the quality preset appropriate for the mode. In Switch 2 handheld
	// the GPU/CPU clock drops, so we reduce quality to keep
	// framerate; in docked we push to max.
	// Levels: 0=Low, 1=Medium, 2=High, 3=Epic, 4=Cinematic
	Scalability::FQualityLevels Quality = Scalability::GetQualityLevels();
	const int32 TargetLevel = (CurrentMode == EDPPlatformMode::Handheld) ? 1 : 3;
	Quality.SetFromSingleQualityLevel(TargetLevel);
	Scalability::SetQualityLevels(Quality);
	Scalability::SaveState(GGameUserSettingsIni);

	// Visual feedback for the mode change via the HUD message subsystem
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UDPMessageSubsystem* MessageSubsystem =
			GI->GetSubsystem<UDPMessageSubsystem>())
		{
			const FString ModeMessage = (CurrentMode == EDPPlatformMode::Handheld)
				? TEXT("Handheld mode")
				: TEXT("Docked mode");
			MessageSubsystem->RequestMessage(ModeMessage, 2.0f);
		}
	}

	OnPlatformModeChanged.Broadcast(CurrentMode);
}

void UDPPlatformModeSubsystem::TogglePlatformMode()
{
	const EDPPlatformMode NextMode = (CurrentMode == EDPPlatformMode::Docked)
		? EDPPlatformMode::Handheld
		: EDPPlatformMode::Docked;

	SetPlatformMode(NextMode);
}

EDPPlatformMode UDPPlatformModeSubsystem::QueryPlatformMode() const
{
#if PLATFORM_SWITCH
	// TODO Switch 2: replace this placeholder with the real Nintendo SDK
	// API once it's available on the devkit. The likely namespace and
	// function are:
	//
	//   #include <nn/oe.h>
	//   const nn::oe::OperationMode Mode = nn::oe::GetOperationMode();
	//   return (Mode == nn::oe::OperationMode_Handheld)
	//       ? EDPPlatformMode::Handheld
	//       : EDPPlatformMode::Docked;
	//
	// Verify against the Switch 2 SDK (the namespace may have changed
	// to nn::oe2 or similar). In the meantime, we return Docked as a
	// safe fallback.
	return EDPPlatformMode::Docked;
#else
	// On PC there's no automatic detection: the mode stays as it is.
	return CurrentMode;
#endif
}

void UDPPlatformModeSubsystem::PollPlatformMode()
{
	const EDPPlatformMode DetectedMode = QueryPlatformMode();
	if (DetectedMode != CurrentMode)
	{
		SetPlatformMode(DetectedMode);
	}
}
