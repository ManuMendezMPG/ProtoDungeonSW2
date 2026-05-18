#include "DPPuzzleDoor.h"
#include "DPPuzzleStateSubsystem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ADPPuzzleDoor::ADPPuzzleDoor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Mesh sólido como root: bloquea al player
	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	RootComponent = DoorMesh;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("/Engine/BasicShapes/Cube"));
	if (CubeMeshFinder.Succeeded())
	{
		DoorMesh->SetStaticMesh(CubeMeshFinder.Object);
	}
	// Puerta delgada (30cm), ancha (200cm), alta (300cm)
	DoorMesh->SetRelativeScale3D(FVector(0.3f, 2.0f, 3.0f));
	DoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	DoorMesh->SetCollisionObjectType(ECC_WorldStatic);
	DoorMesh->SetCollisionResponseToAllChannels(ECR_Block);

	// Esfera de interacción: solo overlap, no bloquea
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(RootComponent);
	InteractionSphere->InitSphereRadius(150.f);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	// CRÍTICO: WorldDynamic también debe responder para que la InteractionSphere
	// del player (WorldDynamic) detecte esta esfera. Sin esto el overlap no dispara.
	InteractionSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	InteractionSphere->SetGenerateOverlapEvents(true);
}

void ADPPuzzleDoor::Interact(AActor* InteractingActor)
{
	if (bIsOpen)
	{
		return;
	}

	if (UDPPuzzleStateSubsystem* PuzzleState = GetGameInstance()->GetSubsystem<UDPPuzzleStateSubsystem>())
	{
		if (PuzzleState->bPlayerHasKey)
		{
			OpenDoor();
		}
		else
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Necesitas la llave"));
			}
		}
	}
}

void ADPPuzzleDoor::OpenDoor()
{
	bIsOpen = true;
	DoorMesh->SetVisibility(false);
	DoorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("Puerta abierta"));
	}
}
