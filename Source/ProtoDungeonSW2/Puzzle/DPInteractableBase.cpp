#include "DPInteractableBase.h"

ADPInteractableBase::ADPInteractableBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ADPInteractableBase::Interact(AActor* InteractingActor)
{
	// Empty base implementation. Derived classes override.
}
