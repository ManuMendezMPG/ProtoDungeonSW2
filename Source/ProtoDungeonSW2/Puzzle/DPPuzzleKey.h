#pragma once

#include "CoreMinimal.h"
#include "DPInteractableBase.h"
#include "DPPuzzleKey.generated.h"

class UStaticMeshComponent;
class USphereComponent;

UCLASS()
class PROTODUNGEONSW2_API ADPPuzzleKey : public ADPInteractableBase
{
	GENERATED_BODY()

public:
	ADPPuzzleKey();

	// State: whether the key is active (visible + collidable).
	UPROPERTY(BlueprintReadOnly, Category = "Puzzle")
	bool bIsActive = false;

	// Activates the key (makes it visible and enables collision). Called when the subsystem event fires.
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void ActivateKey();

	// Override: picking up the key marks the player as having it and self-destructs.
	virtual void Interact(AActor* InteractingActor) override;

protected:
	// Visual mesh (cube placeholder).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> KeyMesh;

	// Overlap sphere to detect when the player approaches.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> InteractionSphere;

	virtual void BeginPlay() override;

	// Handler for the subsystem event.
	UFUNCTION()
	void OnBallReachedGoalHandler();
};
