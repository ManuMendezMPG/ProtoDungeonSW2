#include "DPPuzzlePlayerController.h"
#include "../Characters/DPPlayerCharacter.h"
#include "../Characters/DPPuzzleBall.h"
#include "../Input/DPPlatformModeSubsystem.h"
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

	// Suscribirse al cambio de modo.
	if (UDPPlatformModeSubsystem* PlatformSubsystem = GetGameInstance()->GetSubsystem<UDPPlatformModeSubsystem>())
	{
		PlatformSubsystem->OnPlatformModeChanged.AddDynamic(this, &ADPPuzzlePlayerController::OnPlatformModeChanged);

		// Al iniciar, el modo es Docked, así que poseemos al player character.
		OnPlatformModeChanged(PlatformSubsystem->CurrentMode);
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
