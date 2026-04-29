#include "DPEnemyBase.h"
#include "../AI/DPEnemyAIController.h"
#include "../Combat/DPCombatComponent.h"

ADPEnemyBase::ADPEnemyBase()
{
	// Controlador de IA propio del proyecto; las subclases pueden sustituirlo
	AIControllerClass = ADPEnemyAIController::StaticClass();

	// Que el AIController posea al pawn tanto si está colocado en el mapa como si se spawnea en runtime
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// Los enemigos son más débiles que el jugador por defecto
	MaxHealth = 50.f;
	CurrentHealth = MaxHealth;

	// Componente de combate con valores ajustados para enemigos
	CombatComponent = CreateDefaultSubobject<UDPCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->BasicAttackDamage   = 10.f;
	CombatComponent->BasicAttackRange    = 120.f;
	CombatComponent->BasicAttackCooldown = 1.2f;
	CombatComponent->AttackRadius        = 60.f;
}

void ADPEnemyBase::OnDeath()
{
	// Comportamiento base: mensaje en pantalla, desactivar colisión, parar movimiento
	Super::OnDeath();

	// Pequeño retardo para que la "muerte" sea visible antes de desaparecer
	SetLifeSpan(2.f);
}
