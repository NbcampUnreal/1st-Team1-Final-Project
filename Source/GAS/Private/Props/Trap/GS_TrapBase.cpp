#include "Props/Trap/GS_TrapBase.h"
#include "Character/Player/GS_Player.h"
#include "Engine/DamageEvents.h"

AGS_TrapBase::AGS_TrapBase()
{
	PrimaryActorTick.bCanEverTick = false;

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
	DamageBoxComp->OnComponentBeginOverlap.AddDynamic(this, &AGS_TrapBase::OnDamageBoxOverlap);
}


//데미지 박스에 오버랩된 경우 HandleTrapDamage 함수 실행
void AGS_TrapBase::OnDamageBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this)
	{
		AGS_Player* Player = Cast<AGS_Player>(OtherActor);
		if (Player)
		{
			DamageBoxEffect(OtherActor);
			HandleTrapDamage(OtherActor);
		}
	}
}

void AGS_TrapBase::DamageBoxEffect_Implementation(AActor* OtherActor)
{
	UE_LOG(LogTemp, Warning, TEXT("DamageBoxEffect Applied"));
}


void AGS_TrapBase::HandleTrapDamage(AActor* OtherActor)
{
	UE_LOG(LogTemp, Warning, TEXT("Player damaged"));
	if (!OtherActor) return;
	AGS_Player* DamagedPlayer = Cast<AGS_Player>(OtherActor);
	if (!DamagedPlayer) return;

	FTrapData* FoundTrapData = TrapDataTable->FindRow<FTrapData>(TrapID, TEXT("Damage"));
	if (!FoundTrapData) return;

	//기본 데미지 부여
	FDamageEvent DamageEvent;
	DamagedPlayer->TakeDamage(FoundTrapData->Effect.Damage, DamageEvent, nullptr, this);

	//디버프 추가

}

void AGS_TrapBase::HandleTrapAreaDamage(const TArray<AActor*>& AffectedActors)
{
	//for 루프 써서 안에 있는 모든 affectedActors에게 HandleTrapDamage 호출
	//for (AActor* Actor : AffectedActors)
	//{
	//	HandleTrapDamage(Actor);
	//}
}

//후에 플레이어 데미지, 디버프와 연결용
void AGS_TrapBase::LoadTrapData()
{
}