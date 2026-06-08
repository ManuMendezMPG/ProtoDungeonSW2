#include "DPPlatformModeSubsystem.h"
#include "../UI/DPMessageSubsystem.h"
#include "CoreGlobals.h"
#include "Engine/World.h"
#include "Scalability.h"
#include "TimerManager.h"

void UDPPlatformModeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Estado inicial: consulta al SDK en Switch, o Docked por defecto en PC
	CurrentMode = QueryPlatformMode();

#if PLATFORM_SWITCH
	// En Switch arrancamos polling cada 0.5s para detectar cuando el
	// usuario desacopla/acopla físicamente la consola. En PC no hace
	// falta porque el cambio es manual vía TogglePlatformMode().
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			PollingTimerHandle, this,
			&UDPPlatformModeSubsystem::PollPlatformMode,
			0.5f, true);
	}
#endif
}

void UDPPlatformModeSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PollingTimerHandle);
	}

	OnPlatformModeChanged.Clear();
	Super::Deinitialize();
}

void UDPPlatformModeSubsystem::SetPlatformMode(EDPPlatformMode NewMode)
{
	if (NewMode == CurrentMode)
	{
		return;
	}

	CurrentMode = NewMode;

	// Aplicar preset de calidad apropiado al modo. En Switch 2 portátil
	// baja el clock de GPU/CPU, así que reducimos calidad para mantener
	// framerate; en docked subimos al máximo.
	// Niveles: 0=Low, 1=Medium, 2=High, 3=Epic, 4=Cinematic
	Scalability::FQualityLevels Quality = Scalability::GetQualityLevels();
	const int32 TargetLevel = (CurrentMode == EDPPlatformMode::Handheld) ? 1 : 3;
	Quality.SetFromSingleQualityLevel(TargetLevel);
	Scalability::SetQualityLevels(Quality);
	Scalability::SaveState(GGameUserSettingsIni);

	// Feedback visual del cambio de modo via el HUD message subsystem
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UDPMessageSubsystem* MessageSubsystem =
			GI->GetSubsystem<UDPMessageSubsystem>())
		{
			const FString ModeMessage = (CurrentMode == EDPPlatformMode::Handheld)
				? TEXT("Handheld mode")
				: TEXT("Docked mode");
			MessageSubsystem->RequestMessage(ModeMessage, 2.0f);
		}
	}

	OnPlatformModeChanged.Broadcast(CurrentMode);
}

void UDPPlatformModeSubsystem::TogglePlatformMode()
{
	const EDPPlatformMode NextMode = (CurrentMode == EDPPlatformMode::Docked)
		? EDPPlatformMode::Handheld
		: EDPPlatformMode::Docked;

	SetPlatformMode(NextMode);
}

EDPPlatformMode UDPPlatformModeSubsystem::QueryPlatformMode() const
{
#if PLATFORM_SWITCH
	// TODO Switch 2: reemplazar este placeholder con la API real del SDK
	// de Nintendo cuando esté disponible en el devkit. El namespace y
	// función probables son:
	//
	//   #include <nn/oe.h>
	//   const nn::oe::OperationMode Mode = nn::oe::GetOperationMode();
	//   return (Mode == nn::oe::OperationMode_Handheld)
	//       ? EDPPlatformMode::Handheld
	//       : EDPPlatformMode::Docked;
	//
	// Verificar en el SDK de Switch 2 (puede haber cambiado el namespace
	// a nn::oe2 o similar). Mientras tanto, devolvemos Docked como
	// fallback seguro.
	return EDPPlatformMode::Docked;
#else
	// En PC no hay detección automática: el modo se mantiene como esté.
	return CurrentMode;
#endif
}

void UDPPlatformModeSubsystem::PollPlatformMode()
{
	const EDPPlatformMode DetectedMode = QueryPlatformMode();
	if (DetectedMode != CurrentMode)
	{
		SetPlatformMode(DetectedMode);
	}
}
