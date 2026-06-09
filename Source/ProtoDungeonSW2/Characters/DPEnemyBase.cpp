#include "DPEnemyBase.h"
#include "../AI/DPEnemyAIController.h"
#include "../Combat/DPCombatComponent.h"

ADPEnemyBase::ADPEnemyBase()
{
	// Project-specific AI controller; subclasses can replace it
	AIControllerClass = ADPEnemyAIController::StaticClass();

	// Have the AIController possess the pawn whether it's placed in the map or spawned at runtime
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// Enemies are weaker than the player by default
	MaxHealth = 50.f;
	CurrentHealth = MaxHealth;

	// Combat component with values tuned for enemies
	CombatComponent = CreateDefaultSubobject<UDPCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->BasicAttackDamage   = 10.f;
	CombatComponent->BasicAttackRange    = 120.f;
	CombatComponent->BasicAttackCooldown = 1.2f;
	CombatComponent->AttackRadius        = 60.f;
}

void ADPEnemyBase::OnDeath()
{
	// Base behavior: on-screen message, disable collision, stop movement
	Super::OnDeath();

	// Small delay so the "death" is visible before disappearing
	SetLifeSpan(2.f);
}
