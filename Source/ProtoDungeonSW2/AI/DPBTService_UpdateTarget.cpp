#include "DPBTService_UpdateTarget.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

UDPBTService_UpdateTarget::UDPBTService_UpdateTarget()
{
	NodeName = TEXT("Update Target");

	// Periodic tick: every 0.2s with +/-0.05s deviation to desynchronize enemies
	Interval = 0.2f;
	RandomDeviation = 0.05f;

	// Type filters in the editor: TargetActor only accepts keys of type Object/AActor;
	// IsInAttackRange only accepts keys of type Bool
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UDPBTService_UpdateTarget, TargetActorKey), AActor::StaticClass());
	IsInAttackRangeKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UDPBTService_UpdateTarget, IsInAttackRangeKey));
}

void UDPBTService_UpdateTarget::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	// Resolve the selected keys' IDs against the asset's BB
	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		TargetActorKey.ResolveSelectedKey(*BBAsset);
		IsInAttackRangeKey.ResolveSelectedKey(*BBAsset);
	}
}

void UDPBTService_UpdateTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return;
	}

	APawn* SelfPawn = AIController->GetPawn();
	if (!SelfPawn)
	{
		return;
	}

	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	if (!BBComp)
	{
		return;
	}

	APawn* Player = UGameplayStatics::GetPlayerPawn(SelfPawn->GetWorld(), 0);
	if (!Player)
	{
		// No player in the world: clear target and mark out of range
		BBComp->ClearValue(TargetActorKey.SelectedKeyName);
		BBComp->SetValueAsBool(IsInAttackRangeKey.SelectedKeyName, false);
		return;
	}

	const float Distance = FVector::Dist(SelfPawn->GetActorLocation(), Player->GetActorLocation());

	// Detection: inside the radius it's assigned as target; outside, it's cleared
	if (Distance <= DetectionRadius)
	{
		BBComp->SetValueAsObject(TargetActorKey.SelectedKeyName, Player);
	}
	else
	{
		BBComp->ClearValue(TargetActorKey.SelectedKeyName);
	}

	// Attack range: bool independent of the target (a target can be visible but far away)
	const bool bInRange = (Distance <= AttackRange);
	BBComp->SetValueAsBool(IsInAttackRangeKey.SelectedKeyName, bInRange);
}
