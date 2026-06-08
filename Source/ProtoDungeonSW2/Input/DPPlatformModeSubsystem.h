#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DPPlatformModeSubsystem.generated.h"

UENUM(BlueprintType)
enum class EDPPlatformMode : uint8
{
	Docked     UMETA(DisplayName = "Docked"),
	Handheld   UMETA(DisplayName = "Handheld")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnPlatformModeChangedSignature, EDPPlatformMode, NewMode);

UCLASS()
class PROTODUNGEONSW2_API UDPPlatformModeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Lifecycle.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Accessores BlueprintPure. Source of truth para cualquier sistema que
	// dependa del modo (cámara, HUD, post-process, scalability).
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Platform Mode")
	EDPPlatformMode GetCurrentMode() const { return CurrentMode; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Platform Mode")
	bool IsHandheld() const { return CurrentMode == EDPPlatformMode::Handheld; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Platform Mode")
	bool IsDocked() const { return CurrentMode == EDPPlatformMode::Docked; }

	// Setea directamente un modo (útil para Switch 2 cuando el SDK dé el estado real,
	// o para forzar Docked al entrar al puzzle, etc). Notifica si difiere del actual.
	UFUNCTION(BlueprintCallable, Category = "Platform Mode")
	void SetPlatformMode(EDPPlatformMode NewMode);

	// Alterna entre Docked y Handheld. Lo llama BP_PlayerCharacter al pulsar M en PC.
	UFUNCTION(BlueprintCallable, Category = "Platform Mode")
	void TogglePlatformMode();

	// Delegate broadcast al cambiar el modo. Los listeners (cámara, HUD,
	// post-process, scalability) se suscriben aquí.
	UPROPERTY(BlueprintAssignable, Category = "Platform Mode")
	FOnPlatformModeChangedSignature OnPlatformModeChanged;

	// Modo actual cacheado. Source of truth para todo el sistema.
	// Se mantiene público (BlueprintReadOnly) por compatibilidad con C++ existente
	// que lo lee directamente; los nuevos call sites deberían usar GetCurrentMode().
	UPROPERTY(BlueprintReadOnly, Category = "Platform Mode")
	EDPPlatformMode CurrentMode = EDPPlatformMode::Docked;

private:
	// Timer handle para el polling del SDK en Switch. En PC el timer está
	// inactivo (el toggle es manual).
	FTimerHandle PollingTimerHandle;

	// Consulta al SDK de plataforma el modo físico actual. Solo tiene
	// sentido llamarlo en builds de Switch. En PC devuelve siempre
	// CurrentMode (no hace polling).
	EDPPlatformMode QueryPlatformMode() const;

	// Callback del timer en Switch: consulta el modo y si difiere, lo aplica.
	void PollPlatformMode();
};
