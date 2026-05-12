#include "DPGyroInputSubsystem.h"

void UDPGyroInputSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CurrentTilt = FVector2D::ZeroVector;
	bGyroActive = false;

	// Registrar tick global: devuelve true para seguir ticking cada frame
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

	// Clamp por eje al rango [-MaxTilt, MaxTilt]
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

	// Al desactivar, dejar el tilt centrado para no arrastrar estado al volver
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

	// Ya centrado: nada que hacer (evita broadcast espurio cada frame)
	if (CurrentTilt.IsNearlyZero())
	{
		return;
	}

	// Vector hacia el centro, escalado por CenteringSpeed y DeltaTime
	const FVector2D Direction  = -CurrentTilt.GetSafeNormal();
	const float CurrentMag     = CurrentTilt.Size();
	const float CenteringMag   = CenteringSpeed * DeltaTime;

	if (CenteringMag >= CurrentMag)
	{
		// El paso de centering nos pasaría del centro: clavamos en zero
		CurrentTilt = FVector2D::ZeroVector;
	}
	else
	{
		CurrentTilt += Direction * CenteringMag;
	}

	OnTiltChanged.Broadcast(CurrentTilt);
}
