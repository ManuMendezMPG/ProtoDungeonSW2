#pragma once

#include "CoreMinimal.h"
#include "DPCharacterBase.h"
#include "DPEnemyBase.generated.h"

class UDPCombatComponent;

UCLASS()
class PROTODUNGEONSW2_API ADPEnemyBase : public ADPCharacterBase
{
	GENERATED_BODY()

public:
	ADPEnemyBase();

protected:
	// Override del hook de muerte: programa destrucción diferida
	virtual void OnDeath() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UDPCombatComponent> CombatComponent;
};
