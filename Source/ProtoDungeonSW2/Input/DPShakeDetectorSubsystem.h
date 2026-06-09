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
	// Tunables exposed to the editor via console or DefaultEngine.ini (not UPROPERTY EditAnywhere because a subsystem is not an asset).
	UPROPERTY(BlueprintReadWrite, Category = "Shake")
	float ShakeWindowSeconds = 0.2f;

	UPROPERTY(BlueprintReadWrite, Category = "Shake")
	float ShakeThreshold = 2000.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Shake")
	float ShakeCooldown = 0.5f;

	// Delegate any listener can bind from BP or C++.
	UPROPERTY(BlueprintAssignable, Category = "Shake")
	FOnShakeDetectedSignature OnShakeDetected;

	// Input entry point. Called from BP_PlayerCharacter (PC) or a future JoyCon plugin (Switch).
	UFUNCTION(BlueprintCallable, Category = "Shake")
	void FeedInputDelta(FVector Delta);

	// Lifecycle.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	// Sample history: (timestamp, magnitude).
	struct FInputSample
	{
		float Timestamp;
		float Magnitude;
	};

	TArray<FInputSample> InputHistory;

	float LastShakeTime;  // For cooldown.

	void CleanOldSamples(float CurrentTime);
	float GetAccumulatedMagnitude() const;
};
