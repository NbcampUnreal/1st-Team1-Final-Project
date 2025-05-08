#include "Props/Trap/GS_TrapBase.h"

AGS_TrapBase::AGS_TrapBase()
{
	PrimaryActorTick.bCanEverTick = false;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootScene;

	TrapStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TrapStaticMesh"));
	TrapStaticMesh->SetupAttachment(RootComponent);

	DamageBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageBox"));
	DamageBox->SetupAttachment(RootComponent);
	//needs to be fixed
	DamageBox->SetCollisionProfileName(TEXT("OverlapAll"));
}

//load static mesh on Construnction not begin play
void AGS_TrapBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (!TrapDataTable || TrapID.IsNone()) return;
	FTrapData* FoundData = TrapDataTable->FindRow<FTrapData>(TrapID, TEXT("Trap Lookup"));

	if (FoundData)
	{
		TrapData = *FoundData;
		if (TrapData.TrapMesh.IsValid())
		{
			TrapStaticMesh->SetStaticMesh(TrapData.TrapMesh.LoadSynchronous());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Trap with ID '%s' not found in DataTable"), *TrapID.ToString());
	}

}

void AGS_TrapBase::BeginPlay()
{
	Super::BeginPlay();
	DamageBox->OnComponentBeginOverlap.AddDynamic(this, &AGS_TrapBase::OnDamageBoxOverlap);
}

void AGS_TrapBase::LoadTrapData()
{
}

void AGS_TrapBase::ApplyTrapEffect(AActor* TargetActor)
{
	UE_LOG(LogTemp, Warning, TEXT("TrapEffect Applied"));
}


void AGS_TrapBase::OnDamageBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this)
	{
		HandleTrapDamage(OtherActor);
	}
}

void AGS_TrapBase::HandleTrapDamage(AActor* OtherActor)
{
	UE_LOG(LogTemp, Warning, TEXT("damage Box overlapped"));
}
