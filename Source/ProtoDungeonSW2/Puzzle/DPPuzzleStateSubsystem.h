#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DPPuzzleStateSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBallReachedGoalSignature);

UCLASS()
class PROTODUNGEONSW2_API UDPPuzzleStateSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Puzzle")
	bool bBallReachedGoal = false;

	UPROPERTY(BlueprintReadOnly, Category = "Puzzle")
	bool bPlayerHasKey = false;

	UPROPERTY(BlueprintAssignable, Category = "Puzzle")
	FOnBallReachedGoalSignature OnBallReachedGoal;

	// Called by HoleTrigger when the ball falls in.
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void NotifyBallReachedGoal();

	// Called by the player when picking up the key.
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void SetPlayerHasKey(bool bHasKey);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
};
