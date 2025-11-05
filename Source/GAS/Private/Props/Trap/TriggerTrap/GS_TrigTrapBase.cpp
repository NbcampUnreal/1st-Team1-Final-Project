#include "Props/Trap/TriggerTrap/GS_TrigTrapBase.h"
#include "Character/Player/GS_Player.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include <Net/UnrealNetwork.h>
#include "AkGameplayStatics.h"
#include "AkGameplayTypes.h"
#include "AkAudioDevice.h"
#include "AkComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AI/RTS/GS_RTSController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
AGS_TrigTrapBase::AGS_TrigTrapBase()
{
	TriggerBoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBoxComp->SetupAttachment(RotationSceneComp);
	
	//Trigger Box 설정
	TriggerBoxComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	//ECC_GameTraceChannel4 : Trap
	TriggerBoxComp->SetCollisionObjectType(ECC_GameTraceChannel4);
	TriggerBoxComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBoxComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	//"OptimizedCollision" 태그가 있는 경우, 플레이어가 근접한 경우에만 콜리전 활성화됨
	TriggerBoxComp->ComponentTags.Add("OptimizedCollision");
}

void AGS_TrigTrapBase::BeginPlay()
{
	Super::BeginPlay();
	TriggerBoxComp->OnComponentBeginOverlap.AddDynamic(this, &AGS_TrigTrapBase::OnTriggerBeginOverlap);
	TriggerBoxComp->OnComponentEndOverlap.AddDynamic(this, &AGS_TrigTrapBase::OnTriggerEndOverlap);

}

void AGS_TrigTrapBase::ActivateTrap_Implementation(AActor* TargetActor)
{
	Super::ActivateTrap_Implementation(TargetActor);
}

void AGS_TrigTrapBase::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this)
	{
		AGS_Seeker* Seeker = Cast<AGS_Seeker>(OtherActor);
		if (Seeker)
		{
		//함정 트리거 이후, 동작 전 경고 사운드 함수(BP에서 구현)
		// 재발동 시에도 사운드가 들리도록 조건 밖에 호출
		CallTrapAlertSound(Seeker);

			if (!bIsTriggered)
			{
				if (!HasAuthority())
				{
					//클라이언트
					Server_DelayTrapEffect(Seeker);
				}
				else
				{
					DelayTrapEffect(Seeker);
				}
			}
		}

	}
}

void AGS_TrigTrapBase::CallTrapAlertSound(AActor* TargetActor)
{
	if (!HasAuthority())
	{
		return;
	}

	UAkAudioEvent* SoundEvent = SelectSoundEventByMode(TrapData.AlertSound_TPS, TrapData.AlertSound_RTS);
	if (SoundEvent)
	{
		Multicast_PlayTrapAlertSound(TargetActor);
	}

	// 블루프린트에서 추가 로직을 실행할 수 있도록 이벤트 호출
	OnTrapAlertSoundPlayed(TargetActor);
}

void AGS_TrigTrapBase::Multicast_PlayTrapAlertSound_Implementation(AActor* TargetActor)
{
	// 데디케이티드 서버에서는 오디오 처리 불필요
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	// Actor 유효성 체크 (서버 안정성)
	if (!IsValid(this))
	{
		return;
	}

	// 거리 기반 최적화 체크
	if (!ShouldPlayTrapSoundAtLocation(GetActorLocation()))
	{
		return;
	}

	UAkAudioEvent* SoundEvent = SelectSoundEventByMode(TrapData.AlertSound_TPS, TrapData.AlertSound_RTS);
	if (SoundEvent)
	{
		// TrapAkComponent가 있으면 AudioAnchor 위치에서 재생, 없으면 Actor 자체 사용
		if (IsValid(TrapAkComponent))
		{
			TrapAkComponent->PostAkEvent(SoundEvent, 0, FOnAkPostEventCallback());
		}
		else
		{
			UAkGameplayStatics::PostEvent(SoundEvent, this, 0, FOnAkPostEventCallback());
		}
	}
}


void AGS_TrigTrapBase::Server_DelayTrapEffect_Implementation(AActor* TargetActor)
{
	DelayTrapEffect(TargetActor);
}


