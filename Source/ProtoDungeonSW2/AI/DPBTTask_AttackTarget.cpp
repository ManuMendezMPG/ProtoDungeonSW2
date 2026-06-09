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

	// Type filter in the editor: TargetActorKey only accepts Object/AActor keys
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
	// We need the controller to get to the possessed pawn
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

	// CombatComponent is protected in ADPEnemyBase: we look it up by class
	UDPCombatComponent* CombatComp = Enemy->FindComponentByClass<UDPCombatComponent>();
	if (!CombatComp)
	{
		return EBTNodeResult::Failed;
	}

	// Defensive check: there must be a target in the blackboard
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

	// If on cooldown, return Failed so the tree can react
	if (!CombatComp->CanBasicAttack())
	{
		return EBTNodeResult::Failed;
	}

	CombatComp->TryBasicAttack();
	return EBTNodeResult::Succeeded;
}
