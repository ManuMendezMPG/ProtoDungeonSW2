#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DPShakeDetectorSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShakeDetectedSignature);

UCLASS()
class PROTODUNGEONSW2_API UDPShakeDetectorSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Tunables expuestos al editor vía console o DefaultEngine.ini (no UPROPERTY EditAnywhere porque subsystem no es asset).
	UPROPERTY(BlueprintReadWrite, Category = "Shake")
	float ShakeWindowSeconds = 0.2f;

	UPROPERTY(BlueprintReadWrite, Category = "Shake")
	float ShakeThreshold = 2000.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Shake")
	float ShakeCooldown = 0.5f;

	// Delegate que cualquier listener puede bindear desde BP o C++.
	UPROPERTY(BlueprintAssignable, Category = "Shake")
	FOnShakeDetectedSignature OnShakeDetected;

	// Punto de entrada para inputs. Llamado desde BP_PlayerCharacter (PC) o futuro plugin de JoyCon (Switch).
	UFUNCTION(BlueprintCallable, Category = "Shake")
	void FeedInputDelta(FVector Delta);

	// Lifecycle.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	// Historial de samples: (timestamp, magnitude).
	struct FInputSample
	{
		float Timestamp;
		float Magnitude;
	};

	TArray<FInputSample> InputHistory;

	float LastShakeTime;  // Para cooldown.

	void CleanOldSamples(float CurrentTime);
	float GetAccumulatedMagnitude() const;
};
