#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DPLevelTransitionSubsystem.generated.h"

UCLASS()
class PROTODUNGEONSW2_API UDPLevelTransitionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Inicia transición a un mapa con delay opcional antes del fade.
	// DelayBeforeFade: segundos a esperar antes de iniciar el fade (útil para dar tiempo a animaciones de muerte, etc).
	// FadeDuration: duración del fade a negro antes de cargar.
	UFUNCTION(BlueprintCallable, Category = "Transition")
	void TransitionToLevel(FName LevelName, float DelayBeforeFade = 0.f, float FadeDuration = 0.5f);

protected:
	// Estado interno para evitar transiciones múltiples solapadas.
	bool bTransitionInProgress = false;

	// Mapa destino tras el fade (almacenado durante el delay).
	FName PendingLevelName;

	// Inicia el fade y programa OpenLevel.
	void StartFadeAndLoad(float FadeDuration);

	// Ejecuta OpenLevel al final del fade.
	void ExecuteOpenLevel();
};
