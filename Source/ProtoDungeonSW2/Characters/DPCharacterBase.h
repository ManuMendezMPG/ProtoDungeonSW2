#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DPCharacterBase.generated.h"

class UAnimMontage;
class USoundBase;

// Delegate disparado cuando cambia la vida; los listeners (UI, BPs) reciben los valores actualizados
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedSignature, float, CurrentHealth, float, MaxHealth);

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

	// Listeners (UI, BP) se suscriben aquí para reaccionar a cambios de vida
	UPROPERTY(BlueprintAssignable, Category = "Stats")
	FOnHealthChangedSignature OnHealthChanged;

	// Reacción al recibir daño desde delante
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> HitReactFrontMontage;

	// Reacción al recibir daño desde detrás
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> HitReactBackMontage;

	// Sonido reproducido en la posición del actor al recibir daño no letal (gruñido).
	// Si es nullptr, no suena nada. La muerte tiene su propio feedback.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Audio")
	TObjectPtr<USoundBase> DamageSound;

protected:
	virtual void BeginPlay() override;

	// Hook de muerte: las clases derivadas pueden override para ragdoll, despawn, drops, etc.
	virtual void OnDeath();

	// Dispara el delegate OnHealthChanged con los valores actuales de vida.
	virtual void BroadcastHealthChange();

	// Reproduce el AnimMontage de hit reaction apropiado según la dirección del atacante.
	virtual void PlayHitReaction(AActor* DamageCauser);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stats")
	float CurrentHealth = 100.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stats")
	bool bIsDead = false;

	// Si true, al morir este character solicita al UDPLevelTransitionSubsystem cargar NextLevelName.
	// Pensado para enemigos clave del nivel; los players y enemigos genéricos lo dejan en false.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transition")
	bool bTriggersLevelTransitionOnDeath = false;

	// Mapa destino al morir cuando bTriggersLevelTransitionOnDeath = true.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transition")
	FName NextLevelName = NAME_None;
};
