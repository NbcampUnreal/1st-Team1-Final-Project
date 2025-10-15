#include "Props/Interactables/GS_ToxicWater.h"
#include "Character/Player/Seeker/GS_Seeker.h"


AGS_ToxicWater::AGS_ToxicWater()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.bAllowTickOnDedicatedServer = false;

	RootSceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComp"));
	RootComponent = RootSceneComp;

	DamageDebuffBoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageDebuffBoxComp"));
	DamageDebuffBoxComp->SetupAttachment(RootSceneComp);

	ToxicWaterMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ToxicWaterMeshComp"));
	ToxicWaterMeshComp->SetupAttachment(RootSceneComp);
}

void AGS_ToxicWater::BeginPlay()
{
	Super::BeginPlay();
	
	DamageDebuffBoxComp->OnComponentBeginOverlap.AddDynamic(this, &AGS_ToxicWater::OnTWaterBeginOverlap);
	DamageDebuffBoxComp->OnComponentEndOverlap.AddDynamic(this, &AGS_ToxicWater::OnTWaterEndOverlap);
}

void AGS_ToxicWater::OnTWaterBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor)
	{
		return;
	}
	AGS_Seeker* Seeker = Cast<AGS_Seeker>(OtherActor);
	if (Seeker)
	{
		if (UGS_DebuffComp* DebuffComp = Seeker->FindComponentByClass<UGS_DebuffComp>())
		{
			DebuffComp->ApplyDebuff(EDebuffType::Slow, nullptr);
			DebuffComp->ApplyDebuff(EDebuffType::Lava, nullptr);
		}
	}
}

void AGS_ToxicWater::OnTWaterEndOverlap(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	AGS_Seeker* Seeker = Cast<AGS_Seeker>(OtherActor);
	if (Seeker)
	{
		if (UGS_DebuffComp* DebuffComp = Seeker->FindComponentByClass<UGS_DebuffComp>())
		{
			DebuffComp->RemoveDebuff(EDebuffType::Slow);
			//임시
			DebuffComp->RemoveDebuff(EDebuffType::Lava);
		}
	}
	
}
