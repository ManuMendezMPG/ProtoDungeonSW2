#include "DPCombatComponent.h"
#include "Engine/Engine.h"

UDPCombatComponent::UDPCombatComponent()
{
	// De momento no necesita tick
	PrimaryComponentTick.bCanEverTick = false;
}

void UDPCombatComponent::TryBasicAttack()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Basic Attack!"));
	}
}

void UDPCombatComponent::TrySpecialAttack()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Orange, TEXT("Special Attack!"));
	}
}
