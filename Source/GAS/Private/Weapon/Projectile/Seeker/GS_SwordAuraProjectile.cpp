// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Projectile/Seeker/GS_SwordAuraProjectile.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AkGameplayStatics.h"
#include "Character/GS_Character.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "ResourceSystem/Aether/GS_AetherExtractor.h"

AGS_SwordAuraProjectile::AGS_SwordAuraProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);

	// SlashBox 부착
	SlashBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SlashBoxA"));
	SlashBox->SetupAttachment(CollisionComponent);
	SlashBox->SetBoxExtent(FVector(100.f, 20.f, 100.f));
	SlashBox->SetCollisionProfileName(TEXT("Arrow"));

	ProjectileMovementComponent->InitialSpeed = 4000.0f;
	ProjectileMovementComponent->MaxSpeed = 4000.0f;
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
}

void AGS_SwordAuraProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGS_SwordAuraProjectile, EffectType);
}

void AGS_SwordAuraProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	// 오버랩 이벤트 바인딩
	SlashBox->OnComponentBeginOverlap.AddDynamic(this, &AGS_SwordAuraProjectile::OnSlashBoxOverlap);	

	GetWorld()->GetTimerManager().SetTimer(DestorySwordAuraHandle, this, &AGS_SwordAuraProjectile::DestroySwordAura, SwordAuraLifetime, false);
}

void AGS_SwordAuraProjectile::OnSlashBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 서버에서만 실행
	if (!HasAuthority())
	{
		return;
	}

	// 유효성 및 중복 체크
	if (!OtherActor || OtherActor == this || HitActors.Contains(OtherActor))
	{
		return;
	}

	HitActors.Add(OtherActor);
	
	// 데미지 적용
	UGameplayStatics::ApplyDamage(OtherActor, BaseDamage * 2.0f, GetInstigatorController(), this, nullptr);
	
	// 타격 위치 계산 (히트된 액터의 중심 위치 사용)
	FVector HitLocation = OtherActor->GetActorLocation();
	
	// 타격 대상 타입 판별 (최적화: 한 번만 Cast, 상속 계층 고려)
	ESwordAuraHitTargetType TargetType = ESwordAuraHitTargetType::Other;
	
	// 상속 관계 고려: 구체적인 타입부터 체크 (Guardian, Seeker는 Character를 상속)
	if (AGS_Guardian* Guardian = Cast<AGS_Guardian>(OtherActor))
	{
		TargetType = ESwordAuraHitTargetType::Guardian;
	}
	else if (AGS_Seeker* Seeker = Cast<AGS_Seeker>(OtherActor))
	{
		TargetType = ESwordAuraHitTargetType::Seeker;
	}
	else if (AGS_Character* Character = Cast<AGS_Character>(OtherActor))
	{
		TargetType = ESwordAuraHitTargetType::Character;
	}
	else if (AGS_AetherExtractor* Extractor = Cast<AGS_AetherExtractor>(OtherActor))
	{
		TargetType = ESwordAuraHitTargetType::Structure;
	}
	
	// 타격 VFX 및 사운드 재생 (멀티캐스트)
	Multicast_PlayHitEffects(TargetType, HitLocation);
}

void AGS_SwordAuraProjectile::DestroySwordAura()
{
	Destroy();
}

void AGS_SwordAuraProjectile::StartSwordSlashVFX()
{

}

void AGS_SwordAuraProjectile::StopSwordSlashVFX()
{

}


void AGS_SwordAuraProjectile::Multicast_StartSwordSlashVFX_Implementation()
{
	if (!SlashBox)
	{
		return;
	}

	// VFX 타입 선택
	UNiagaraSystem* SelectedVFX = nullptr;
	switch (EffectType)
	{
	case ESwordAuraEffectType::LeftNormal:
		SelectedVFX = LeftNormalSlashVFX;
		break;
	case ESwordAuraEffectType::RightNormal:
		SelectedVFX = RightNormalSlashVFX;
		break;
	case ESwordAuraEffectType::LeftBuff:
		SelectedVFX = LeftBuffSlashVFX;
		break;
	case ESwordAuraEffectType::RightBuff:
		SelectedVFX = RightBuffSlashVFX;
		break;
	}
	
	if (!SelectedVFX)
	{
		UE_LOG(LogTemp, Warning, TEXT("SwordAuraProjectile: SelectedVFX is null"));
		return;
	}

	// VFX 컴포넌트 생성 및 부착
	FVector LocalPos = FVector::ZeroVector;
	FRotator LocalRot = FRotator::ZeroRotator;

	SlashVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		SelectedVFX,
		SlashBox,
		NAME_None,
		LocalPos,
		LocalRot,
		EAttachLocation::KeepRelativeOffset,
		true
	);
}

void AGS_SwordAuraProjectile::Multicast_PlayHitEffects_Implementation(ESwordAuraHitTargetType TargetType, const FVector& HitLocation)
{
	if (!GetWorld())
	{
		return;
	}

	// 궁극기 활성화 상태 확인
	bool bIsBuffed = (EffectType == ESwordAuraEffectType::LeftBuff || EffectType == ESwordAuraEffectType::RightBuff);

	// =============================
	// VFX 재생
	// =============================
	
	// 타격 이펙트
	UNiagaraSystem* SelectedHitVFX = bIsBuffed ? BuffHitVFX : NormalHitVFX;
	if (SelectedHitVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			SelectedHitVFX,
			HitLocation,
			FRotator::ZeroRotator,
			FVector(1.0f),
			true,
			true,
			ENCPoolMethod::AutoRelease
		);
	}

	// 혈흔 이펙트
	UNiagaraSystem* SelectedBloodVFX = bIsBuffed ? BuffBloodSplatterVFX : NormalBloodSplatterVFX;
	if (SelectedBloodVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			SelectedBloodVFX,
			HitLocation,
			FRotator::ZeroRotator,
			FVector(1.0f),
			true,
			true,
			ENCPoolMethod::AutoRelease
		);
	}

	// =============================
	// 사운드 재생
	// =============================
	
	UAkAudioEvent* SoundEventToPlay = nullptr;

	// 타격 대상 타입에 따라 사운드 선택
	switch (TargetType)
	{
	case ESwordAuraHitTargetType::Guardian:
	case ESwordAuraHitTargetType::Character:
		SoundEventToPlay = HitPawnSoundEvent;
		break;
	case ESwordAuraHitTargetType::Seeker:
		SoundEventToPlay = HitSeekerSoundEvent;
		break;
	case ESwordAuraHitTargetType::Structure:
	case ESwordAuraHitTargetType::Other:
		SoundEventToPlay = HitStructureSoundEvent;
		break;
	default:
		break;
	}

	// Wwise 사운드 이벤트 재생
	if (SoundEventToPlay)
	{
		UAkGameplayStatics::PostEventAtLocation(
			SoundEventToPlay,
			HitLocation,
			FRotator::ZeroRotator,
			GetWorld()
		);
	}
}