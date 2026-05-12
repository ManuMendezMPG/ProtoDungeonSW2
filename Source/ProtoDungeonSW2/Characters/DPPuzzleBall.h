#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "DPPuzzleBall.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class USpringArmComponent;
class UCameraComponent;

UCLASS()
class PROTODUNGEONSW2_API ADPPuzzleBall : public APawn
{
	GENERATED_BODY()

public:
	ADPPuzzleBall();

	// Tunables expuestos en editor para iteración.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics")
	float Acceleration = 980.0f;  // Aceleración base, equivalente a "gravedad" cuando tilt = max.

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics")
	float MaxSpeed = 500.0f;  // Velocidad máxima de la bola.

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics")
	float Friction = 0.95f;  // Factor multiplicador de velocidad por segundo (1.0 = sin fricción).

	// Vector de tilt actual (X = pitch del dispositivo, Y = roll). Lo asignaremos desde fuera.
	UPROPERTY(BlueprintReadWrite, Category = "Physics")
	FVector2D CurrentTilt;

	// Lifecycle.
	virtual void Tick(float DeltaTime) override;

	// Para que el GameMode pueda posicionar la bola al spawnear.
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
	// Mesh visible de la bola (esfera).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BallMesh;

	// Collision (esférica) que actúa como root.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionSphere;

	// SpringArm + Camera cenital para vista de puzzle.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> Camera;

	// Velocidad actual de la bola (mundo).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics")
	FVector CurrentVelocity;

	virtual void BeginPlay() override;
};
