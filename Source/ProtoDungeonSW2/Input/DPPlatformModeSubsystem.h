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

	// BlueprintPure accessors. Source of truth for any system that
	// depends on the mode (camera, HUD, post-process, scalability).
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Platform Mode")
	EDPPlatformMode GetCurrentMode() const { return CurrentMode; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Platform Mode")
	bool IsHandheld() const { return CurrentMode == EDPPlatformMode::Handheld; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Platform Mode")
	bool IsDocked() const { return CurrentMode == EDPPlatformMode::Docked; }

	// Set a mode directly (useful for Switch 2 when the SDK exposes the real state,
	// or to force Docked when entering the puzzle, etc). Notifies if it differs from the current one.
	UFUNCTION(BlueprintCallable, Category = "Platform Mode")
	void SetPlatformMode(EDPPlatformMode NewMode);

	// Toggles between Docked and Handheld. Called by BP_PlayerCharacter when M is pressed on PC.
	UFUNCTION(BlueprintCallable, Category = "Platform Mode")
	void TogglePlatformMode();

	// Delegate broadcast on mode change. Listeners (camera, HUD,
	// post-process, scalability) subscribe here.
	UPROPERTY(BlueprintAssignable, Category = "Platform Mode")
	FOnPlatformModeChangedSignature OnPlatformModeChanged;

	// Cached current mode. Source of truth for the whole system.
	// Kept public (BlueprintReadOnly) for compatibility with existing C++
	// that reads it directly; new call sites should use GetCurrentMode().
	UPROPERTY(BlueprintReadOnly, Category = "Platform Mode")
	EDPPlatformMode CurrentMode = EDPPlatformMode::Docked;

private:
	// Timer handle for SDK polling on Switch. On PC the timer is
	// inactive (toggling is manual).
	FTimerHandle PollingTimerHandle;

	// Queries the platform SDK for the current physical mode. Only meaningful
	// in Switch builds. On PC it always returns
	// CurrentMode (no polling).
	EDPPlatformMode QueryPlatformMode() const;

	// Timer callback on Switch: queries the mode and if it differs, applies it.
	void PollPlatformMode();
};
