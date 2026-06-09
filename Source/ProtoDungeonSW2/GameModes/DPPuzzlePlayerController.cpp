#include "DPPuzzlePlayerController.h"
#include "../Characters/DPPlayerCharacter.h"
#include "../Characters/DPPuzzleBall.h"
#include "../Input/DPGyroInputSubsystem.h"
#include "../Input/DPPlatformModeSubsystem.h"
#include "Camera/CameraActor.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"

ADPPuzzlePlayerController::ADPPuzzlePlayerController()
{
	// Empty constructor. Initialization happens in BeginPlay because we need the world.
}

void ADPPuzzlePlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Find the pawns in the map.
	FindPawnsInLevel();

	// Find the maze Camera Actor (tag "PuzzleCamera").
	FindPuzzleCameraInLevel();

	// Register the controller's mapping context (survives pawn changes)
	if (InputMappingContext != nullptr)
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			InputSubsystem->AddMappingContext(InputMappingContext, 0);
		}
	}

	// Subscribe to mode changes.
	if (UDPPlatformModeSubsystem* PlatformSubsystem = GetGameInstance()->GetSubsystem<UDPPlatformModeSubsystem>())
	{
		PlatformSubsystem->OnPlatformModeChanged.AddDynamic(this, &ADPPuzzlePlayerController::OnPlatformModeChanged);

		// Cache the gyro subsystem for use in PlayerTick.
		CachedGyroSubsystem = GetGameInstance()->GetSubsystem<UDPGyroInputSubsystem>();

		// On startup the mode is Docked, so we possess the player character.
		OnPlatformModeChanged(PlatformSubsystem->CurrentMode);
	}
}

void ADPPuzzlePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (ToggleModeAction != nullptr)
		{
			EIC->BindAction(ToggleModeAction, ETriggerEvent::Started, this, &ADPPuzzlePlayerController::HandleToggleMode);
		}
	}
}

void ADPPuzzlePlayerController::HandleToggleMode()
{
	if (UDPPlatformModeSubsystem* PlatformSubsystem = GetGameInstance()->GetSubsystem<UDPPlatformModeSubsystem>())
	{
		PlatformSubsystem->TogglePlatformMode();
	}
}

void ADPPuzzlePlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// Only feed the gyro if it's active (Handheld mode).
	if (CachedGyroSubsystem == nullptr || !CachedGyroSubsystem->bGyroActive)
	{
		return;
	}

	float MouseDeltaX = 0.f;
	float MouseDeltaY = 0.f;
	GetInputMouseDelta(MouseDeltaX, MouseDeltaY);

	// Iteration for Camera Actor with Yaw=-90 (candidate 2): direct mapping Mouse X→X, Mouse Y→Y.
	const FVector2D Delta(MouseDeltaX, -MouseDeltaY);
	CachedGyroSubsystem->FeedTiltDelta(Delta);
}

void ADPPuzzlePlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Cleanup: unsubscribe from the subsystem.
	if (UGameInstance* GameInst = GetGameInstance())
	{
		if (UDPPlatformModeSubsystem* PlatformSubsystem = GameInst->GetSubsystem<UDPPlatformModeSubsystem>())
		{
			PlatformSubsystem->OnPlatformModeChanged.RemoveDynamic(this, &ADPPuzzlePlayerController::OnPlatformModeChanged);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ADPPuzzlePlayerController::FindPawnsInLevel()
{
	// Find the ADPPlayerCharacter in the map.
	TArray<AActor*> FoundCharacters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADPPlayerCharacter::StaticClass(), FoundCharacters);
	if (FoundCharacters.Num() > 0)
	{
		PlayerCharacterPawn = Cast<ADPPlayerCharacter>(FoundCharacters[0]);
	}

	// Find the ADPPuzzleBall in the map.
	TArray<AActor*> FoundBalls;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADPPuzzleBall::StaticClass(), FoundBalls);
	if (FoundBalls.Num() > 0)
	{
		BallPawn = Cast<ADPPuzzleBall>(FoundBalls[0]);
	}
}

void ADPPuzzlePlayerController::FindPuzzleCameraInLevel()
{
	TArray<AActor*> FoundCameras;
	UGameplayStatics::GetAllActorsOfClassWithTag(
		GetWorld(),
		ACameraActor::StaticClass(),
		FName("PuzzleCamera"),
		FoundCameras
	);

	if (FoundCameras.Num() > 0)
	{
		PuzzleCameraActor = Cast<ACameraActor>(FoundCameras[0]);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DPPuzzlePlayerController: No CameraActor with tag 'PuzzleCamera' found in level. Camera switch will fall back to ball."));
	}
}

void ADPPuzzlePlayerController::OnPlatformModeChanged(EDPPlatformMode NewMode)
{
	// Enable/disable gyro based on the mode (independent of pawn state).
	if (CachedGyroSubsystem != nullptr)
	{
		CachedGyroSubsystem->SetGyroActive(NewMode == EDPPlatformMode::Handheld);
	}

	// If we haven't found the pawns yet, do nothing (the map isn't ready yet).
	if (PlayerCharacterPawn == nullptr || BallPawn == nullptr)
	{
		return;
	}

	if (NewMode == EDPPlatformMode::Handheld)
	{
		// Take control of the ball.
		Possess(BallPawn);

		// ViewTarget to the maze Camera Actor; fallback to the ball if none
		AActor* TargetViewTarget = (PuzzleCameraActor != nullptr) ? Cast<AActor>(PuzzleCameraActor) : Cast<AActor>(BallPawn);
		SetViewTargetWithBlend(TargetViewTarget, 0.0f, EViewTargetBlendFunction::VTBlend_EaseInOut);
	}
	else // Docked
	{
		// Take control of the player character.
		Possess(PlayerCharacterPawn);

		// ViewTarget back to the player character (its isometric camera)
		SetViewTargetWithBlend(PlayerCharacterPawn, 0.0f, EViewTargetBlendFunction::VTBlend_EaseInOut);
	}
}
