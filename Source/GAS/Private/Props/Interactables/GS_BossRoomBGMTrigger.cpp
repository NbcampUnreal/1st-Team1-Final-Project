#include "Props/Interactables/GS_BossRoomBGMTrigger.h"
#include "Character/GS_Character.h"
#include "Sound/GS_AudioManager.h"
#include "Engine/GameInstance.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"
#include "AI/RTS/GS_RTSController.h"
#include "EngineUtils.h"


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

	// 서버에서만 RTS 컨트롤러 캐싱
	if (HasAuthority())
	{
		CacheRTSController();
	}
}

void AGS_BossRoomBGMTrigger::CacheRTSController()
{
	for (TActorIterator<AGS_RTSController> It(GetWorld()); It; ++It)
	{
		CachedRTSController = *It;
		break;
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

	// 1. 로컬 플레이어(시커) 로직
	if (Seeker->IsLocallyControlled())
	{
		// 로컬 플레이어만 BGM 변경
		TriggerBossRoomBGMForLocalPlayer(Seeker, BossMusicStartEvent, BossMusicStopEvent);
	}

	// 2. 가디언(RTS) 플레이어 로직 (서버 권한 필요)
	if (HasAuthority())
	{
		TArray<AActor*> OverlappingActors;
		TriggerBoxComp->GetOverlappingActors(OverlappingActors, AGS_Seeker::StaticClass());

		// 첫 번째 시커가 들어왔을 때만 RTS 플레이어에게 BGM 재생 요청
		if (OverlappingActors.Num() == 1)
		{
			if (AGS_RTSController* RTSController = CachedRTSController.Get())
			{
				RTSController->Client_PlayBossBGM(BossMusicStartEvent, BossMusicStopEvent);
			}
		}
	}
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

	// 1. 로컬 플레이어(시커) 로직
	if (Seeker->IsLocallyControlled())
	{
		// 로컬 플레이어만 BGM 종료
		EndBossRoomBGMForLocalPlayer(Seeker);
	}

	// 2. 가디언(RTS) 플레이어 로직 (서버 권한 필요)
	if (HasAuthority())
	{
		TArray<AActor*> OverlappingActors;
		TriggerBoxComp->GetOverlappingActors(OverlappingActors, AGS_Seeker::StaticClass());

		// 마지막 시커가 나갔을 때만 RTS 플레이어에게 BGM 종료 요청
		if (OverlappingActors.Num() == 0)
		{
			if (AGS_RTSController* RTSController = CachedRTSController.Get())
			{
				RTSController->Client_StopBossBGM();
			}
		}
	}
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
