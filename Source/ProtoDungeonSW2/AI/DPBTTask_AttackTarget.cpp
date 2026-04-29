#include "DPBTTask_AttackTarget.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "../Combat/DPCombatComponent.h"
#include "../Characters/DPEnemyBase.h"

UDPBTTask_AttackTarget::UDPBTTask_AttackTarget()
{
	NodeName = TEXT("Attack Target");

	// Filtro de tipo en el editor: TargetActorKey solo acepta keys Object/AActor
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UDPBTTask_AttackTarget, TargetActorKey), AActor::StaticClass());
}

void UDPBTTask_AttackTarget::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		TargetActorKey.ResolveSelectedKey(*BBAsset);
	}
}

EBTNodeResult::Type UDPBTTask_AttackTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Necesitamos el controller para llegar al pawn poseído
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	ADPEnemyBase* Enemy = Cast<ADPEnemyBase>(AIController->GetPawn());
	if (!Enemy)
	{
		return EBTNodeResult::Failed;
	}

	// CombatComponent es protected en ADPEnemyBase: lo localizamos por clase
	UDPCombatComponent* CombatComp = Enemy->FindComponentByClass<UDPCombatComponent>();
	if (!CombatComp)
	{
		return EBTNodeResult::Failed;
	}

	// Comprobación defensiva: debe haber un target en el blackboard
	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	if (!BBComp)
	{
		return EBTNodeResult::Failed;
	}

	AActor* Target = Cast<AActor>(BBComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!Target)
	{
		return EBTNodeResult::Failed;
	}

	// Si está en cooldown, devolvemos Failed para que el árbol pueda reaccionar
	if (!CombatComp->CanBasicAttack())
	{
		return EBTNodeResult::Failed;
	}

	CombatComp->TryBasicAttack();
	return EBTNodeResult::Succeeded;
}
