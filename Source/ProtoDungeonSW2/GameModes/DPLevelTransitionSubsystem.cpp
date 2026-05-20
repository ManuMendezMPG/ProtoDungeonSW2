#include "DPLevelTransitionSubsystem.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

void UDPLevelTransitionSubsystem::TransitionToLevel(FName LevelName, float DelayBeforeFade, float FadeDuration)
{
	if (bTransitionInProgress)
	{
		return;
	}

	bTransitionInProgress = true;
	PendingLevelName = LevelName;

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		bTransitionInProgress = false;
		return;
	}

	if (DelayBeforeFade > 0.f)
	{
		FTimerHandle DelayHandle;
		World->GetTimerManager().SetTimer(
			DelayHandle,
			FTimerDelegate::CreateUObject(this, &UDPLevelTransitionSubsystem::StartFadeAndLoad, FadeDuration),
			DelayBeforeFade,
			false
		);
	}
	else
	{
		StartFadeAndLoad(FadeDuration);
	}
}

void UDPLevelTransitionSubsystem::StartFadeAndLoad(float FadeDuration)
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	// Fade a negro vía PlayerCameraManager
	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		if (PC->PlayerCameraManager != nullptr)
		{
			PC->PlayerCameraManager->StartCameraFade(
				0.f,                 // from alpha (transparente)
				1.f,                 // to alpha (totalmente negro)
				FadeDuration,
				FLinearColor::Black,
				false,               // bShouldFadeAudio
				true                 // bHoldWhenFinished (mantener negro tras terminar)
			);
		}
	}

	// Programar OpenLevel cuando termine el fade
	FTimerHandle LoadHandle;
	World->GetTimerManager().SetTimer(
		LoadHandle,
		FTimerDelegate::CreateUObject(this, &UDPLevelTransitionSubsystem::ExecuteOpenLevel),
		FadeDuration,
		false
	);
}

void UDPLevelTransitionSubsystem::ExecuteOpenLevel()
{
	UGameplayStatics::OpenLevel(this, PendingLevelName);
	bTransitionInProgress = false;
}
