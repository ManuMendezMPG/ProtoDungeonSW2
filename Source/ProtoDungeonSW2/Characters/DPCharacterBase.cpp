#include "DPCharacterBase.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"

ADPCharacterBase::ADPCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ADPCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	// Inicializar vida actual al máximo al comenzar el juego
	CurrentHealth = MaxHealth;

	// Notificar el valor inicial a cualquier listener ya suscrito (p.ej. widget de UI)
	BroadcastHealthChange();
}

float ADPCharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// Si ya está muerto, ignorar daño adicional
	if (bIsDead)
	{
		return 0.f;
	}

	// Llamar a Super para que UE module el daño según DamageType, multipliers, etc.
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (ActualDamage > 0.f)
	{
		CurrentHealth -= ActualDamage;

		if (GEngine)
		{
			const FString Msg = FString::Printf(TEXT("[%s] took %.1f damage. HP: %.1f/%.1f"),
				*GetName(), ActualDamage, FMath::Max(CurrentHealth, 0.f), MaxHealth);
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, Msg);
		}

		BroadcastHealthChange();

		if (CurrentHealth <= 0.f)
		{
			CurrentHealth = 0.f;
			bIsDead = true;
			BroadcastHealthChange();
			OnDeath();
		}
	}

	return ActualDamage;
}

float ADPCharacterBase::GetHealthPercent() const
{
	return MaxHealth > 0.f ? CurrentHealth / MaxHealth : 0.f;
}

void ADPCharacterBase::OnDeath()
{
	if (GEngine)
	{
		const FString Msg = FString::Printf(TEXT("[%s] died"), *GetName());
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, Msg);
	}

	// Desactivar colisión para no estorbar a otros actores
	SetActorEnableCollision(false);

	// Detener movimiento
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->DisableMovement();
	}
}

void ADPCharacterBase::BroadcastHealthChange()
{
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}
