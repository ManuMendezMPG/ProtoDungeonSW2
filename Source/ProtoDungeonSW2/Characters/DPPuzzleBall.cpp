#include "DPPuzzleBall.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ADPPuzzleBall::ADPPuzzleBall()
{
	PrimaryActorTick.bCanEverTick = true;

	// Collision esférica como root (50cm de radio)
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->InitSphereRadius(50.f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionSphere->SetCollisionObjectType(ECC_Pawn);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Block);
	RootComponent = CollisionSphere;

	// Mesh visible — la colisión la lleva la sphere component, el mesh sólo visual
	BallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallMesh"));
	BallMesh->SetupAttachment(RootComponent);
	BallMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Mesh de esfera de UE: escala 1 = 100cm diámetro (50cm radio), coherente con la sphere collision
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshFinder(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereMeshFinder.Succeeded())
	{
		BallMesh->SetStaticMesh(SphereMeshFinder.Object);
	}
	BallMesh->SetRelativeScale3D(FVector(1.f));

	// La cámara del puzzle vive en el mundo (Camera Actor con tag "PuzzleCamera");
	// el ViewTarget lo gestiona ADPPuzzlePlayerController al cambiar de modo.
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

	// Aceleración derivada del tilt: X tilt -> X mundo, Y tilt -> Y mundo (Z = 0, sin saltos)
	const FVector TiltAcceleration = FVector(CurrentTilt.X, CurrentTilt.Y, 0.f) * Acceleration;

	CurrentVelocity += TiltAcceleration * DeltaTime;

	// Fricción exponencial: Friction^DeltaTime ⇒ en 1s la velocidad se multiplica por Friction
	CurrentVelocity *= FMath::Pow(Friction, DeltaTime);

	// Clamp por magnitud (SizeSquared para evitar sqrt si no hace falta)
	if (CurrentVelocity.SizeSquared() > FMath::Square(MaxSpeed))
	{
		CurrentVelocity = CurrentVelocity.GetSafeNormal() * MaxSpeed;
	}

	// Movimiento con barrido de colisiones (sweep = true)
	const FVector NewLocation = GetActorLocation() + CurrentVelocity * DeltaTime;
	FHitResult Hit;
	SetActorLocation(NewLocation, true, &Hit);

	// Si hubo bloqueo, proyectar la velocidad en el plano perpendicular a la normal
	// para que la bola deslice a lo largo de la pared en lugar de quedar pegada.
	if (Hit.bBlockingHit)
	{
		CurrentVelocity = FVector::VectorPlaneProject(CurrentVelocity, Hit.ImpactNormal);
	}
}

void ADPPuzzleBall::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Reservado: la bola no tiene inputs propios por ahora (el tilt viene del subsystem)
}
