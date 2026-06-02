#include "DPMessageSubsystem.h"

void UDPMessageSubsystem::RequestMessage(const FString& Message, float Duration)
{
	OnMessageRequested.Broadcast(Message, Duration);
}
