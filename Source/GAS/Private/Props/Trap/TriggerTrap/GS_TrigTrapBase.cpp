#include "Props/Trap/TriggerTrap/GS_TrigTrapBase.h"
#include "Character/Player/GS_Player.h"
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
	TriggerBoxComp->OnComponentBeginOverlap.AddDynamic(this, &AGS_TrigTrapBase::OnTriggerOverlap);
	TriggerBoxComp->OnComponentEndOverlap.AddDynamic(this, &AGS_TrigTrapBase::OnTriggerEndOverlap);

}

void AGS_TrigTrapBase::OnTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	//change it to Player Character later
	if (OtherActor && OtherActor != this)
	{
		AGS_Player* Player = Cast<AGS_Player>(OtherActor);
		if (Player)
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
	Multicast_PlayTrapEffect(TargetActor);
}

void AGS_TrigTrapBase::Multicast_PlayTrapEffect_Implementation(AActor* TargetActor)
{
	ApplyTrapEffect(TargetActor);
}

void AGS_TrigTrapBase::ApplyTrapEffect_Implementation(AActor* TargetActor)
{
	UE_LOG(LogTemp, Warning, TEXT("TrapEffect Applied"));
}





void AGS_TrigTrapBase::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	//change it to Player Character later
	if (OtherActor && OtherActor != this)
	{
		AGS_Player* Player = Cast<AGS_Player>(OtherActor);
		if (Player)
		{
			if(!HasAuthority())
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