#include "DPCombatComponent.h"
#include "../Characters/DPCharacterBase.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"

UDPCombatComponent::UDPCombatComponent()
{
	// De momento no necesita tick
	PrimaryComponentTick.bCanEverTick = false;
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

	PerformAttack(BasicAttackDamage, BasicAttackRange);
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

	PerformAttack(SpecialAttackDamage, SpecialAttackRange);
}

bool UDPCombatComponent::CanBasicAttack() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const float Now = World->GetTimeSeconds();

	// Bloqueado por su propio cooldown
	if (Now - LastBasicAttackTime < BasicAttackCooldown)
	{
		return false;
	}

	// Bloqueado mientras el especial sigue activo (lockout global)
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

	// Necesitamos un Pawn para tener forward vector y controller para el daño
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

	// Centro de la esfera: delante del owner, a distancia Range
	const FVector OwnerLocation = OwnerPawn->GetActorLocation();
	const FVector OwnerForward  = OwnerPawn->GetActorForwardVector();
	const FVector AttackLocation = OwnerLocation + (OwnerForward * Range);

	// Filtrar por canal de objeto Pawn (queremos solo personajes)
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	// Ignorar al propio owner para evitar auto-daño
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
		// Solo dañamos personajes del proyecto (DPCharacterBase y derivados)
		ADPCharacterBase* Character = Cast<ADPCharacterBase>(Target);
		if (Character && Character != OwnerActor)
		{
			UGameplayStatics::ApplyDamage(Character, Damage, OwnerController, OwnerActor, nullptr);
			++HitCount;
		}
	}

	// Visualización de la esfera de detección
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
