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
	// Constructor vacío. La inicialización va en BeginPlay porque necesitamos el mundo.
}

void ADPPuzzlePlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Buscar los pawns en el mapa.
	FindPawnsInLevel();

	// Buscar el Camera Actor del laberinto (tag "PuzzleCamera").
	FindPuzzleCameraInLevel();

	// Registrar el mapping context del controller (sobrevive al cambio de pawn)
	if (InputMappingContext != nullptr)
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			InputSubsystem->AddMappingContext(InputMappingContext, 0);
		}
	}

	// Suscribirse al cambio de modo.
	if (UDPPlatformModeSubsystem* PlatformSubsystem = GetGameInstance()->GetSubsystem<UDPPlatformModeSubsystem>())
	{
		PlatformSubsystem->OnPlatformModeChanged.AddDynamic(this, &ADPPuzzlePlayerController::OnPlatformModeChanged);

		// Cachear el gyro subsystem para uso en PlayerTick.
		CachedGyroSubsystem = GetGameInstance()->GetSubsystem<UDPGyroInputSubsystem>();

		// Al iniciar, el modo es Docked, así que poseemos al player character.
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

	// Solo alimentamos el gyro si está activo (modo Handheld).
	if (CachedGyroSubsystem == nullptr || !CachedGyroSubsystem->bGyroActive)
	{
		return;
	}

	float MouseDeltaX = 0.f;
	float MouseDeltaY = 0.f;
	GetInputMouseDelta(MouseDeltaX, MouseDeltaY);

	// Iteración para Camera Actor con Yaw=-90 (candidato 2): mapeo directo Mouse X→X, Mouse Y→Y.
	const FVector2D Delta(MouseDeltaX, -MouseDeltaY);
	CachedGyroSubsystem->FeedTiltDelta(Delta);
}

void ADPPuzzlePlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Limpieza: desuscribirse del subsistema.
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
	// Buscar el ADPPlayerCharacter en el mapa.
	TArray<AActor*> FoundCharacters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADPPlayerCharacter::StaticClass(), FoundCharacters);
	if (FoundCharacters.Num() > 0)
	{
		PlayerCharacterPawn = Cast<ADPPlayerCharacter>(FoundCharacters[0]);
	}

	// Buscar el ADPPuzzleBall en el mapa.
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
	// Activar/desactivar gyro según el modo (independiente del estado de los pawns).
	if (CachedGyroSubsystem != nullptr)
	{
		CachedGyroSubsystem->SetGyroActive(NewMode == EDPPlatformMode::Handheld);
	}

	// Si todavía no hemos encontrado los pawns, no hacemos nada (todavía no está listo el mapa).
	if (PlayerCharacterPawn == nullptr || BallPawn == nullptr)
	{
		return;
	}

	if (NewMode == EDPPlatformMode::Handheld)
	{
		// Tomar control de la bola.
		Possess(BallPawn);

		// ViewTarget al Camera Actor del laberinto; fallback a la bola si no hay
		AActor* TargetViewTarget = (PuzzleCameraActor != nullptr) ? Cast<AActor>(PuzzleCameraActor) : Cast<AActor>(BallPawn);
		SetViewTargetWithBlend(TargetViewTarget, 0.0f, EViewTargetBlendFunction::VTBlend_EaseInOut);
	}
	else // Docked
	{
		// Tomar control del player character.
		Possess(PlayerCharacterPawn);

		// ViewTarget de vuelta al player character (su cámara isométrica)
		SetViewTargetWithBlend(PlayerCharacterPawn, 0.0f, EViewTargetBlendFunction::VTBlend_EaseInOut);
	}
}
