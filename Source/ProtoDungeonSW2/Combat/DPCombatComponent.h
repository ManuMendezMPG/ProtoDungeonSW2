#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DPCombatComponent.generated.h"

class UAnimMontage;
class USoundBase;

UENUM(BlueprintType)
enum class EDPAttackType : uint8
{
	None,
	Basic,
	Special
};

UCLASS(Blueprintable, ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class PROTODUNGEONSW2_API UDPCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDPCombatComponent();

	// Basic attack (button)
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void TryBasicAttack();

	// Special attack (JoyCon gesture, simulated on PC)
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void TrySpecialAttack();

	// Called from an Anim Notify when an attack's damage moment should be applied.
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void OnDamageNotify();

	// Basic attack damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float BasicAttackDamage = 25.f;

	// Special attack damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float SpecialAttackDamage = 50.f;

	// Distance from the owner to the center of the basic attack's hit sphere
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float BasicAttackRange = 80.f;

	// Distance from the owner to the center of the special attack's hit sphere
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float SpecialAttackRange = 95.f;

	// Radius of the hit detection sphere
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackRadius = 45.f;

	// If enabled, draws the detection sphere on screen for 1s
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Debug")
	bool bDrawDebugAttacks = true;

	// Basic attack cooldown (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "0.0"))
	float BasicAttackCooldown = 0.5f;

	// Special attack cooldown (seconds). Blocks BOTH basic and special while active.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "0.0"))
	float SpecialAttackCooldown = 1.0f;

	// Basic attack montage. If assigned, TryBasicAttack plays it and damage is
	// applied via OnDamageNotify (called from an Anim Notify). If nullptr, damage
	// is applied instantly (legacy flow used by enemies without a montage).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> BasicAttackMontage;

	// Special attack montage (same pattern as BasicAttackMontage).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> SpecialAttackMontage;

	// Sound played at the location of each actor hit in PerformAttack.
	// If nullptr or the attack hits nothing, nothing plays.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Audio")
	TObjectPtr<USoundBase> HitImpactSound;

	// Can a basic attack fire right now? (false if its own CD is active or the special blocks it)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat")
	bool CanBasicAttack() const;

	// Can a special attack fire right now? (only its own CD blocks it)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat")
	bool CanSpecialAttack() const;

protected:
	// Shared logic: detects characters in a sphere in front of the owner and applies damage to them
	void PerformAttack(float Damage, float Range);

	// Attack type currently in progress (None if no montage active). Consulted by
	// OnDamageNotify to know which damage/range to apply.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
	EDPAttackType CurrentAttackType;

private:
	// Runtime state: timestamp of last use (in game seconds). Initialized far
	// in the past so the first attack can always fire. Not a UPROPERTY:
	// not exposed to the editor and not serialized.
	float LastBasicAttackTime = -1000.f;
	float LastSpecialAttackTime = -1000.f;
};
