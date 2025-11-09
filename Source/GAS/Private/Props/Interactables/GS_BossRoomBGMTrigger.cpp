#include "Props/Interactables/GS_BossRoomBGMTrigger.h"
#include "Character/GS_Character.h"
#include "Sound/GS_AudioManager.h"
#include "Engine/GameInstance.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"


AGS_BossRoomBGMTrigger::AGS_BossRoomBGMTrigger()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	RootSceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootSceneComp;
	RootSceneComp->SetMobility(EComponentMobility::Movable);

	TriggerBoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBoxComp->SetupAttachment(RootComponent);
	TriggerBoxComp->SetBoxExtent(FVector(100.f, 100.f, 100.f));
	TriggerBoxComp->SetCollisionProfileName(TEXT("Trigger"));
	TriggerBoxComp->SetMobility(EComponentMobility::Movable);
}

void AGS_BossRoomBGMTrigger::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerBoxComp)
	{
		TriggerBoxComp->OnComponentBeginOverlap.AddDynamic(this, &AGS_BossRoomBGMTrigger::OnTriggerBeginOverlap);
		TriggerBoxComp->OnComponentEndOverlap.AddDynamic(this, &AGS_BossRoomBGMTrigger::OnTriggerEndOverlap);
	}
}

void AGS_BossRoomBGMTrigger::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 델리게이트 언바인딩으로 레벨 전환 시 안전성 보장
	if (TriggerBoxComp)
	{
		TriggerBoxComp->OnComponentBeginOverlap.RemoveAll(this);
		TriggerBoxComp->OnComponentEndOverlap.RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
}

void AGS_BossRoomBGMTrigger::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	// 시커만 트리거 가능
	AGS_Seeker* Seeker = Cast<AGS_Seeker>(OtherActor);
	if (!Seeker)
	{
		return;
	}

	// 로컬 플레이어가 조종하는 시커인지 확인
	if (!Seeker->IsLocallyControlled())
	{
		return;
	}
	
	// 로컬 플레이어만 BGM 변경
	TriggerBossRoomBGMForLocalPlayer(Seeker, BossMusicStartEvent, BossMusicStopEvent);
}

void AGS_BossRoomBGMTrigger::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	// 시커만 처리
	AGS_Seeker* Seeker = Cast<AGS_Seeker>(OtherActor);
	if (!Seeker)
	{
		return;
	}

	// 로컬 플레이어가 조종하는 시커인지 확인
	if (!Seeker->IsLocallyControlled())
	{
		return;
	}
	
	// 로컬 플레이어만 BGM 종료
	EndBossRoomBGMForLocalPlayer(Seeker);
}

void AGS_BossRoomBGMTrigger::TriggerBossRoomBGMForLocalPlayer(AActor* TargetActor, UAkAudioEvent* StartEvent, UAkAudioEvent* StopEvent)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UGS_AudioManager* AudioManager = GameInstance->GetSubsystem<UGS_AudioManager>())
		{
			// 로컬 전용 함수 사용 (멀티캐스트 불필요)
			AudioManager->StartBossSequenceLocal(TargetActor, StartEvent, StopEvent);
		}
	}
}

void AGS_BossRoomBGMTrigger::EndBossRoomBGMForLocalPlayer(AActor* TargetActor)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UGS_AudioManager* AudioManager = GameInstance->GetSubsystem<UGS_AudioManager>())
		{
			// 로컬 전용 함수 사용 (멀티캐스트 불필요)
			AudioManager->EndBossSequenceLocal(TargetActor, 2.0f);
		}
	}
}