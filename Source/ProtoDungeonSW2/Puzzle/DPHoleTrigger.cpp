#include "DPHoleTrigger.h"
#include "../Characters/DPPuzzleBall.h"
#include "../Input/DPPlatformModeSubsystem.h"
#include "DPPuzzleStateSubsystem.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

ADPHoleTrigger::ADPHoleTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	// Trigger box como root: solo overlap, no bloquea nada
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
	// Solo nos interesa la bola del puzzle
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

	// 1) Notificar al subsystem de estado del puzzle
	if (UDPPuzzleStateSubsystem* PuzzleState = GameInst->GetSubsystem<UDPPuzzleStateSubsystem>())
	{
		PuzzleState->NotifyBallReachedGoal();
	}

	// 2) Cambiar el modo a Docked ANTES de destruir la bola: el PlayerController necesita
	//    que ambos pawns sigan vivos para poder hacer Possess(PlayerCharacterPawn).
	if (UDPPlatformModeSubsystem* PlatformMode = GameInst->GetSubsystem<UDPPlatformModeSubsystem>())
	{
		PlatformMode->SetPlatformMode(EDPPlatformMode::Docked);
	}

	// 3) Destruir la bola — ya hemos hecho el switch al player character
	Ball->Destroy();
}
