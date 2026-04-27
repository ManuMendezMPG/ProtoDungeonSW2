#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DPCombatComponent.generated.h"

UCLASS(Blueprintable, ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class PROTODUNGEONSW2_API UDPCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDPCombatComponent();

	// Ataque básico (botón)
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void TryBasicAttack();

	// Ataque especial (gesto JoyCon, simulado en PC)
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void TrySpecialAttack();
};
