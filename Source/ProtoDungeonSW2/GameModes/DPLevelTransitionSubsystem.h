#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DPLevelTransitionSubsystem.generated.h"

UCLASS()
class PROTODUNGEONSW2_API UDPLevelTransitionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Starts a transition to a map with an optional delay before the fade.
	// DelayBeforeFade: seconds to wait before starting the fade (useful to give death animations etc time to play).
	// FadeDuration: duration of the fade to black before loading.
	UFUNCTION(BlueprintCallable, Category = "Transition")
	void TransitionToLevel(FName LevelName, float DelayBeforeFade = 0.f, float FadeDuration = 0.5f);

protected:
	// Internal state to avoid multiple overlapping transitions.
	bool bTransitionInProgress = false;

	// Destination map after the fade (stored during the delay).
	FName PendingLevelName;

	// Starts the fade and schedules OpenLevel.
	void StartFadeAndLoad(float FadeDuration);

	// Runs OpenLevel at the end of the fade.
	void ExecuteOpenLevel();
};
