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

	// State: whether the door is open.
	UPROPERTY(BlueprintReadOnly, Category = "Puzzle")
	bool bIsOpen = false;

	// If true, when this door opens it asks UDPLevelTransitionSubsystem to load NextLevelName.
	// Intended for end-of-level doors; other doors leave this as false.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
	bool bTriggersLevelTransitionOnOpen = false;

	// Destination map on open when bTriggersLevelTransitionOnOpen = true.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
	FName NextLevelName = NAME_None;

	// Delay before starting the fade after the door opens (seconds).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transition")
	float TransitionDelay = 0.5f;

	// Message requested for display via UDPMessageSubsystem when trying to open the door without a key.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Door")
	FString NoKeyMessage = TEXT("You need a key. Maybe you'll find it taking the bull by the joycons... I mean, horns.");

	// Duration of the "no key" on-screen message.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle Door")
	float NoKeyMessageDuration = 6.0f;

	// Animation played when the door opens (Kenney's gateopen, assigned in the BP).
	// PlayAnimation with Loop=false leaves the door at the last frame (open) without returning to bind pose.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Puzzle Door")
	TObjectPtr<UAnimSequence> OpenAnimation;

	virtual void Interact(AActor* InteractingActor) override;

protected:
	// Door skeletal mesh (Kenney's gate, assigned in the BP). Acts as the root component.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> DoorMesh;

	// Overlap sphere to detect the player.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> InteractionSphere;

	// Marks the door as open and plays the animation. Collision and transition
	// are handled in OnOpenAnimationEnded when the animation finishes.
	void OpenDoor();

	// Callback when OpenAnimation finishes (fired by timer). Disables collision and triggers
	// the level transition if the door is the level exit.
	void OnOpenAnimationEnded();

private:
	// Timer that counts OpenAnimation's duration; PlayAnimation does not fire OnMontageEnded.
	FTimerHandle OpenAnimTimerHandle;
};
