#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DPMessageSubsystem.generated.h"

// Delegate fired when some system requests an on-screen message.
// Message widgets subscribe to react.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMessageRequested, const FString&, Message, float, Duration);

UCLASS()
class PROTODUNGEONSW2_API UDPMessageSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Fires when a message is requested for display. UI widgets hook in here.
	UPROPERTY(BlueprintAssignable, Category = "Message")
	FOnMessageRequested OnMessageRequested;

	// Requests an on-screen message for Duration seconds.
	// Simply rebroadcasts the request via OnMessageRequested; stores no state.
	UFUNCTION(BlueprintCallable, Category = "Message")
	void RequestMessage(const FString& Message, float Duration = 3.0f);
};
