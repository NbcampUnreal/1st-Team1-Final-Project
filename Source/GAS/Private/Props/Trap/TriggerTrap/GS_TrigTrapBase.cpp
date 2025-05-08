#include "Props/Trap/TriggerTrap/GS_TrigTrapBase.h"

AGS_TrigTrapBase::AGS_TrigTrapBase()
{
	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootComponent);
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
}

void AGS_TrigTrapBase::BeginPlay()
{
	Super::BeginPlay();
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AGS_TrigTrapBase::OnTriggerOverlap);
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