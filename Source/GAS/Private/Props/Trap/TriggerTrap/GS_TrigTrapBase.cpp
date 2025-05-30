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
	TriggerBoxComp->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	TriggerBoxComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBoxComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);


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
	//change it to Player Character later
	if (OtherActor && OtherActor != this && !bIsTriggered)
	{
		AGS_Seeker* Seeker = Cast<AGS_Seeker>(OtherActor);
		if (Seeker)
		{
			if (!HasAuthority())
			{
				//클라이언트
				Server_ApplyTrapEffect(OtherActor);
			}
			else
			{
				//서버
				ApplyTrapEffect(OtherActor);
			}
		}
		
	}
}

void AGS_TrigTrapBase::Server_ApplyTrapEffect_Implementation(AActor* TargetActor)
{
	ApplyTrapEffect(TargetActor);
}

void AGS_TrigTrapBase::ApplyTrapEffect_Implementation(AActor* TargetActor)
{
	UE_LOG(LogTemp, Warning, TEXT("TrapEffect Applied"));
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
			Server_ApplyTrapEffect(Actor);
			return;
		}
	}
	bIsTriggered = false;
}



void AGS_TrigTrapBase::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	//change it to Player Character later
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