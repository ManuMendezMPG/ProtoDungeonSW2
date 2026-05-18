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

	// Estado: si la llave está activa (visible + collidable).
	UPROPERTY(BlueprintReadOnly, Category = "Puzzle")
	bool bIsActive = false;

	// Activa la llave (la hace visible y activa collision). Llamado al recibir el evento del subsystem.
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void ActivateKey();

	// Override: recoger la llave marca al player como teniéndola y se autodestruye.
	virtual void Interact(AActor* InteractingActor) override;

protected:
	// Mesh visual (placeholder con cubo).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> KeyMesh;

	// Esfera de overlap para detectar cuando el player se acerca.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> InteractionSphere;

	virtual void BeginPlay() override;

	// Handler para el evento del subsystem.
	UFUNCTION()
	void OnBallReachedGoalHandler();
};
