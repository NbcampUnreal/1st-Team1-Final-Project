#include "Props/Trap/TriggerTrap/GS_TrigTrapBase.h"
#include "Character/Player/GS_Player.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include <Net/UnrealNetwork.h>
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

void AGS_TrigTrapBase::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this && !bIsTriggered)
	{
		AGS_Seeker* Seeker = Cast<AGS_Seeker>(OtherActor);
		if (Seeker)
		{
			//함정 트리거 이후, 동작 전 경고 사운드 함수(BP에서 구현)
			PlayTrapAlertSound(Seeker);

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

void AGS_TrigTrapBase::PlayTrapAlertSound_Implementation(AActor* TargetActor)
{
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



void AGS_TrigTrapBase::EndTrapEffect_Implementation(AActor* TargetActor)
{
	UE_LOG(LogTemp, Warning, TEXT("TrapEffect Ended"));
}


void AGS_TrigTrapBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGS_TrigTrapBase, bIsTriggered);
}