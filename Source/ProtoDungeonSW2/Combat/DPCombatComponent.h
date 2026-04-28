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

	// ¿Puede dispararse ahora un ataque básico? (false si su propio CD activo o si el especial bloquea)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat")
	bool CanBasicAttack() const;

	// ¿Puede dispararse ahora un ataque especial? (solo lo bloquea su propio CD)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat")
	bool CanSpecialAttack() const;

protected:
	// Lógica compartida: detecta personajes en una esfera delante del owner y les aplica daño
	void PerformAttack(float Damage, float Range);

private:
	// Estado runtime: timestamp del último uso (en segundos de juego). Inicializado lejos
	// en el pasado para que el primer ataque siempre pueda dispararse. No es UPROPERTY:
	// no se expone al editor ni se serializa.
	float LastBasicAttackTime = -1000.f;
	float LastSpecialAttackTime = -1000.f;
};
