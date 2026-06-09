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
	// Override of the death hook: schedules deferred destruction
	virtual void OnDeath() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UDPCombatComponent> CombatComponent;
};
