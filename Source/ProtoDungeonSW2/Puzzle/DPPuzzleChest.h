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

	// Animation played on interact (chest opening).
	// We use UAnimSequence + PlayAnimation because, unlike Montage_Play,
	// PlayAnimation with Loop=false keeps the last frame on screen.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle Chest")
	TObjectPtr<UAnimSequence> OpenAnimation;

	// Message requested for display via UDPMessageSubsystem when the chest opens.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Chest")
	FString OnOpenMessage = TEXT("You found a key");

	// Duration of the on-screen message when opening.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Chest")
	float OnOpenMessageDuration = 3.0f;

	// Internal state: whether the chest has already been opened. Runtime only, not editable.
	UPROPERTY(BlueprintReadOnly, Category = "Puzzle Chest")
	bool bHasBeenOpened = false;

	// Override: opens the chest, plays the montage, and on completion grants key + message.
	virtual void Interact(AActor* InteractingActor) override;

protected:
	// Chest skeletal mesh (acts as the root component).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> ChestMesh;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Subsystem event handler: the ball has reached the goal, the chest becomes visible.
	UFUNCTION()
	void OnBallReachedGoalHandler();

	// Makes the chest visible and collidable.
	void ShowChest();

	// Callback when OpenAnimation finishes (fired by timer). Grants the key and requests the message display.
	void OnOpenAnimationEnded();

private:
	// Timer that counts OpenAnimation's duration; PlayAnimation does not fire OnMontageEnded.
	FTimerHandle OpenAnimTimerHandle;
};
