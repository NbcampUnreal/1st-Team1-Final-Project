#include "Props/Trap/GS_TrapBase.h"
#include "Character/Player/GS_Player.h"


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
	DamageBoxComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
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
	UE_LOG(LogTemp, Warning, TEXT("DamageBox 1"));
	if (OtherActor && OtherActor != this)
	{
		AGS_Player* Player = Cast<AGS_Player>(OtherActor);
		if (Player)
		{
			UE_LOG(LogTemp, Warning, TEXT("DamageBox 2"));
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
}

void AGS_TrapBase::HandleTrapAreaDamage(const TArray<AActor*>& AffectedActors)
{

}

//후에 플레이어 데미지, 디버프와 연결용
void AGS_TrapBase::LoadTrapData()
{
}