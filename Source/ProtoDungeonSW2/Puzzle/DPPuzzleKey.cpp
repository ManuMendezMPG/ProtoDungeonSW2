#include "DPPuzzleKey.h"
#include "DPPuzzleStateSubsystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ADPPuzzleKey::ADPPuzzleKey()
{
	PrimaryActorTick.bCanEverTick = false;

	// Interaction sphere as root: overlap-only with Pawn
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->InitSphereRadius(100.f);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractionSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	InteractionSphere->SetGenerateOverlapEvents(true);
	RootComponent = InteractionSphere;

	// Visual mesh (30cm cube)
	KeyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("KeyMesh"));
	KeyMesh->SetupAttachment(RootComponent);
	KeyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("/Engine/BasicShapes/Cube"));
	if (CubeMeshFinder.Succeeded())
	{
		KeyMesh->SetStaticMesh(CubeMeshFinder.Object);
	}
	KeyMesh->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.3f));

	// Initial state: hidden and with no collision until the subsystem activates us
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

void ADPPuzzleKey::BeginPlay()
{
	Super::BeginPlay();

	if (UDPPuzzleStateSubsystem* PuzzleState = GetGameInstance()->GetSubsystem<UDPPuzzleStateSubsystem>())
	{
		PuzzleState->OnBallReachedGoal.AddDynamic(this, &ADPPuzzleKey::OnBallReachedGoalHandler);

		// Defensive: if the event already fired before this key existed, activate it directly
		if (PuzzleState->bBallReachedGoal)
		{
			ActivateKey();
		}
	}
}

void ADPPuzzleKey::OnBallReachedGoalHandler()
{
	ActivateKey();
}

void ADPPuzzleKey::ActivateKey()
{
	if (bIsActive)
	{
		return;
	}

	bIsActive = true;
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
}

void ADPPuzzleKey::Interact(AActor* InteractingActor)
{
	// You can't pick up a key that hasn't been activated yet
	if (!bIsActive)
	{
		return;
	}

	if (UDPPuzzleStateSubsystem* PuzzleState = GetGameInstance()->GetSubsystem<UDPPuzzleStateSubsystem>())
	{
		PuzzleState->SetPlayerHasKey(true);
	}

	Destroy();
}
