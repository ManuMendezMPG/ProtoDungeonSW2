#include "DPHoleTrigger.h"
#include "../Characters/DPPuzzleBall.h"
#include "../Input/DPPlatformModeSubsystem.h"
#include "DPPuzzleStateSubsystem.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

ADPHoleTrigger::ADPHoleTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	// Trigger box as root: overlap only, blocks nothing
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetBoxExtent(FVector(100.f, 100.f, 100.f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);
	RootComponent = TriggerBox;
}

void ADPHoleTrigger::BeginPlay()
{
	Super::BeginPlay();

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ADPHoleTrigger::OnTriggerBeginOverlap);
}

void ADPHoleTrigger::OnTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// We only care about the puzzle ball
	ADPPuzzleBall* Ball = Cast<ADPPuzzleBall>(OtherActor);
	if (Ball == nullptr)
	{
		return;
	}

	UGameInstance* GameInst = GetGameInstance();
	if (GameInst == nullptr)
	{
		return;
	}

	// 1) Notify the puzzle state subsystem
	if (UDPPuzzleStateSubsystem* PuzzleState = GameInst->GetSubsystem<UDPPuzzleStateSubsystem>())
	{
		PuzzleState->NotifyBallReachedGoal();
	}

	// 2) Switch to Docked mode BEFORE destroying the ball: the PlayerController needs
	//    both pawns alive to be able to Possess(PlayerCharacterPawn).
	if (UDPPlatformModeSubsystem* PlatformMode = GameInst->GetSubsystem<UDPPlatformModeSubsystem>())
	{
		PlatformMode->SetPlatformMode(EDPPlatformMode::Docked);
	}

	// 3) Destroy the ball — we have already switched to the player character
	Ball->Destroy();
}
