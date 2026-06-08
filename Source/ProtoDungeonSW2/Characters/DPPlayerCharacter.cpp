#include "DPPlayerCharacter.h"
#include "../Combat/DPCombatComponent.h"
#include "../Input/DPPlatformModeSubsystem.h"
#include "../Puzzle/DPInteractableBase.h"
#include "Animation/AnimSequence.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

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

	// Componente de combate
	CombatComponent = CreateDefaultSubobject<UDPCombatComponent>(TEXT("CombatComponent"));

	// Esfera de detección de interactables. Object type WorldDynamic + respuesta Overlap a WorldDynamic
	// (los interactables son WorldDynamic). La suscripción a los eventos va en BeginPlay.
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(RootComponent);
	InteractionSphere->SetSphereRadius(150.f);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	InteractionSphere->SetGenerateOverlapEvents(true);
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

	// Asegurar estado de input correcto al spawnear: tras un Game Over Retry,
	// el viewport del editor puede mantener el InputMode anterior (UIOnly).
	// Forzamos GameOnly y ocultamos el cursor para que el gameplay reciba
	// input correctamente desde el primer frame.
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(false);
		EnableInput(PC);
	}

	// Suscribirse a overlaps de la esfera de interacción
	if (InteractionSphere)
	{
		InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ADPPlayerCharacter::OnInteractionSphereBeginOverlap);
		InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &ADPPlayerCharacter::OnInteractionSphereEndOverlap);
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

		if (InteractAction != nullptr)
		{
			EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &ADPPlayerCharacter::OnInteractPressed);
		}

		if (ToggleModeAction != nullptr)
		{
			EIC->BindAction(ToggleModeAction, ETriggerEvent::Started, this, &ADPPlayerCharacter::OnToggleModePressed);
		}
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

void ADPPlayerCharacter::OnInteractionSphereBeginOverlap(
	UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (ADPInteractableBase* Interactable = Cast<ADPInteractableBase>(OtherActor))
	{
		// Sobrescribe cualquier interactable previo: el último que entra gana
		CurrentInteractable = Interactable;
	}
}

void ADPPlayerCharacter::OnInteractionSphereEndOverlap(
	UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// Solo limpiamos si el que sale es el que teníamos en range
	// (si entramos en A, luego B, y salimos de A, seguimos teniendo B disponible)
	if (OtherActor == CurrentInteractable)
	{
		CurrentInteractable = nullptr;
	}
}

void ADPPlayerCharacter::OnInteractPressed()
{
	if (CurrentInteractable != nullptr)
	{
		CurrentInteractable->Interact(this);
	}
}

void ADPPlayerCharacter::OnDeath()
{
	// Comportamiento heredado: reproduce DeathAnimation, desactiva
	// collision, detiene movement, opcionalmente transiciona de nivel
	Super::OnDeath();

	// Bloquear input del player para que no pueda moverse / atacar
	// mientras se reproduce la animación de muerte
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		DisableInput(PC);
	}

	// Programar la aparición del Game Over para cuando termine la
	// animación de muerte. Si no hay animación, fallback a 2 segundos
	// para que el jugador procese el momento antes de ver el widget
	float Delay = 2.0f;
	if (DeathAnimation)
	{
		Delay = DeathAnimation->GetPlayLength();
	}

	GetWorldTimerManager().SetTimer(
		GameOverTimerHandle, this,
		&ADPPlayerCharacter::ShowGameOverScreen,
		Delay, false);
}

void ADPPlayerCharacter::ShowGameOverScreen()
{
	if (!GameOverWidgetClass)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("ADPPlayerCharacter: GameOverWidgetClass no asignado. "
				 "Asignar WBP_GameOver en el Blueprint."));
		return;
	}

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return;
	}

	// Crear y mostrar el widget
	UUserWidget* GameOverWidget = CreateWidget<UUserWidget>(
		PC, GameOverWidgetClass);
	if (GameOverWidget)
	{
		GameOverWidget->AddToViewport();
	}

	// Configurar input para UI: ratón visible, eventos de UI activos,
	// sin captura de input de gameplay
	PC->SetShowMouseCursor(true);
	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(InputMode);

	// Pausar el juego para congelar enemigos y mantener la pose de
	// muerte del player en su último frame. El UI tick sigue
	// funcionando para el botón Retry
	UGameplayStatics::SetGamePaused(this, true);
}

void ADPPlayerCharacter::OnToggleModePressed()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UDPPlatformModeSubsystem* PlatformSubsystem =
			GI->GetSubsystem<UDPPlatformModeSubsystem>())
		{
			PlatformSubsystem->TogglePlatformMode();
		}
	}
}
