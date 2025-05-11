#include "Props/Trap/TriggerTrap/GS_TrigTrapBase.h"

AGS_TrigTrapBase::AGS_TrigTrapBase()
{
	TriggerBoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBoxComp->SetupAttachment(RotationSceneComp);
	TriggerBoxComp->SetCollisionProfileName(TEXT("Trigger"));
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
		ApplyTrapEffect(OtherActor);
	}
}


void AGS_TrigTrapBase::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	//change it to Player Character later
	if (OtherActor && OtherActor != this)
	{
		EndTrapEffect(OtherActor);
	}
}


void AGS_TrigTrapBase::ApplyTrapEffect_Implementation(AActor* TargetActor)
{
	UE_LOG(LogTemp, Warning, TEXT("TrapEffect Applied"));
}

void AGS_TrigTrapBase::EndTrapEffect_Implementation(AActor* TargetActor)
{
	UE_LOG(LogTemp, Warning, TEXT("TrapEffect Ended"));
}