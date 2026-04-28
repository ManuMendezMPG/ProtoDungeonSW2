#include "DPEnemyBase.h"
#include "AIController.h"

ADPEnemyBase::ADPEnemyBase()
{
	// Controlador de IA estándar; las subclases pueden sustituirlo por uno propio
	AIControllerClass = AAIController::StaticClass();

	// Que el AIController posea al pawn tanto si está colocado en el mapa como si se spawnea en runtime
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// Los enemigos son más débiles que el jugador por defecto
	MaxHealth = 50.f;
	CurrentHealth = MaxHealth;
}

void ADPEnemyBase::OnDeath()
{
	// Comportamiento base: mensaje en pantalla, desactivar colisión, parar movimiento
	Super::OnDeath();

	// Pequeño retardo para que la "muerte" sea visible antes de desaparecer
	SetLifeSpan(2.f);
}
