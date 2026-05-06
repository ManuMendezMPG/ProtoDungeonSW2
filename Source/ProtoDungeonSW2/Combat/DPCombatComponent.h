#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DPCombatComponent.generated.h"

class UAnimMontage;

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

	// Ataque básico (botón)
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void TryBasicAttack();

	// Ataque especial (gesto JoyCon, simulado en PC)
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void TrySpecialAttack();

	// Llamada desde Anim Notify cuando el momento de daño de un ataque debe aplicarse.
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void OnDamageNotify();

	// Daño del ataque básico
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float BasicAttackDamage = 25.f;

	// Daño del ataque especial
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float SpecialAttackDamage = 50.f;

	// Distancia desde el owner al centro de la esfera de impacto del ataque básico
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float BasicAttackRange = 150.f;

	// Distancia desde el owner al centro de la esfera de impacto del ataque especial
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float SpecialAttackRange = 250.f;

	// Radio de la esfera de detección de impactos
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackRadius = 80.f;

	// Si está activo, dibuja en pantalla la esfera de detección durante 1s
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Debug")
	bool bDrawDebugAttacks = true;

	// Cooldown del ataque básico (segundos)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "0.0"))
	float BasicAttackCooldown = 0.4f;

	// Cooldown del ataque especial (segundos). Bloquea TANTO básico como especial mientras está activo.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "0.0"))
	float SpecialAttackCooldown = 1.0f;

	// Montage del ataque básico. Si está asignado, TryBasicAttack lo reproduce y el daño
	// se aplica vía OnDamageNotify (llamado desde un Anim Notify). Si es nullptr, el daño
	// se aplica al instante (flujo legacy usado por enemigos sin montage).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> BasicAttackMontage;

	// Montage del ataque especial (mismo patrón que BasicAttackMontage).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> SpecialAttackMontage;

	// ¿Puede dispararse ahora un ataque básico? (false si su propio CD activo o si el especial bloquea)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat")
	bool CanBasicAttack() const;

	// ¿Puede dispararse ahora un ataque especial? (solo lo bloquea su propio CD)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat")
	bool CanSpecialAttack() const;

protected:
	// Lógica compartida: detecta personajes en una esfera delante del owner y les aplica daño
	void PerformAttack(float Damage, float Range);

	// Tipo de ataque actualmente en curso (None si ningún montage activo). Lo consulta
	// OnDamageNotify para saber qué daño/rango aplicar.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|State")
	EDPAttackType CurrentAttackType;

private:
	// Estado runtime: timestamp del último uso (en segundos de juego). Inicializado lejos
	// en el pasado para que el primer ataque siempre pueda dispararse. No es UPROPERTY:
	// no se expone al editor ni se serializa.
	float LastBasicAttackTime = -1000.f;
	float LastSpecialAttackTime = -1000.f;
};
