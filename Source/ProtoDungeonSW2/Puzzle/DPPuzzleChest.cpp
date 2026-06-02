#include "DPPuzzleChest.h"
#include "DPPuzzleStateSubsystem.h"
#include "../UI/DPMessageSubsystem.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"

ADPPuzzleChest::ADPPuzzleChest()
{
	PrimaryActorTick.bCanEverTick = false;

	// Skeletal mesh como root: reemplaza el scene root por defecto.
	ChestMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ChestMesh"));
	RootComponent = ChestMesh;
}

void ADPPuzzleChest::BeginPlay()
{
	Super::BeginPlay();

	// Estado inicial: oculto y sin collision hasta que el subsystem nos active.
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);

	if (UDPPuzzleStateSubsystem* PuzzleState = GetGameInstance()->GetSubsystem<UDPPuzzleStateSubsystem>())
	{
		PuzzleState->OnBallReachedGoal.AddDynamic(this, &ADPPuzzleChest::OnBallReachedGoalHandler);

		// Defensivo: si el evento ya se disparó antes de que este cofre existiera, mostrar directamente.
		if (PuzzleState->bBallReachedGoal)
		{
			ShowChest();
		}
	}
}

void ADPPuzzleChest::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Desuscribirse para evitar punteros colgantes si el subsystem sobrevive al actor.
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UDPPuzzleStateSubsystem* PuzzleState = GI->GetSubsystem<UDPPuzzleStateSubsystem>())
		{
			PuzzleState->OnBallReachedGoal.RemoveDynamic(this, &ADPPuzzleChest::OnBallReachedGoalHandler);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ADPPuzzleChest::OnBallReachedGoalHandler()
{
	ShowChest();
}

void ADPPuzzleChest::ShowChest()
{
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
}

void ADPPuzzleChest::Interact(AActor* InteractingActor)
{
	// No se puede abrir un cofre dos veces. El flag es la única fuente de verdad;
	// no tocamos collision para que el cofre siga bloqueando al player tras abrirse.
	if (bHasBeenOpened)
	{
		return;
	}

	bHasBeenOpened = true;

	// PlayAnimation con Loop=false mantiene el último frame: el cofre se queda abierto en pantalla.
	// Programamos un timer porque PlayAnimation no dispara OnMontageEnded.
	if (OpenAnimation && ChestMesh)
	{
		ChestMesh->PlayAnimation(OpenAnimation, false);
		const float Duration = OpenAnimation->GetPlayLength();
		GetWorldTimerManager().SetTimer(
			OpenAnimTimerHandle, this,
			&ADPPuzzleChest::OnOpenAnimationEnded,
			Duration, false);
	}
	else
	{
		// Fallback: si no hay animación asignada, ejecuta directamente
		// la lógica final para que el puzzle no se quede colgado
		OnOpenAnimationEnded();
	}
}

void ADPPuzzleChest::OnOpenAnimationEnded()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UDPPuzzleStateSubsystem* PuzzleState = GI->GetSubsystem<UDPPuzzleStateSubsystem>())
		{
			PuzzleState->SetPlayerHasKey(true);
		}

		if (UDPMessageSubsystem* MessageSubsystem = GI->GetSubsystem<UDPMessageSubsystem>())
		{
			MessageSubsystem->RequestMessage(OnOpenMessage, OnOpenMessageDuration);
		}
	}
}
