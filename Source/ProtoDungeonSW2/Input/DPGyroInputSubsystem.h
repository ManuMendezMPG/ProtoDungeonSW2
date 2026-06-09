#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DPGyroInputSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTiltChangedSignature, FVector2D, NewTilt);

UCLASS()
class PROTODUNGEONSW2_API UDPGyroInputSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Current tilt on X (pitch) and Y (roll), values between -1 and 1.
	// Positive X = tilted forward (North in top-down view).
	// Positive Y = tilted to the right.
	UPROPERTY(BlueprintReadOnly, Category = "Gyro")
	FVector2D CurrentTilt;

	// How much of the input delta to pick up each time it arrives (sensitivity).
	UPROPERTY(BlueprintReadWrite, Category = "Gyro")
	float TiltSensitivity = 0.05f;

	// How much the tilt passively "returns to center" when there's no input (per second).
	// 0 = never returns. 1 = fully returns to center in 1 second.
	UPROPERTY(BlueprintReadWrite, Category = "Gyro")
	float CenteringSpeed = 2.0f;

	// Maximum absolute tilt limit on each axis.
	UPROPERTY(BlueprintReadWrite, Category = "Gyro")
	float MaxTilt = 1.0f;

	// Delegate fired when the tilt changes.
	UPROPERTY(BlueprintAssignable, Category = "Gyro")
	FOnTiltChangedSignature OnTiltChanged;

	// Input entry point from the caller (BP on PC, JoyCon plugin on Switch).
	// Normalized delta: the caller must pass the already-scaled delta (e.g. mouse delta).
	UFUNCTION(BlueprintCallable, Category = "Gyro")
	void FeedTiltDelta(FVector2D Delta);

	// Resets the tilt to center manually.
	UFUNCTION(BlueprintCallable, Category = "Gyro")
	void ResetTilt();

	// To enable/disable processing (when we're outside puzzle mode).
	UFUNCTION(BlueprintCallable, Category = "Gyro")
	void SetGyroActive(bool bActive);

	UPROPERTY(BlueprintReadOnly, Category = "Gyro")
	bool bGyroActive;

	// Lifecycle.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

protected:
	// Centering tick: subsystems don't have a native tick, we use FTSTicker.
	void TickCentering(float DeltaTime);

	FTSTicker::FDelegateHandle TickerHandle;
};
