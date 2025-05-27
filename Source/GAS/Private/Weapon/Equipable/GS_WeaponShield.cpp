// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/Equipable/GS_WeaponShield.h"
#include "Character/GS_Character.h"

AGS_WeaponShield::AGS_WeaponShield()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;

	// 방패 메시 생성
	ShieldMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ShieldMesh"));
	RootComponent = ShieldMesh;

	// 방어 콜리전 박스 생성
	BlockBox = CreateDefaultSubobject<UBoxComponent>(TEXT("BlockBox"));
	BlockBox->SetupAttachment(ShieldMesh);
	BlockBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BlockBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	BlockBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	BlockBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Block);
	BlockBox->OnComponentBeginOverlap.AddDynamic(this, &AGS_WeaponShield::OnBlock);

	// 방패 메시 에셋 로드
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(
		TEXT("/Game/Weapons/Shield_02/SKM_Shield_02_L.SKM_Shield_02_L"));
	if (MeshAsset.Succeeded())
	{
		ShieldMesh->SetSkeletalMesh(MeshAsset.Object);
	}
}

void AGS_WeaponShield::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AGS_WeaponShield::BeginPlay()
{
	Super::BeginPlay();
	SetOwningCharacter(Cast<AGS_Character>(GetOwner()));
}

void AGS_WeaponShield::EnableBlock()
{
	if (HasAuthority())
	{
		SetBlockCollision(true);
	}
	else
	{
		Server_SetBlockCollision(true);
	}
}

void AGS_WeaponShield::DisableBlock()
{
	if (HasAuthority())
	{
		SetBlockCollision(false);
	}
	else
	{
		Server_SetBlockCollision(false);
	}
}

void AGS_WeaponShield::Server_SetBlockCollision_Implementation(bool bEnable)
{
	SetBlockCollision(bEnable);
}

void AGS_WeaponShield::SetBlockCollision(bool bEnable)
{
	if (BlockBox)
	{
		BlockBox->SetCollisionEnabled(bEnable ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	}
}

void AGS_WeaponShield::OnBlock(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 기본 필터링
	if (!OtherActor || OtherActor == this || OtherActor == OwnerCharacter)
	{
		return;
	}

	// 방어 처리 로직 (데미지 감소, 넉백 등)
	// 여기서 실제 방어 로직을 구현

	// 방어 사운드 재생
	PlayBlockSound();

	// 로그 출력 (디버깅용)
	UE_LOG(LogTemp, Log, TEXT("%s blocked attack from %s"), *GetName(), *OtherActor->GetName());
}

void AGS_WeaponShield::PlayBlockSound()
{
	if (BlockSound && AkComponent)
	{
		AkComponent->PostAkEvent(BlockSound);
	}
}

void AGS_WeaponShield::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}