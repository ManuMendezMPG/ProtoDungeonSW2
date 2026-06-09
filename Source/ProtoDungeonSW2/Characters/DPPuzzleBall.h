#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "DPPuzzleBall.generated.h"

class UStaticMeshComponent;
class USphereComponent;

UCLASS()
class PROTODUNGEONSW2_API ADPPuzzleBall : public APawn
{
	GENERATED_BODY()

public:
	ADPPuzzleBall();

	// Tunables exposed in the editor for iteration.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics")
	float Acceleration = 980.0f;  // Base acceleration, equivalent to "gravity" when tilt = max.

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics")
	float MaxSpeed = 500.0f;  // Maximum ball speed.

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics")
	float Friction = 0.95f;  // Per-second velocity multiplier (1.0 = no friction).

	// Current tilt vector (X = device pitch, Y = roll). Assigned from outside.
	UPROPERTY(BlueprintReadWrite, Category = "Physics")
	FVector2D CurrentTilt;

	// Lifecycle.
	virtual void Tick(float DeltaTime) override;

	// So the GameMode can position the ball on spawn.
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
	// Visible ball mesh (sphere).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BallMesh;

	// Sphere collision that acts as root.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionSphere;

	// Current ball velocity (world space).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Physics")
	FVector CurrentVelocity;

	virtual void BeginPlay() override;
};
