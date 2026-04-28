#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DPCharacterBase.generated.h"

UCLASS()
class PROTODUNGEONSW2_API ADPCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	ADPCharacterBase();

	// Override de AActor::TakeDamage — punto de entrada estándar de UE para daño
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stats")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stats")
	bool IsDead() const { return bIsDead; }

protected:
	virtual void BeginPlay() override;

	// Hook de muerte: las clases derivadas pueden override para ragdoll, despawn, drops, etc.
	virtual void OnDeath();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stats")
	float CurrentHealth = 100.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stats")
	bool bIsDead = false;
};
