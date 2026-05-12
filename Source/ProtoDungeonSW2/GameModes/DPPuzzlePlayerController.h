#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DPPuzzlePlayerController.generated.h"

class ADPPlayerCharacter;
class ADPPuzzleBall;
class UInputAction;
class UInputMappingContext;
enum class EDPPlatformMode : uint8;

UCLASS()
class PROTODUNGEONSW2_API ADPPuzzlePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ADPPuzzlePlayerController();

protected:
	// Pawn de tipo player character. Se encuentra en el mapa al iniciar.
	UPROPERTY(BlueprintReadOnly, Category = "Puzzle")
	TObjectPtr<ADPPlayerCharacter> PlayerCharacterPawn;

	// Pawn de tipo bola. Se encuentra en el mapa al iniciar.
	UPROPERTY(BlueprintReadOnly, Category = "Puzzle")
	TObjectPtr<ADPPuzzleBall> BallPawn;

	// Contexto de input registrado en BeginPlay (asignar el asset desde el BP del PlayerController).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	// Acción que alterna Docked/Handheld. Sobrevive al cambio de pawn porque vive en el controller.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ToggleModeAction;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;

	// Llamado cuando el modo del subsistema cambia. Cambia la posesión al pawn correspondiente.
	UFUNCTION()
	void OnPlatformModeChanged(EDPPlatformMode NewMode);

	// Helpers para encontrar los pawns en el mapa.
	void FindPawnsInLevel();

private:
	void HandleToggleMode();
};
