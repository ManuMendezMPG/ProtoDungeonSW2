#include "DPGameModeBase.h"
#include "../Characters/DPPlayerCharacter.h"

ADPGameModeBase::ADPGameModeBase()
{
	DefaultPawnClass = ADPPlayerCharacter::StaticClass();
}
