#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "DPBTTask_AttackTarget.generated.h"

UCLASS(ClassGroup = "AI")
class PROTODUNGEONSW2_API UDPBTTask_AttackTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UDPBTTask_AttackTarget();

	// Key del BB que contiene el target a atacar (Object: AActor)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
