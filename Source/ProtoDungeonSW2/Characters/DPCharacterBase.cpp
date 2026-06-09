#include "DPCharacterBase.h"
#include "../GameModes/DPLevelTransitionSubsystem.h"
#include "AIController.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "BrainComponent.h"
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

	// Initialize current health to max when the game starts
	CurrentHealth = MaxHealth;

	// Notify the initial value to any already-subscribed listener (e.g. UI widget)
	BroadcastHealthChange();
}

float ADPCharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// If already dead, ignore additional damage
	if (bIsDead)
	{
		return 0.f;
	}

	// Call Super so UE modulates the damage based on DamageType, multipliers, etc.
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

		// Hit reaction only if the character survives the hit (death handles its own animation)
		if (CurrentHealth > 0.f)
		{
			PlayHitReaction(DamageCauser);

			// Non-lethal damage grunt at the actor's location (3D)
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
	// Play the death animation if assigned. PlayAnimation with
	// Loop=false keeps the pose at the last frame; we don't return to
	// the AnimBP (no need — the character is dead)
	if (DeathAnimation && GetMesh())
	{
		GetMesh()->PlayAnimation(DeathAnimation, false);
	}

	// Stop any active montage (e.g. an attack mid-flight).
	// This prevents pending notifies like DamageMoment from triggering
	// post-death damage. We use a short but non-instant BlendOut so the
	// transition to the DeathAnimation pose doesn't look abrupt
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		{
			AnimInstance->Montage_Stop(0.1f);
		}
	}

	if (GEngine)
	{
		const FString Msg = FString::Printf(TEXT("[%s] died"), *GetName());
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, Msg);
	}

	// Disable collision so the corpse doesn't get in the way of other actors
	SetActorEnableCollision(false);

	// Stop movement
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->DisableMovement();
	}

	// If the actor is controlled by an AIController, stop its Behavior
	// Tree and unpossess the pawn. Without this the AI keeps evaluating
	// the BT during the transition delay and can execute actions
	// (chase, attack) on a corpse
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		if (UBrainComponent* Brain = AIController->GetBrainComponent())
		{
			Brain->StopLogic(TEXT("Character died"));
		}
	}

	// Trigger the level transition if this character is the level's "trigger" (e.g. the final enemy of L_Combat)
	if (bTriggersLevelTransitionOnDeath && NextLevelName != NAME_None)
	{
		if (UDPLevelTransitionSubsystem* TransitionSubsystem = GetGameInstance()->GetSubsystem<UDPLevelTransitionSubsystem>())
		{
			TransitionSubsystem->TransitionToLevel(NextLevelName, 2.0f, 0.5f);
		}
	}
}

void ADPCharacterBase::BroadcastHealthChange()
{
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void ADPCharacterBase::PlayHitReaction(AActor* DamageCauser)
{
	// Default to Front: if there is no causer (self-damage, fallthrough, etc.) or direction can't be determined
	UAnimMontage* MontageToPlay = HitReactFrontMontage;

	if (DamageCauser != nullptr)
	{
		// Vector from defender to attacker, normalized (GetSafeNormal avoids div/0 if they coincide)
		const FVector ToAttacker = (DamageCauser->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		const FVector Forward    = GetActorForwardVector();

		// Dot > 0 => attacker in front; Dot < 0 => behind. Threshold at 90°.
		const float DotProduct = FVector::DotProduct(Forward, ToAttacker);

		if (DotProduct < 0.f && HitReactBackMontage != nullptr)
		{
			MontageToPlay = HitReactBackMontage;
		}
	}

	// Stop any montage in progress so a new PlayAnimMontage isn't ignored
	// (PlayAnimMontage does not replace a montage that is still playing)
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
