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

	// Si true, al abrirse esta puerta solicita al UDPLevelTransitionSubsystem cargar NextLevelName.
	// Pensado para puertas de fin de nivel; el resto de puertas lo dejan en false.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
	bool bTriggersLevelTransitionOnOpen = false;

	// Mapa destino al abrirse cuando bTriggersLevelTransitionOnOpen = true.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
	FName NextLevelName = NAME_None;

	// Delay antes de iniciar el fade tras abrirse la puerta (segundos).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
	float TransitionDelay = 0.5f;

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
