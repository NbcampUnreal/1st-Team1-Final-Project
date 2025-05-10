#include "Props/Trap/GS_TrapBase.h"

AGS_TrapBase::AGS_TrapBase()
{
	PrimaryActorTick.bCanEverTick = false;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootScene;

	RotationScene = CreateDefaultSubobject<USceneComponent>(TEXT("RotationScene"));
	RotationScene->SetupAttachment(RootComponent);

	TrapStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TrapStaticMesh"));
	TrapStaticMesh->SetupAttachment(RotationScene);

	DamageBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageBox"));
	DamageBox->SetupAttachment(TrapStaticMesh);
	//needs to be fixed
	// DamageBox 콜리전 설정
	DamageBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	DamageBox->SetCollisionObjectType(ECC_Pawn); // Object Type을 Pawn으로 설정

	// 기본 응답: 모두 무시
	DamageBox->SetCollisionResponseToAllChannels(ECR_Ignore);

	// 트레이스 채널
	DamageBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Overlap);
	DamageBox->SetCollisionResponseToChannel(ECC_Camera, ECR_Overlap);

	// 오브젝트 채널
	DamageBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	DamageBox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	DamageBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	DamageBox->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);
	DamageBox->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Block);
	DamageBox->SetCollisionResponseToChannel(ECC_Destructible, ECR_Overlap);
}


void AGS_TrapBase::BeginPlay()
{
	Super::BeginPlay();
	DamageBox->OnComponentBeginOverlap.AddDynamic(this, &AGS_TrapBase::OnDamageBoxOverlap);
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


//후에 플레이어 데미지, 디버프와 연결용
void AGS_TrapBase::LoadTrapData()
{
}