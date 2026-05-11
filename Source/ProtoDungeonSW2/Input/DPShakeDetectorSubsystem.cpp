#include "DPShakeDetectorSubsystem.h"

void UDPShakeDetectorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LastShakeTime = -1000.0f;  // Valor lejano para que el primer shake nunca esté en cooldown.
	InputHistory.Empty();
}

void UDPShakeDetectorSubsystem::Deinitialize()
{
	InputHistory.Empty();
	OnShakeDetected.Clear();
	Super::Deinitialize();
}

void UDPShakeDetectorSubsystem::FeedInputDelta(FVector Delta)
{
	const UWorld* World = GetWorld();
	if (World == nullptr) return;

	const float CurrentTime = World->GetTimeSeconds();

	// Si estamos en cooldown post-shake, ignoramos input.
	if (CurrentTime - LastShakeTime < ShakeCooldown)
	{
		return;
	}

	// Añadir muestra.
	FInputSample Sample;
	Sample.Timestamp = CurrentTime;
	Sample.Magnitude = Delta.Size();
	InputHistory.Add(Sample);

	// Limpiar muestras viejas.
	CleanOldSamples(CurrentTime);

	// Verificar si supera threshold.
	const float Total = GetAccumulatedMagnitude();
	if (Total >= ShakeThreshold)
	{
		LastShakeTime = CurrentTime;
		InputHistory.Empty();
		OnShakeDetected.Broadcast();
	}
}

void UDPShakeDetectorSubsystem::CleanOldSamples(float CurrentTime)
{
	const float CutoffTime = CurrentTime - ShakeWindowSeconds;
	InputHistory.RemoveAll([CutoffTime](const FInputSample& Sample) {
		return Sample.Timestamp < CutoffTime;
	});
}

float UDPShakeDetectorSubsystem::GetAccumulatedMagnitude() const
{
	float Sum = 0.0f;
	for (const FInputSample& Sample : InputHistory)
	{
		Sum += Sample.Magnitude;
	}
	return Sum;
}
