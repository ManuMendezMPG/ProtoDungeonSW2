#include "DPEnemyAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

void ADPEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!BehaviorTreeAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("ADPEnemyAIController::OnPossess: BehaviorTreeAsset no asignado en %s"), *GetName());
		return;
	}

	// Inicializa el blackboard si el BT lo declara, antes de arrancar el árbol
	if (BehaviorTreeAsset->BlackboardAsset)
	{
		UBlackboardComponent* BlackboardComp = nullptr;
		UseBlackboard(BehaviorTreeAsset->BlackboardAsset, BlackboardComp);
	}

	RunBehaviorTree(BehaviorTreeAsset);
}

void ADPEnemyAIController::OnUnPossess()
{
	// El BT se detiene automáticamente al despossess; nada extra por ahora
	Super::OnUnPossess();
}
