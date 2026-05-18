#include "DPInteractableBase.h"

ADPInteractableBase::ADPInteractableBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ADPInteractableBase::Interact(AActor* InteractingActor)
{
	// Implementación base vacía. Las clases derivadas hacen override.
}
