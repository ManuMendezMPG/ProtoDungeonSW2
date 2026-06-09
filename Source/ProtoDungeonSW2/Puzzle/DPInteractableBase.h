#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DPInteractableBase.generated.h"

UCLASS(Abstract)
class PROTODUNGEONSW2_API ADPInteractableBase : public AActor
{
	GENERATED_BODY()

public:
	ADPInteractableBase();

	// Called when the player interacts with this actor (presses E while in range).
	// Derived classes override to implement specific behavior.
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual void Interact(AActor* InteractingActor);
};
