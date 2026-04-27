#include "DPPlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputActionValue.h"

ADPPlayerCharacter::ADPPlayerCharacter()
{
	// El personaje no rota con el controller — la cámara es fija isométrica
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw   = false;
	bUseControllerRotationRoll  = false;

	// El personaje rota hacia la dirección de movimiento
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate              = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->MaxWalkSpeed              = 500.f;

	// Brazo del resorte con ángulo isométrico fijo
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 1500.f;
	SpringArm->SetRelativeRotation(FRotator(-30.f, -45.f, 0.f));
	SpringArm->bDoCollisionTest = false;
	SpringArm->bInheritPitch    = false;
	SpringArm->bInheritYaw      = false;
	SpringArm->bInheritRoll     = false;

    // NUEVA LÍNEA: rotación absoluta (no hereda la del Character)
    SpringArm->SetUsingAbsoluteRotation(true);

	// Cámara anclada al extremo del brazo
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->FieldOfView = 60.f;
}

void ADPPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Registrar el contexto de input con prioridad 0
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ADPPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADPPlayerCharacter::Move);
	}
}

void ADPPlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (!Controller)
	{
		return;
	}

	const FRotator CameraYawRotator(0.f, SpringArm->GetComponentRotation().Yaw, 0.f);
	const FVector ForwardDirection = FRotationMatrix(CameraYawRotator).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(CameraYawRotator).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);

}
