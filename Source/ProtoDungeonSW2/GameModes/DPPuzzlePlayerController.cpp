#include "DPPuzzlePlayerController.h"
#include "../Characters/DPPlayerCharacter.h"
#include "../Characters/DPPuzzleBall.h"
#include "../Input/DPPlatformModeSubsystem.h"
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

void ADPPuzzlePlayerController::OnPlatformModeChanged(EDPPlatformMode NewMode)
{
	// Si todavía no hemos encontrado los pawns, no hacemos nada (todavía no está listo el mapa).
	if (PlayerCharacterPawn == nullptr || BallPawn == nullptr)
	{
		return;
	}

	if (NewMode == EDPPlatformMode::Handheld)
	{
		// Tomar control de la bola.
		Possess(BallPawn);
	}
	else // Docked
	{
		// Tomar control del player character.
		Possess(PlayerCharacterPawn);
	}
}
