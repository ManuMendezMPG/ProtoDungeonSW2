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

	// Override de la muerte: añade al flujo base el bloqueo de input y el Game Over UI
	// tras la duración de DeathAnimation (o un fallback si no hay animación asignada)
	virtual void OnDeath() override;

	// Clase del widget que aparece al morir el player. Se asigna WBP_GameOver desde el Blueprint child.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Over")
	TSubclassOf<UUserWidget> GameOverWidgetClass;

	// Contexto de input por defecto (Combat)
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	// Acción de movimiento 2D
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	// Acción de ataque básico
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> AttackAction;

	// Acción de ataque especial
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SpecialAttackAction;

	// Acción de interacción (pulsar E para recoger llaves, abrir puertas, etc.)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;

	// Acción para alternar Docked/Handheld (tecla M en PC). El IA está en IMC_Default.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ToggleModeAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UDPCombatComponent> CombatComponent;

	// Esfera de detección de interactables a corta distancia
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<USphereComponent> InteractionSphere;

	// Interactable actualmente en range (último que ha entrado en la esfera y sigue dentro)
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

	// Timer para retrasar la aparición del Game Over hasta que termine DeathAnimation.
	FTimerHandle GameOverTimerHandle;

	// Muestra el widget de Game Over y bloquea input. Llamado tras la
	// duración de DeathAnimation para que el feedback visual de muerte
	// y la UI estén sincronizados
	void ShowGameOverScreen();
};
