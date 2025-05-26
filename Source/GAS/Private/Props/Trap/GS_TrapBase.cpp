#include "Props/Trap/GS_TrapBase.h"
#include "Character/Player/GS_Player.h"
#include "Engine/DamageEvents.h"

AGS_TrapBase::AGS_TrapBase()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);

	RootSceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComp"));
	RootComponent = RootSceneComp;

	RotationSceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("RotationScene"));
	RotationSceneComp->SetupAttachment(RootComponent);

	MeshParentSceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("MeshParentSceneComp"));
	MeshParentSceneComp->SetupAttachment(RotationSceneComp);

	DamageBoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageBox"));
	DamageBoxComp->SetupAttachment(MeshParentSceneComp);
	// DamageBox 콜리전 설정
	DamageBoxComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	DamageBoxComp->SetCollisionObjectType(ECC_WorldDynamic);
	DamageBoxComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	DamageBoxComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}


void AGS_TrapBase::BeginPlay()
{
	Super::BeginPlay();

	LoadTrapData();
	DamageBoxComp->OnComponentBeginOverlap.AddDynamic(this, &AGS_TrapBase::OnDamageBoxOverlap);
}

void AGS_TrapBase::LoadTrapData()
{
	if (!TrapDataTable) return;
	FTrapData* FoundTrapData = TrapDataTable->FindRow<FTrapData>(TrapID, TEXT("LoadTrapData"));
	if (FoundTrapData)
	{
		TrapData = *FoundTrapData;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TrapData not found for TrapID : %s"), *TrapID.ToString());
	}
}

//데미지 박스에 오버랩된 경우 HandleTrapDamage 함수 실행
void AGS_TrapBase::OnDamageBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	AGS_Player* Player = Cast<AGS_Player>(OtherActor);
	if (!Player)
	{
		return;
	}
	if (!HasAuthority())
	{
		//클라이언트
		Server_DamageBoxEffect(Player);
		Server_CustomTrapEffect(Player);
		Server_HandleTrapDamage(Player);
	}
	else
	{
		//서버
		DamageBoxEffect(Player);
		CustomTrapEffect(Player);
		HandleTrapDamage(Player);
		
	}
}

void AGS_TrapBase::Server_HandleTrapDamage_Implementation(AActor* OtherActor)
{
	HandleTrapDamage(OtherActor);
}


void AGS_TrapBase::HandleTrapDamage(AActor* OtherActor)
{
	UE_LOG(LogTemp, Warning, TEXT("Player damaged"));
	if (!OtherActor) return;
	AGS_Player* DamagedPlayer = Cast<AGS_Player>(OtherActor);
	if (!DamagedPlayer) return;
	if (TrapData.Effect.Damage <= 0.f) return;

 	//기본 데미지 부여
	FDamageEvent DamageEvent;
	DamagedPlayer->TakeDamage(TrapData.Effect.Damage, DamageEvent, nullptr, this);

	//디버프 추가

}

void AGS_TrapBase::HandleTrapAreaDamage(const TArray<AActor*>& AffectedActors)
{
}


void AGS_TrapBase::Server_DamageBoxEffect_Implementation(AActor* OtherActor)
{
	UE_LOG(LogTemp, Warning, TEXT("Server_DamageBoxEffect_Implementation called"));
	//Multicast_DamageBoxEffect(OtherActor);
	DamageBoxEffect(OtherActor);
}


void AGS_TrapBase::Multicast_DamageBoxEffect_Implementation(AActor* TargetActor)
{
	UE_LOG(LogTemp, Warning, TEXT("Multicast_DamageBoxEffect_Implementation called"));
	DamageBoxEffect(TargetActor);
}

void AGS_TrapBase::DamageBoxEffect_Implementation(AActor* OtherActor)
{
	UE_LOG(LogTemp, Warning, TEXT("DamageBoxEffect Applied"));
}


void AGS_TrapBase::Server_CustomTrapEffect_Implementation(AActor* TargetActor)
{
	CustomTrapEffect(TargetActor);
}

void AGS_TrapBase::CustomTrapEffect_Implementation(AActor* TargetActor)
{

}