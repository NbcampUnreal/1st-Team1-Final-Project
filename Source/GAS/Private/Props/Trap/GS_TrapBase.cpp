#include "Props/Trap/GS_TrapBase.h"

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
	//needs to be fixed
	// DamageBox 콜리전 설정
	DamageBoxComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	DamageBoxComp->SetCollisionObjectType(ECC_Pawn); // Object Type을 Pawn으로 설정

	// 기본 응답: 모두 무시
	DamageBoxComp->SetCollisionResponseToAllChannels(ECR_Ignore);

	// 트레이스 채널
	DamageBoxComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Overlap);
	DamageBoxComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Overlap);

	// 오브젝트 채널
	DamageBoxComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	DamageBoxComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	DamageBoxComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	DamageBoxComp->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);
	DamageBoxComp->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Block);
	DamageBoxComp->SetCollisionResponseToChannel(ECC_Destructible, ECR_Overlap);
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
		UE_LOG(LogTemp, Warning, TEXT("DamageBox 2"));
		DamageBoxEffect(OtherActor);
		HandleTrapDamage(OtherActor);
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