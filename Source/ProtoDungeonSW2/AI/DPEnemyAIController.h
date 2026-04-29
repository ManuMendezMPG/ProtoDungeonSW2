#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "DPEnemyAIController.generated.h"

UCLASS()
class PROTODUNGEONSW2_API ADPEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	// BT que ejecutará este controller al poseer al pawn. Se asigna desde el BP del controller.
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<class UBehaviorTree> BehaviorTreeAsset;

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
};
