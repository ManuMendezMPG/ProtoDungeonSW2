#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DPHoleTrigger.generated.h"

class UBoxComponent;
class ADPPuzzleBall;

UCLASS()
class PROTODUNGEONSW2_API ADPHoleTrigger : public AActor
{
	GENERATED_BODY()

public:
	ADPHoleTrigger();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> TriggerBox;

	// Callback when something enters the trigger.
	UFUNCTION()
	void OnTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	virtual void BeginPlay() override;
};
