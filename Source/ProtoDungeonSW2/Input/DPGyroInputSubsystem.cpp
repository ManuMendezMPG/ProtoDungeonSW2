#include "DPGyroInputSubsystem.h"

void UDPGyroInputSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CurrentTilt = FVector2D::ZeroVector;
	bGyroActive = false;

	// Register a global tick: return true to keep ticking every frame
	TickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateLambda([this](float DeltaTime) -> bool
		{
			TickCentering(DeltaTime);
			return true;
		})
	);
}

void UDPGyroInputSubsystem::Deinitialize()
{
	FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
	OnTiltChanged.Clear();
	Super::Deinitialize();
}

void UDPGyroInputSubsystem::FeedTiltDelta(FVector2D Delta)
{
	if (!bGyroActive)
	{
		return;
	}

	CurrentTilt += Delta * TiltSensitivity;

	// Clamp per axis to the range [-MaxTilt, MaxTilt]
	CurrentTilt.X = FMath::Clamp(CurrentTilt.X, -MaxTilt, MaxTilt);
	CurrentTilt.Y = FMath::Clamp(CurrentTilt.Y, -MaxTilt, MaxTilt);

	OnTiltChanged.Broadcast(CurrentTilt);
}

void UDPGyroInputSubsystem::ResetTilt()
{
	CurrentTilt = FVector2D::ZeroVector;
	OnTiltChanged.Broadcast(CurrentTilt);
}

void UDPGyroInputSubsystem::SetGyroActive(bool bActive)
{
	bGyroActive = bActive;

	// On disable, leave the tilt centered so state doesn't carry over when we come back
	if (!bGyroActive)
	{
		ResetTilt();
	}
}

void UDPGyroInputSubsystem::TickCentering(float DeltaTime)
{
	if (!bGyroActive)
	{
		return;
	}

	// Already centered: nothing to do (avoids spurious broadcasts every frame)
	if (CurrentTilt.IsNearlyZero())
	{
		return;
	}

	// Vector toward the center, scaled by CenteringSpeed and DeltaTime
	const FVector2D Direction  = -CurrentTilt.GetSafeNormal();
	const float CurrentMag     = CurrentTilt.Size();
	const float CenteringMag   = CenteringSpeed * DeltaTime;

	if (CenteringMag >= CurrentMag)
	{
		// The centering step would overshoot zero: snap to zero
		CurrentTilt = FVector2D::ZeroVector;
	}
	else
	{
		CurrentTilt += Direction * CenteringMag;
	}

	OnTiltChanged.Broadcast(CurrentTilt);
}
