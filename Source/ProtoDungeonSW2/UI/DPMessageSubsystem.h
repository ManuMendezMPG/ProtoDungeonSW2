#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DPMessageSubsystem.generated.h"

// Delegate disparado cuando algún sistema solicita mostrar un mensaje en pantalla.
// Los widgets de mensaje se suscriben para reaccionar.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMessageRequested, const FString&, Message, float, Duration);

UCLASS()
class PROTODUNGEONSW2_API UDPMessageSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Se dispara cuando se solicita mostrar un mensaje. Los widgets de UI se enganchan aquí.
	UPROPERTY(BlueprintAssignable, Category = "Message")
	FOnMessageRequested OnMessageRequested;

	// Solicita mostrar un mensaje durante Duration segundos.
	// Simplemente retransmite la solicitud vía OnMessageRequested; no almacena estado.
	UFUNCTION(BlueprintCallable, Category = "Message")
	void RequestMessage(const FString& Message, float Duration = 3.0f);
};
