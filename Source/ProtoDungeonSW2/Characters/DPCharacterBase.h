#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DPCharacterBase.generated.h"

class UAnimMontage;
class UAnimSequence;
class USoundBase;

// Delegate fired when health changes; listeners (UI, BPs) receive the updated values
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedSignature, float, CurrentHealth, float, MaxHealth);

UCLASS()
class PROTODUNGEONSW2_API ADPCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	ADPCharacterBase();

	// Override of AActor::TakeDamage — UE's standard entry point for damage
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stats")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stats")
	bool IsDead() const { return bIsDead; }

	// Listeners (UI, BP) subscribe here to react to health changes
	UPROPERTY(BlueprintAssignable, Category = "Stats")
	FOnHealthChangedSignature OnHealthChanged;

	// Reaction when taking damage from the front
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> HitReactFrontMontage;

	// Reaction when taking damage from behind
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> HitReactBackMontage;

	// Death animation (one-shot). If assigned, it plays via PlayAnimation
	// and holds the last frame. If nullptr, the actor dies with no specific animation.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimSequence> DeathAnimation;

	// Sound played at the actor's location when taking non-lethal damage (grunt).
	// If nullptr, nothing plays. Death has its own feedback.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Audio")
	TObjectPtr<USoundBase> DamageSound;

protected:
	virtual void BeginPlay() override;

	// Death hook: derived classes can override for ragdoll, despawn, drops, etc.
	virtual void OnDeath();

	// Fires the OnHealthChanged delegate with the current health values.
	virtual void BroadcastHealthChange();

	// Plays the appropriate hit reaction AnimMontage based on the attacker's direction.
	virtual void PlayHitReaction(AActor* DamageCauser);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stats")
	float CurrentHealth = 100.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stats")
	bool bIsDead = false;

	// If true, when this character dies it asks the UDPLevelTransitionSubsystem to load NextLevelName.
	// Intended for key enemies in a level; players and generic enemies leave this as false.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transition")
	bool bTriggersLevelTransitionOnDeath = false;

	// Destination map on death when bTriggersLevelTransitionOnDeath = true.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transition")
	FName NextLevelName = NAME_None;
};
