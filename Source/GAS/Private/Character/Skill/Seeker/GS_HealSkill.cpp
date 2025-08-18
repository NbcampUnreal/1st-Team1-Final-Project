// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Skill/Seeker/GS_HealSkill.h"
#include "Character/Player/GS_Player.h"
#include "Character/Component/GS_StatComp.h"
#include "Net/UnrealNetwork.h"

UGS_HealSkill::UGS_HealSkill()
{
	HealAmount = 200.0f; // 기본 치유량 설정
	MaxHealCount = 5; // 기본 포션 개수
	CurrentHealCount = MaxHealCount; // 시작 시 최대 개수로 설정
	bIsPotionDepletedOrHealthFull = false; 
}

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

void UGS_HealSkill::ActiveSkill()
{
	Super::ActiveSkill();

	// 피해 감지 바인딩 초기화 (한 번만 실행됨)
	InitializeDamageBinding();

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
		UGS_StatComp* StatComp = OwnerCharacter->GetStatComp();
		if (StatComp)
		{
			StatComp->ServerRPCHeal(HealAmount);
		}
	}

	// 포션 개수 감소
	int32 OldPotionCount = CurrentHealCount;
	CurrentHealCount = FMath::Max(0, CurrentHealCount - 1);
	
	// 스킬 사용 후 비활성화
	DeactiveSkill();
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
}

void UGS_HealSkill::OnOwnerDamaged(AActor* DamagedActor, float DamageAmount, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	// 체력이 가득 찬 상태에서 피해를 입었을 때만 제한 해제
	if (bIsPotionDepletedOrHealthFull)
	{
		// 현재 체력 상태를 다시 확인
		bool bIsStillHealthFull = IsHealthFull();
		
		if (!bIsStillHealthFull)
		{
			bIsPotionDepletedOrHealthFull = false;
			SetCoolingDown(false);
		}
	}
}