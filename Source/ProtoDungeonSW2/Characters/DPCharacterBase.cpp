#include "DPCharacterBase.h"

ADPCharacterBase::ADPCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ADPCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}
