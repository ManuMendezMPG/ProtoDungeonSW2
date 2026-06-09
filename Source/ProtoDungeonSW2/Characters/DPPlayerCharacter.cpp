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
	// The character doesn't rotate with the controller — the camera is a fixed isometric view
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw   = false;
	bUseControllerRotationRoll  = false;

	// The character rotates toward the movement direction
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate              = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->MaxWalkSpeed              = 500.f;

	// Spring arm with a fixed isometric angle
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 2000.f;
	SpringArm->SetRelativeRotation(FRotator(-50.f, -45.f, 0.f));
	SpringArm->bDoCollisionTest = false;
	SpringArm->bInheritPitch    = false;
	SpringArm->bInheritYaw      = false;
	SpringArm->bInheritRoll     = false;

    // NEW LINE: absolute rotation (does not inherit from the Character)
    SpringArm->SetUsingAbsoluteRotation(true);

	// Camera attached to the end of the arm
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->FieldOfView = 60.f;

	// Combat component
	CombatComponent = CreateDefaultSubobject<UDPCombatComponent>(TEXT("CombatComponent"));

	// Interactable detection sphere. Object type WorldDynamic + Overlap response to WorldDynamic
	// (interactables are WorldDynamic). Event subscription happens in BeginPlay.
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

	// Register the input context with priority 0
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// Ensure correct input state on spawn: after a Game Over Retry,
	// the editor viewport can retain the previous InputMode (UIOnly).
	// We force GameOnly and hide the cursor so gameplay receives
	// input correctly from the first frame.
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(false);
		EnableInput(PC);
	}

	// Subscribe to overlaps of the interaction sphere
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
		// Overrides any previous interactable: the most recent entrant wins
		CurrentInteractable = Interactable;
	}
}

void ADPPlayerCharacter::OnInteractionSphereEndOverlap(
	UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// Only clear if the one leaving is the one we had in range
	// (if we enter A, then B, and leave A, we still have B available)
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
	// Inherited behavior: plays DeathAnimation, disables
	// collision, stops movement, optionally triggers a level transition
	Super::OnDeath();

	// Block the player's input so they can't move / attack
	// while the death animation plays
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		DisableInput(PC);
	}

	// Schedule the Game Over to appear when the death
	// animation ends. With no animation, fall back to 2 seconds
	// so the player processes the moment before seeing the widget
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
			TEXT("ADPPlayerCharacter: GameOverWidgetClass not assigned. "
				 "Assign WBP_GameOver in the Blueprint."));
		return;
	}

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return;
	}

	// Create and show the widget
	UUserWidget* GameOverWidget = CreateWidget<UUserWidget>(
		PC, GameOverWidgetClass);
	if (GameOverWidget)
	{
		GameOverWidget->AddToViewport();
	}

	// Configure input for UI: mouse visible, UI events active,
	// no gameplay input capture
	PC->SetShowMouseCursor(true);
	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(InputMode);

	// Pause the game to freeze enemies and hold the player's
	// death pose at its last frame. The UI tick keeps
	// working for the Retry button
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
