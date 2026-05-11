#include "DPCharacterBase.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

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

		// Hit reaction solo si sobrevive el golpe (la muerte gestiona su propia animación)
		if (CurrentHealth > 0.f)
		{
			PlayHitReaction(DamageCauser);

			// Grunt de daño no letal en la posición del actor (3D)
			if (DamageSound)
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), DamageSound, GetActorLocation());
			}
		}

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

void ADPCharacterBase::PlayHitReaction(AActor* DamageCauser)
{
	// Default a Front: si no hay causer (autodaño, fallthrough, etc.) o no se puede determinar dirección
	UAnimMontage* MontageToPlay = HitReactFrontMontage;

	if (DamageCauser != nullptr)
	{
		// Vector desde el defensor al atacante, normalizado (GetSafeNormal evita div/0 si coinciden)
		const FVector ToAttacker = (DamageCauser->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		const FVector Forward    = GetActorForwardVector();

		// Dot > 0 => atacante delante; Dot < 0 => detrás. Umbral en 90°.
		const float DotProduct = FVector::DotProduct(Forward, ToAttacker);

		if (DotProduct < 0.f && HitReactBackMontage != nullptr)
		{
			MontageToPlay = HitReactBackMontage;
		}
	}

	// Detener cualquier montage en curso para que un nuevo PlayAnimMontage no sea ignorado
	// (PlayAnimMontage no reemplaza un montage que aún se está reproduciendo)
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		{
			AnimInstance->Montage_Stop(0.1f);
		}
	}

	if (MontageToPlay != nullptr)
	{
		PlayAnimMontage(MontageToPlay);
	}
}
