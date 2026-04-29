#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "DPBTTask_AttackTarget.generated.h"

UCLASS(ClassGroup = "AI")
class PROTODUNGEONSW2_API UDPBTTask_AttackTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UDPBTTask_AttackTarget();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
