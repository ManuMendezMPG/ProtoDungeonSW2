#pragma once

#include "CoreMinimal.h"
#include "DPInteractableBase.h"
#include "DPPuzzleDoor.generated.h"

class UStaticMeshComponent;
class USphereComponent;

UCLASS()
class PROTODUNGEONSW2_API ADPPuzzleDoor : public ADPInteractableBase
{
	GENERATED_BODY()

public:
	ADPPuzzleDoor();

	// Estado: si la puerta está abierta.
	UPROPERTY(BlueprintReadOnly, Category = "Puzzle")
	bool bIsOpen = false;

	virtual void Interact(AActor* InteractingActor) override;

protected:
	// Mesh visual de la puerta (placeholder cubo escalado).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> DoorMesh;

	// Esfera de overlap para detectar al player.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> InteractionSphere;

	// Abre la puerta: hide mesh + disable collision.
	void OpenDoor();
};
