#pragma once

#include "CoreMinimal.h"
#include "DPCharacterBase.h"
#include "DPPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class USphereComponent;
class UInputMappingContext;
class UInputAction;
class UDPCombatComponent;
class ADPInteractableBase;
class UUserWidget;
struct FInputActionValue;

UCLASS()
class PROTODUNGEONSW2_API ADPPlayerCharacter : public ADPCharacterBase
{
	GENERATED_BODY()

public:
	ADPPlayerCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// Death override: adds input blocking and the Game Over UI to the base flow,
	// after DeathAnimation's duration (or a fallback if no animation is assigned)
	virtual void OnDeath() override;

	// Widget class shown when the player dies. WBP_GameOver is assigned from the Blueprint child.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Over")
	TSubclassOf<UUserWidget> GameOverWidgetClass;

	// Default input context (Combat)
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	// 2D movement action
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	// Basic attack action
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> AttackAction;

	// Special attack action
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SpecialAttackAction;

	// Interaction action (press E to pick up keys, open doors, etc.)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;

	// Action to toggle Docked/Handheld (M key on PC). The IA lives in IMC_Default.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ToggleModeAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UDPCombatComponent> CombatComponent;

	// Short-range interactable detection sphere
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<USphereComponent> InteractionSphere;

	// Interactable currently in range (the most recent one that entered the sphere and is still inside)
	UPROPERTY()
	TObjectPtr<ADPInteractableBase> CurrentInteractable;

	UFUNCTION()
	void OnInteractionSphereBeginOverlap(
		UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnInteractionSphereEndOverlap(
		UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex
	);

private:
	void Move(const FInputActionValue& Value);
	void OnAttack(const FInputActionValue& Value);
	void OnSpecialAttack(const FInputActionValue& Value);
	void OnInteractPressed();
	void OnToggleModePressed();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> Camera;

	// Timer to delay the Game Over appearance until DeathAnimation finishes.
	FTimerHandle GameOverTimerHandle;

	// Shows the Game Over widget and blocks input. Called after
	// DeathAnimation's duration so the visual death feedback and the UI
	// stay synchronized
	void ShowGameOverScreen();
};