void AGS_TrigTrapBase::ApplyTrapEffect_Implementation(AActor* TargetActor)
{
	UE_LOG(LogTemp, Warning, TEXT("ApplyTrapEffect_Implementation Applied"));
}

void AGS_TrigTrapBase::DelayTrapEffect(AActor* TargetActor)
{
	//Trigger Delay가 0보다 큰 경우, TriggerDelay초 후 함정 발동
	//(사운드 또는 위젯으로 함정 발동 예정임을 알리는 위치)
	if (TriggerDelay > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			DelayHandle,
			FTimerDelegate::CreateUObject(this, &AGS_TrigTrapBase::ApplyTrapEffect, TargetActor),
			TriggerDelay,
			false
		);
	}
	else
	{
		ApplyTrapEffect(TargetActor);
	}
}

//만약 함정의 동작이 끝났는데 플레이어가 남아 있다면 함정 동작 다시 실행
void AGS_TrigTrapBase::TrapEffectComplete()
{
	TArray<AActor*> OverlappingActors;
	TriggerBoxComp->GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		if (IsValid(Actor) && Actor->IsA<AGS_Player>())
		{
			Server_DelayTrapEffect(Actor);
			return;
		}
	}
	bIsTriggered = false;
}



void AGS_TrigTrapBase::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor != this && !bIsTriggered)
	{
		AGS_Seeker* Seeker = Cast<AGS_Seeker>(OtherActor);
		if (Seeker)
		{
			if (!HasAuthority())
			{
				Server_EndTrapEffect(OtherActor);
			}
			else
			{
				EndTrapEffect(OtherActor);
			}

		}
	}
}


void AGS_TrigTrapBase::Server_EndTrapEffect_Implementation(AActor* TargetActor)
{
	Multicast_EndTrapEffect(TargetActor);
}

void AGS_TrigTrapBase::Multicast_EndTrapEffect_Implementation(AActor* TargetActor)
{
	EndTrapEffect(TargetActor);
}

void AGS_TrigTrapBase::DeActivateTrap_Implementation()
{
	Super::DeActivateTrap_Implementation();
}

void AGS_TrigTrapBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 타이머 정리 (레벨 전환 안전성)
	if (UWorld* World = GetWorld())
	{
		if (DelayHandle.IsValid())
		{
			World->GetTimerManager().ClearTimer(DelayHandle);
			DelayHandle.Invalidate();
		}
	}

	Super::EndPlay(EndPlayReason);
}

UAkComponent* AGS_TrigTrapBase::GetOrCreateTrapAkComponent()
{
	if (IsValid(TrapAkComponent))
	{
		TrapAkComponent->OcclusionRefreshInterval = 0.0f;
		return TrapAkComponent;
	}

	TrapAkComponent = FindComponentByClass<UAkComponent>();
	if (IsValid(TrapAkComponent))
	{
		TrapAkComponent->OcclusionRefreshInterval = 0.0f;
		return TrapAkComponent;
	}

	UWorld* World = GetWorld();
	if (!World || World->bIsTearingDown)
	{
		return nullptr;
	}

    TrapAkComponent = NewObject<UAkComponent>(this, UAkComponent::StaticClass(), NAME_None, RF_Transient);
	if (!IsValid(TrapAkComponent))
	{
		return nullptr;
	}

    USceneComponent* AttachTarget = GetRootComponent();
    TrapAkComponent->AttachToComponent(AttachTarget ? AttachTarget : GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	TrapAkComponent->RegisterComponent();
	TrapAkComponent->SetAutoActivate(false);
	TrapAkComponent->SetStopWhenOwnerDestroyed(true);
	
	TrapAkComponent->OcclusionRefreshInterval = 0.0f;

	return TrapAkComponent;
}



void AGS_TrigTrapBase::EndTrapEffect_Implementation(AActor* TargetActor)
{
	UE_LOG(LogTemp, Warning, TEXT("TrapEffect Ended"));
}


void AGS_TrigTrapBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGS_TrigTrapBase, bIsTriggered);
}