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

	// Tick periódico: cada 0.2s con +/-0.05s de variación para desincronizar enemigos
	Interval = 0.2f;
	RandomDeviation = 0.05f;

	// Filtros de tipo en el editor: TargetActor solo acepta keys de tipo Object/AActor;
	// IsInAttackRange solo acepta keys de tipo Bool
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UDPBTService_UpdateTarget, TargetActorKey), AActor::StaticClass());
	IsInAttackRangeKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UDPBTService_UpdateTarget, IsInAttackRangeKey));
}

void UDPBTService_UpdateTarget::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	// Resolver los IDs de las keys seleccionadas contra el BB del asset
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
		// Sin jugador en el mundo: limpiar target y marcar fuera de rango
		BBComp->ClearValue(TargetActorKey.SelectedKeyName);
		BBComp->SetValueAsBool(IsInAttackRangeKey.SelectedKeyName, false);
		return;
	}

	const float Distance = FVector::Dist(SelfPawn->GetActorLocation(), Player->GetActorLocation());

	// Detección: dentro del radio se asigna como target; fuera, se limpia
	if (Distance <= DetectionRadius)
	{
		BBComp->SetValueAsObject(TargetActorKey.SelectedKeyName, Player);
	}
	else
	{
		BBComp->ClearValue(TargetActorKey.SelectedKeyName);
	}

	// Rango de ataque: bool independiente del target (puede haber target visible pero lejos)
	const bool bInRange = (Distance <= AttackRange);
	BBComp->SetValueAsBool(IsInAttackRangeKey.SelectedKeyName, bInRange);
}
