#include "DPPuzzleKey.h"
#include "DPPuzzleStateSubsystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ADPPuzzleKey::ADPPuzzleKey()
{
	PrimaryActorTick.bCanEverTick = false;

	// Esfera de interacción como root: solo overlap con Pawn
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->InitSphereRadius(100.f);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractionSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	InteractionSphere->SetGenerateOverlapEvents(true);
	RootComponent = InteractionSphere;

	// Mesh visual (cubo 30cm de lado)
	KeyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("KeyMesh"));
	KeyMesh->SetupAttachment(RootComponent);
	KeyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("/Engine/BasicShapes/Cube"));
	if (CubeMeshFinder.Succeeded())
	{
		KeyMesh->SetStaticMesh(CubeMeshFinder.Object);
	}
	KeyMesh->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.3f));

	// Estado inicial: oculto y sin collision hasta que el subsystem nos active
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

void ADPPuzzleKey::BeginPlay()
{
	Super::BeginPlay();

	if (UDPPuzzleStateSubsystem* PuzzleState = GetGameInstance()->GetSubsystem<UDPPuzzleStateSubsystem>())
	{
		PuzzleState->OnBallReachedGoal.AddDynamic(this, &ADPPuzzleKey::OnBallReachedGoalHandler);

		// Defensivo: si el evento ya se disparó antes de que esta llave existiera, activarla directamente
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
	// No se puede recoger una llave que aún no se ha activado
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
