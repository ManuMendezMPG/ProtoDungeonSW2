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

	// Llamado cuando el player interactúa con este actor (pulsa E estando en range).
	// Las clases derivadas hacen override para implementar comportamiento específico.
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	virtual void Interact(AActor* InteractingActor);
};
