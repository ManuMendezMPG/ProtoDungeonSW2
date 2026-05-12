#include "DPGameModePuzzle.h"
#include "../Characters/DPPlayerCharacter.h"
#include "DPPuzzlePlayerController.h"

ADPGameModePuzzle::ADPGameModePuzzle()
{
	DefaultPawnClass = ADPPlayerCharacter::StaticClass();
	PlayerControllerClass = ADPPuzzlePlayerController::StaticClass();
}
