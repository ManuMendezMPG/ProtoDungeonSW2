#pragma once

#include "CoreMinimal.h"
#include "DPInteractableBase.h"
#include "DPPuzzleChest.generated.h"

class USkeletalMeshComponent;
class UAnimSequence;

UCLASS()
class PROTODUNGEONSW2_API ADPPuzzleChest : public ADPInteractableBase
{
	GENERATED_BODY()

public:
	ADPPuzzleChest();

	// Animación que se reproduce al interactuar (apertura del cofre).
	// Usamos UAnimSequence + PlayAnimation porque, a diferencia de Montage_Play,
	// PlayAnimation con Loop=false mantiene el último frame en pantalla.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle Chest")
	TObjectPtr<UAnimSequence> OpenAnimation;

	// Mensaje que se solicita mostrar vía UDPMessageSubsystem al abrir el cofre.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Chest")
	FString OnOpenMessage = TEXT("Has obtenido una llave");

	// Duración del mensaje on-screen al abrirse.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Chest")
	float OnOpenMessageDuration = 3.0f;

	// Estado interno: si el cofre ya ha sido abierto. Sólo runtime, no editable.
	UPROPERTY(BlueprintReadOnly, Category = "Puzzle Chest")
	bool bHasBeenOpened = false;

	// Override: abre el cofre, reproduce montage y al terminar otorga llave + mensaje.
	virtual void Interact(AActor* InteractingActor) override;

protected:
	// Skeletal mesh del cofre (sirve como root component).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> ChestMesh;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Handler del evento del subsystem: la bola ha llegado a la meta, el cofre se hace visible.
	UFUNCTION()
	void OnBallReachedGoalHandler();

	// Hace visible y collidable el cofre.
	void ShowChest();

	// Callback al terminar la OpenAnimation (disparado por timer). Otorga la llave y solicita mostrar el mensaje.
	void OnOpenAnimationEnded();

private:
	// Timer que cuenta la duración de OpenAnimation; PlayAnimation no emite OnMontageEnded.
	FTimerHandle OpenAnimTimerHandle;
};
