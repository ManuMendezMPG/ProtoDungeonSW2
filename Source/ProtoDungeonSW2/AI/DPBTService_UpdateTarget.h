#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "DPBTService_UpdateTarget.generated.h"

UCLASS(ClassGroup = "AI")
class PROTODUNGEONSW2_API UDPBTService_UpdateTarget : public UBTService_BlackboardBase
{
	GENERATED_BODY()

public:
	UDPBTService_UpdateTarget();

	// Maximum radius at which the enemy "sees" the player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (ClampMin = "0.0"))
	float DetectionRadius = 1000.f;

	// Distance at which the player is considered in attack range
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (ClampMin = "0.0"))
	float AttackRange = 130.f;

	// BB key where we write the reference to the detected player (Object: AActor)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	FBlackboardKeySelector TargetActorKey;

	// BB key where we write whether the player is in attack range (Bool)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	FBlackboardKeySelector IsInAttackRangeKey;

protected:
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
