#include "DPPlayerCharacter.h"
#include "../Combat/DPCombatComponent.h"
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
	SpringArm->TargetArmLength = 2000.f;
	SpringArm->SetRelativeRotation(FRotator(-50.f, -45.f, 0.f));
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

	// Cámara cenital (top-down) para modo handheld
	TopDownSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("TopDownSpringArm"));
	TopDownSpringArm->SetupAttachment(RootComponent);
	TopDownSpringArm->TargetArmLength = 1500.f;
	TopDownSpringArm->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	TopDownSpringArm->bUsePawnControlRotation = false;
	TopDownSpringArm->bInheritPitch    = false;
	TopDownSpringArm->bInheritYaw      = false;
	TopDownSpringArm->bInheritRoll     = false;
	TopDownSpringArm->bDoCollisionTest = false;
	TopDownSpringArm->SetUsingAbsoluteRotation(true);

	TopDownCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCamera->SetupAttachment(TopDownSpringArm, USpringArmComponent::SocketName);
	TopDownCamera->FieldOfView = 60.f;
	TopDownCamera->SetActive(false);  // Inicia inactiva: la isométrica es la default

	// Componente de combate
	CombatComponent = CreateDefaultSubobject<UDPCombatComponent>(TEXT("CombatComponent"));
}

void ADPPlayerCharacter::SetTopDownCameraActive(bool bActive)
{
	if (TopDownCamera == nullptr || Camera == nullptr)
	{
		return;
	}

	TopDownCamera->SetActive(bActive);
	Camera->SetActive(!bActive);
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
		EIC->BindAction(AttackAction, ETriggerEvent::Started, this, &ADPPlayerCharacter::OnAttack);
		EIC->BindAction(SpecialAttackAction, ETriggerEvent::Started, this, &ADPPlayerCharacter::OnSpecialAttack);
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

void ADPPlayerCharacter::OnAttack(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		CombatComponent->TryBasicAttack();
	}
}

void ADPPlayerCharacter::OnSpecialAttack(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		CombatComponent->TrySpecialAttack();
	}
}
