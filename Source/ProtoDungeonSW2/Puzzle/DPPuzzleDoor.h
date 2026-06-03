#pragma once

#include "CoreMinimal.h"
#include "DPInteractableBase.h"
#include "DPPuzzleDoor.generated.h"

class USkeletalMeshComponent;
class USphereComponent;
class UAnimSequence;

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

	// Mensaje que se solicita mostrar vía UDPMessageSubsystem al intentar abrir la puerta sin llave.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Door")
	FString NoKeyMessage = TEXT("You need a key. Maybe you'll find it taking the bull by the joycons... I mean, horns.");

	// Duración del mensaje "sin llave" on-screen.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Door")
	float NoKeyMessageDuration = 6.0f;

	// Animación que se reproduce al abrirse la puerta (gateopen de Kenney, asignada en el BP).
	// PlayAnimation con Loop=false deja la puerta en el último frame (abierta) sin volver al bind pose.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle Door")
	TObjectPtr<UAnimSequence> OpenAnimation;

	virtual void Interact(AActor* InteractingActor) override;

protected:
	// Skeletal mesh de la puerta (gate de Kenney, asignado en el BP). Sirve como root component.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> DoorMesh;

	// Esfera de overlap para detectar al player.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> InteractionSphere;

	// Marca la puerta como abierta y reproduce la animación. La colisión y la transición
	// se gestionan en OnOpenAnimationEnded al terminar la animación.
	void OpenDoor();

	// Callback al terminar OpenAnimation (disparado por timer). Desactiva colisión y dispara
	// la transición de nivel si la puerta es la salida del nivel.
	void OnOpenAnimationEnded();

private:
	// Timer que cuenta la duración de OpenAnimation; PlayAnimation no emite OnMontageEnded.
	FTimerHandle OpenAnimTimerHandle;
};
