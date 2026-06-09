#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DPPuzzlePlayerController.generated.h"

class ACameraActor;
class ADPPlayerCharacter;
class ADPPuzzleBall;
class UDPGyroInputSubsystem;
class UInputAction;
class UInputMappingContext;
enum class EDPPlatformMode : uint8;

UCLASS()
class PROTODUNGEONSW2_API ADPPuzzlePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ADPPuzzlePlayerController();

protected:
	// Player character pawn. Found in the map on init.
	UPROPERTY(BlueprintReadOnly, Category = "Puzzle")
	TObjectPtr<ADPPlayerCharacter> PlayerCharacterPawn;

	// Ball pawn. Found in the map on init.
	UPROPERTY(BlueprintReadOnly, Category = "Puzzle")
	TObjectPtr<ADPPuzzleBall> BallPawn;

	// Input context registered in BeginPlay (assign the asset from the PlayerController BP).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	// Action that toggles Docked/Handheld. Survives pawn changes because it lives on the controller.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ToggleModeAction;

	// Cached reference to the gyro subsystem so we don't look it up every frame in PlayerTick.
	UPROPERTY()
	TObjectPtr<UDPGyroInputSubsystem> CachedGyroSubsystem;

	// Maze Camera Actor (looked up by tag "PuzzleCamera" in BeginPlay).
	UPROPERTY()
	TObjectPtr<ACameraActor> PuzzleCameraActor;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;

	// Called when the subsystem's mode changes. Switches possession to the corresponding pawn.
	UFUNCTION()
	void OnPlatformModeChanged(EDPPlatformMode NewMode);

	// Helpers to find the pawns in the map.
	void FindPawnsInLevel();

	// Looks for an ACameraActor tagged "PuzzleCamera" in the level.
	void FindPuzzleCameraInLevel();

private:
	void HandleToggleMode();
};
