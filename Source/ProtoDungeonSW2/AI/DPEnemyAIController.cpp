#include "DPEnemyAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

void ADPEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!BehaviorTreeAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("ADPEnemyAIController::OnPossess: BehaviorTreeAsset not assigned on %s"), *GetName());
		return;
	}

	// Initialize the blackboard if the BT declares one, before starting the tree
	if (BehaviorTreeAsset->BlackboardAsset)
	{
		UBlackboardComponent* BlackboardComp = nullptr;
		UseBlackboard(BehaviorTreeAsset->BlackboardAsset, BlackboardComp);
	}

	RunBehaviorTree(BehaviorTreeAsset);
}

void ADPEnemyAIController::OnUnPossess()
{
	// The BT stops automatically on unpossess; nothing extra for now
	Super::OnUnPossess();
}
