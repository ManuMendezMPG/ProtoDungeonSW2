#include "DPPuzzleBall.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ADPPuzzleBall::ADPPuzzleBall()
{
	PrimaryActorTick.bCanEverTick = true;

	// Sphere collision as root (50cm radius)
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->InitSphereRadius(50.f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionSphere->SetCollisionObjectType(ECC_Pawn);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Block);
	RootComponent = CollisionSphere;

	// Visible mesh — collision lives on the sphere component, the mesh is purely visual
	BallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallMesh"));
	BallMesh->SetupAttachment(RootComponent);
	BallMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// UE sphere mesh: scale 1 = 100cm diameter (50cm radius), matches the sphere collision
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshFinder(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereMeshFinder.Succeeded())
	{
		BallMesh->SetStaticMesh(SphereMeshFinder.Object);
	}
	BallMesh->SetRelativeScale3D(FVector(1.f));

	// The puzzle camera lives in the world (Camera Actor tagged "PuzzleCamera");
	// the ViewTarget is managed by ADPPuzzlePlayerController on mode change.
}

void ADPPuzzleBall::BeginPlay()
{
	Super::BeginPlay();

	CurrentVelocity = FVector::ZeroVector;
	CurrentTilt     = FVector2D::ZeroVector;
}

void ADPPuzzleBall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Tilt-derived acceleration: X tilt -> X world, Y tilt -> Y world (Z = 0, no jumps)
	const FVector TiltAcceleration = FVector(CurrentTilt.X, CurrentTilt.Y, 0.f) * Acceleration;

	CurrentVelocity += TiltAcceleration * DeltaTime;

	// Exponential friction: Friction^DeltaTime ⇒ over 1s velocity is multiplied by Friction
	CurrentVelocity *= FMath::Pow(Friction, DeltaTime);

	// Clamp by magnitude (SizeSquared avoids the sqrt if unnecessary)
	if (CurrentVelocity.SizeSquared() > FMath::Square(MaxSpeed))
	{
		CurrentVelocity = CurrentVelocity.GetSafeNormal() * MaxSpeed;
	}

	// Movement with collision sweep (sweep = true)
	const FVector NewLocation = GetActorLocation() + CurrentVelocity * DeltaTime;
	FHitResult Hit;
	SetActorLocation(NewLocation, true, &Hit);

	// On a blocking hit, project velocity onto the plane perpendicular to the normal
	// so the ball slides along the wall instead of sticking.
	if (Hit.bBlockingHit)
	{
		CurrentVelocity = FVector::VectorPlaneProject(CurrentVelocity, Hit.ImpactNormal);
	}
}

void ADPPuzzleBall::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Reserved: the ball has no inputs of its own for now (tilt comes from the subsystem)
}
