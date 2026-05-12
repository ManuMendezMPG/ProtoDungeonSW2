#include "DPPuzzleBall.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/SpringArmComponent.h"
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

	// SpringArm cenital
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 1500.f;
	SpringArm->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritPitch    = false;
	SpringArm->bInheritYaw      = false;
	SpringArm->bInheritRoll     = false;
	SpringArm->bDoCollisionTest = false;
	SpringArm->SetUsingAbsoluteRotation(true);

	// Cámara cenital
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->FieldOfView = 60.f;
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
	SetActorLocation(NewLocation, true);
}

void ADPPuzzleBall::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Reservado: la bola no tiene inputs propios por ahora (el tilt viene del subsystem)
}
