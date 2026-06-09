#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "DPEnemyAIController.generated.h"

UCLASS()
class PROTODUNGEONSW2_API ADPEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	// BT this controller will run when it possesses the pawn. Assigned from the controller's BP.
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<class UBehaviorTree> BehaviorTreeAsset;

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
};
