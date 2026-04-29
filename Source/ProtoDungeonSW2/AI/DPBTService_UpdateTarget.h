#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "DPBTService_UpdateTarget.generated.h"

UCLASS(ClassGroup = "AI")
class PROTODUNGEONSW2_API UDPBTService_UpdateTarget : public UBTService_BlackboardBase
{
	GENERATED_BODY()

public:
	UDPBTService_UpdateTarget();

	// Radio máximo al que el enemigo "ve" al jugador
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (ClampMin = "0.0"))
	float DetectionRadius = 1000.f;

	// Distancia a la que se considera al jugador en rango de ataque
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (ClampMin = "0.0"))
	float AttackRange = 130.f;

	// Key del BB donde escribir la referencia al jugador detectado (Object: AActor)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	FBlackboardKeySelector TargetActorKey;

	// Key del BB donde escribir si el jugador está en rango de ataque (Bool)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	FBlackboardKeySelector IsInAttackRangeKey;

protected:
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
