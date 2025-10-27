// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Skill/Seeker/GS_HealSkill.h"
#include "Character/Player/GS_Player.h"
#include "Character/Component/GS_StatComp.h"
#include "Sound/GS_SeekerAudioComponent.h"
#include "Net/UnrealNetwork.h"

UGS_HealSkill::UGS_HealSkill()
{
	HealAmount = 200.0f; // 기본 치유량 설정
	MaxHealCount = 5; // 기본 포션 개수
	CurrentHealCount = MaxHealCount; // 시작 시 최대 개수로 설정
	bIsPotionDepletedOrHealthFull = false; 
}

/*
void UGS_HealSkill::InitializeDamageBinding()
{
	// 한 번만 바인딩하도록 체크
	static bool bIsAlreadyBound = false;
	
	if (!bIsAlreadyBound && OwnerCharacter)
	{
		OwnerCharacter->OnTakeAnyDamage.AddDynamic(this, &UGS_HealSkill::OnOwnerDamaged);
		bIsAlreadyBound = true;
	}
}
*/

void UGS_HealSkill::ActiveSkill()
{
	Super::ActiveSkill();

	/*// 피해 감지 바인딩 초기화 (한 번만 실행됨)
	InitializeDamageBinding();*/

	// 서버 권한 확인
	if (!OwnerCharacter || !OwnerCharacter->HasAuthority())
	{
		return;
	}

	if (!CanActivateHealSkill())
	{
		ShowPotionDepletedEffect();
		return;
	}

	if (OwnerCharacter)
	{
		// 여기가 아니라 직접 입에 갖다 대는 애니메이션이 시작되면 그때 AnimNotify 로 호출. -> 여기에서는 AnimMontage 를 호출.
		OwnerCharacter->Multicast_PlaySkillMontage(SkillAnimMontages[0]);
		
		// 체력 회복 (서버 권한)
		UGS_StatComp* StatComp = OwnerCharacter->GetStatComp();
		if (StatComp)
		{
			StatComp->ServerRPCHeal(HealAmount);
		}

		// VFX 재생 (모든 클라이언트에 동기화)
		if (OwningComp)
		{
			// Cast VFX: 스킬 시전 시 플레이어 위치에 표시
			if (SkillCastVFX)
			{
				OwningComp->Multicast_PlayCastVFX(CurrentSkillType, OwnerCharacter->GetActorLocation(), OwnerCharacter->GetActorRotation());
			}

			// Impact VFX: 힐링 효과를 플레이어에게 표시
			if (SkillImpactVFX)
			{
				OwningComp->Multicast_PlayImpactVFX(CurrentSkillType, OwnerCharacter->GetActorLocation());
			}
		}

		// SFX 재생 (모든 클라이언트에 동기화)
		if (UGS_SeekerAudioComponent* AudioComp = OwnerCharacter->FindComponentByClass<UGS_SeekerAudioComponent>())
		{
			// Multicast RPC 직접 호출 (CanSendRPC 체크 우회)
			// AudioEventType 0 = 스킬 시작 사운드
			AudioComp->Multicast_RequestSkillAudio(CurrentSkillType, 0, OwnerCharacter->GetActorLocation());
		}
	}

	// 포션 개수 감소
	int32 OldPotionCount = CurrentHealCount;
	CurrentHealCount = FMath::Max(0, CurrentHealCount - 1);
	
	// UI 업데이트를 위해 즉시 클라이언트에 알림 (서버에서만 실행)
	if (OwningComp && OwnerCharacter->HasAuthority())
	{
		OwningComp->Client_BroadcastHealCountChanged(CurrentSkillType, CurrentHealCount, MaxHealCount);
	}
	
	// 스킬 사용 후 비활성화
	// -> 이걸 drinkpotion animation 끝났을 때 실행.
	DeactiveSkill();
}

void UGS_HealSkill::DeactiveSkill()
{
	// 부모 클래스의 DeactiveSkill 호출
	Super::DeactiveSkill();

	// 서버 권한에서만 종료 사운드 재생 (Multicast로 모든 클라이언트에 동기화)
	if (OwnerCharacter && OwnerCharacter->HasAuthority())
	{
		if (UGS_SeekerAudioComponent* AudioComp = OwnerCharacter->FindComponentByClass<UGS_SeekerAudioComponent>())
		{
			// Multicast RPC 직접 호출 (CanSendRPC 체크 우회)
			// AudioEventType 1 = 스킬 종료 사운드
			AudioComp->Multicast_RequestSkillAudio(CurrentSkillType, 1, OwnerCharacter->GetActorLocation());
		}
	}
}

