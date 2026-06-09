#include "DPCombatComponent.h"
#include "../Characters/DPCharacterBase.h"
#include "Animation/AnimMontage.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"

UDPCombatComponent::UDPCombatComponent()
{
	// No tick needed for now
	PrimaryComponentTick.bCanEverTick = false;

	CurrentAttackType = EDPAttackType::None;
}

void UDPCombatComponent::TryBasicAttack()
{
	if (!CanBasicAttack())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor(150, 150, 150), TEXT("Basic on cooldown"));
		}
		return;
	}

	LastBasicAttackTime = GetWorld()->GetTimeSeconds();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Basic Attack!"));
	}

	if (BasicAttackMontage)
	{
		// Flow with animation: play the montage and wait for the notify to apply damage
		ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
		if (OwnerCharacter)
		{
			OwnerCharacter->PlayAnimMontage(BasicAttackMontage);
		}
		CurrentAttackType = EDPAttackType::Basic;
	}
	else
	{
		// Legacy flow (no montage, e.g. enemies): instant damage
		PerformAttack(BasicAttackDamage, BasicAttackRange);
	}
}

void UDPCombatComponent::TrySpecialAttack()
{
	if (!CanSpecialAttack())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor(150, 150, 150), TEXT("Special on cooldown"));
		}
		return;
	}

	LastSpecialAttackTime = GetWorld()->GetTimeSeconds();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, TEXT("Special Attack!"));
	}

	if (SpecialAttackMontage)
	{
		// Flow with animation: play the montage and wait for the notify to apply damage
		ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
		if (OwnerCharacter)
		{
			OwnerCharacter->PlayAnimMontage(SpecialAttackMontage);
		}
		CurrentAttackType = EDPAttackType::Special;
	}
	else
	{
		// Legacy flow (no montage): instant damage
		PerformAttack(SpecialAttackDamage, SpecialAttackRange);
	}
}

void UDPCombatComponent::OnDamageNotify()
{
	if (CurrentAttackType == EDPAttackType::Basic)
	{
		PerformAttack(BasicAttackDamage, BasicAttackRange);
	}
	else if (CurrentAttackType == EDPAttackType::Special)
	{
		PerformAttack(SpecialAttackDamage, SpecialAttackRange);
	}

	// Reset state — the next attack will set the correct type
	CurrentAttackType = EDPAttackType::None;
}

bool UDPCombatComponent::CanBasicAttack() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const float Now = World->GetTimeSeconds();

	// Blocked by its own cooldown
	if (Now - LastBasicAttackTime < BasicAttackCooldown)
	{
		return false;
	}

	// Blocked while the special is still active (global lockout)
	if (Now - LastSpecialAttackTime < SpecialAttackCooldown)
	{
		return false;
	}

	return true;
}

bool UDPCombatComponent::CanSpecialAttack() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const float Now = World->GetTimeSeconds();
	return (Now - LastSpecialAttackTime) >= SpecialAttackCooldown;
}

void UDPCombatComponent::PerformAttack(float Damage, float Range)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	// We need a Pawn to have a forward vector and a controller for damage
	APawn* OwnerPawn = Cast<APawn>(OwnerActor);
	if (!OwnerPawn)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Sphere center: in front of the owner, at distance Range
	const FVector OwnerLocation = OwnerPawn->GetActorLocation();
	const FVector OwnerForward  = OwnerPawn->GetActorForwardVector();
	const FVector AttackLocation = OwnerLocation + (OwnerForward * Range);

	// Filter by Pawn object channel (we only want characters)
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	// Ignore the owner itself to avoid self-damage
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(OwnerActor);

	TArray<AActor*> OverlappedActors;
	UKismetSystemLibrary::SphereOverlapActors(
		World,
		AttackLocation,
		AttackRadius,
		ObjectTypes,
		nullptr,
		ActorsToIgnore,
		OverlappedActors
	);

	AController* OwnerController = OwnerPawn->GetController();

	int32 HitCount = 0;
	for (AActor* Target : OverlappedActors)
	{
		// Only damage project characters (DPCharacterBase and derivatives)
		ADPCharacterBase* Character = Cast<ADPCharacterBase>(Target);
		if (Character && Character != OwnerActor)
		{
			UGameplayStatics::ApplyDamage(Character, Damage, OwnerController, OwnerActor, nullptr);
			++HitCount;

			// Hit feedback at the target's location (once per target)
			if (HitImpactSound)
			{
				UGameplayStatics::PlaySoundAtLocation(World, HitImpactSound, Character->GetActorLocation());
			}
		}
	}

	// Detection sphere visualization
	if (bDrawDebugAttacks)
	{
		DrawDebugSphere(World, AttackLocation, AttackRadius, 12, FColor::Red, false, 1.f);
	}

	if (GEngine)
	{
		const FString Msg = FString::Printf(TEXT("Hit %d target(s)"), HitCount);
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Cyan, Msg);
	}
}
