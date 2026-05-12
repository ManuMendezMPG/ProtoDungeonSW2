#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DPPlatformModeSubsystem.generated.h"

UENUM(BlueprintType)
enum class EDPPlatformMode : uint8
{
	Docked,
	Handheld
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlatformModeChangedSignature, EDPPlatformMode, NewMode);

UCLASS()
class PROTODUNGEONSW2_API UDPPlatformModeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Modo actual. Por defecto Docked al iniciar.
	UPROPERTY(BlueprintReadOnly, Category = "Platform")
	EDPPlatformMode CurrentMode;

	// Delegate que se dispara al cambiar de modo.
	UPROPERTY(BlueprintAssignable, Category = "Platform")
	FOnPlatformModeChangedSignature OnPlatformModeChanged;

	// Alterna entre Docked y Handheld. Lo llama BP_PlayerCharacter al pulsar M en PC.
	UFUNCTION(BlueprintCallable, Category = "Platform")
	void TogglePlatformMode();

	// Setea directamente un modo (útil para Switch 2 cuando el SDK dé el estado real).
	UFUNCTION(BlueprintCallable, Category = "Platform")
	void SetPlatformMode(EDPPlatformMode NewMode);

	// Lifecycle.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
};