bool UGS_HealSkill::CanActive() const
{
	// 기본 조건 체크 (부모 클래스)
	if (!Super::CanActive())
	{
		return false;
	}
	
	// 힐 스킬 전용 조건 체크
	bool bCanActivateHeal = CanActivateHealSkill();
	
	return bCanActivateHeal;
}

void UGS_HealSkill::SetCurrentHealCount(int32 NewCount)
{
	int32 OldCount = CurrentHealCount;
	CurrentHealCount = FMath::Clamp(NewCount, 0, MaxHealCount);
	
	if (OldCount == 0 && NewCount > 0)
	{
		bIsPotionDepletedOrHealthFull = false;
		SetCoolingDown(false);
	}
	
	// UI 업데이트를 위해 클라이언트에 알림 (서버에서만 실행)
	if (OwningComp && OwnerCharacter && OwnerCharacter->HasAuthority())
	{
		OwningComp->Client_BroadcastHealCountChanged(CurrentSkillType, CurrentHealCount, MaxHealCount);
	}
}

bool UGS_HealSkill::CanUseHeal() const
{
	bool bResult = CurrentHealCount > 0;
	return bResult;
}

bool UGS_HealSkill::IsHealthFull() const
{
	if (!OwnerCharacter)
	{
		return false;
	}
	
	UGS_StatComp* StatComp = OwnerCharacter->GetStatComp();
	if (!StatComp)
	{
		return false;
	}
	
	bool bIsFull = StatComp->GetCurrentHealth() >= StatComp->GetMaxHealth();
	return bIsFull;
}

bool UGS_HealSkill::CanActivateHealSkill() const
{
	// 실제 포션 상태와 체력 상태를 먼저 확인
	bool bCanUsePotion = CanUseHeal();
	bool bIsHealthFull = IsHealthFull();
	bool bShouldBeBlocked = !bCanUsePotion || bIsHealthFull;
	
	// 실제 상태와 bIsPotionDepletedOrHealthFull이 다르면 동기화
	if (!bShouldBeBlocked && bIsPotionDepletedOrHealthFull)
	{
		// 실제로는 사용 가능한데 차단 상태라면 해제
		const_cast<UGS_HealSkill*>(this)->bIsPotionDepletedOrHealthFull = false;
		const_cast<UGS_HealSkill*>(this)->SetCoolingDown(false);
	}
	else if (bShouldBeBlocked && !bIsPotionDepletedOrHealthFull)
	{
		// 실제로는 사용 불가능한데 정상 상태라면 차단
		const_cast<UGS_HealSkill*>(this)->bIsPotionDepletedOrHealthFull = true;
	}

	if (bIsCoolingDown)
	{
		return false;
	}
	
	bool bCanActivate = bCanUsePotion && !bIsHealthFull;
	
	return bCanActivate;
}

void UGS_HealSkill::ShowPotionDepletedEffect()
{
	bIsPotionDepletedOrHealthFull = true;
	SetCoolingDown(true);

	if (OwningComp)
	{
		OwningComp->Client_BroadcastSkillCooldownBlocked_Implementation(CurrentSkillType);
	}

	if (OwnerCharacter && OwnerCharacter->GetWorld())
	{
		FTimerHandle TimerHandle;
		OwnerCharacter->GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
		{
			SetCoolingDown(false);
		}, 2.0f, false);
	}
}

void UGS_HealSkill::InitializeDelegate()
{
	Super::InitializeDelegate();

	OwnerCharacter->OnTakeAnyDamage.AddDynamic(this, &UGS_HealSkill::OnOwnerDamaged);
}

void UGS_HealSkill::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UGS_HealSkill, CurrentHealCount);
}

void UGS_HealSkill::OnRep_CurrentHealCount()
{
	// 포션이 0에서 증가하면 제한 상태 및 쿨다운 해제
	if (CurrentHealCount > 0)
	{
		bIsPotionDepletedOrHealthFull = false;
		SetCoolingDown(false);
	}

	if (OwningComp)
	{
		OwningComp->Client_BroadcastHealCountChanged(CurrentSkillType, CurrentHealCount, MaxHealCount);
	}
}

void UGS_HealSkill::OnOwnerDamaged(AActor* DamagedActor, float DamageAmount, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	// 체력이 가득 찬 상태에서 피해를 입었을 때만 제한 해제
	if (bIsPotionDepletedOrHealthFull)
	{		
		if (!IsHealthFull())
		{
			bIsPotionDepletedOrHealthFull = false;
			SetCoolingDown(false);
		}
	}
}