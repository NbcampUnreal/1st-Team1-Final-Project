// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Skill/Seeker/GS_HealSkill.h"
#include "Character/Player/GS_Player.h"
#include "Character/Component/GS_StatComp.h"
#include "Sound/GS_SeekerAudioComponent.h"
#include "Net/UnrealNetwork.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Animation/Character/GS_SeekerAnimInstance.h"
#include "Props/Item/SeekerItem/GS_HP_Potion.h"
#include "Weapon/GS_Weapon.h"

UGS_HealSkill::UGS_HealSkill()
{
	HealAmount = 200.0f; // 기본 치유량 설정
	MaxHealCount = 5; // 기본 포션 개수
	CurrentHealCount = MaxHealCount; // 시작 시 최대 개수로 설정
	bIsPotionDepletedOrHealthFull = false;
}

void UGS_HealSkill::ActiveSkill()
{
	Super::ActiveSkill();

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
	
	OwnerCharacter->Multicast_PlaySkillMontage(SkillAnimMontages[0]);
	bIsCoolingDown = true;

	AGS_Seeker* Seeker = Cast<AGS_Seeker>(OwnerCharacter);

	if (!Seeker)
	{
		return;
	}
	
	Seeker->Multicast_SetMontageSlot(ESeekerMontageSlot::UpperBody);
	Seeker->Server_SetSeekerGait(EGait::Walk);
}

void UGS_HealSkill::DeactiveSkill()
{
	// 부모 클래스의 DeactiveSkill 호출
	Super::DeactiveSkill();
	
	bIsCoolingDown = false;
	
	// 서버 권한에서만 종료 사운드 재생 (Multicast로 모든 클라이언트에 동기화)
	if (OwnerCharacter && OwnerCharacter->HasAuthority())
	{
		//if (UGS_SeekerAudioComponent* AudioComp = OwnerCharacter->FindComponentByClass<UGS_SeekerAudioComponent>()) // SJE
		UGS_SeekerAudioComponent* AudioComp = OwnerCharacter->FindComponentByClass<UGS_SeekerAudioComponent>();
		if (IsValid(AudioComp))
		{
			// Multicast RPC 직접 호출 (CanSendRPC 체크 우회)
			// AudioEventType 1 = 스킬 종료 사운드
			AudioComp->Multicast_RequestSkillAudio(CurrentSkillType, 1, OwnerCharacter->GetActorLocation());
		}
		AGS_Seeker* Seeker = Cast<AGS_Seeker>(OwnerCharacter);
		if (Seeker)
		{
			if (Seeker->GetSkillComp())
			{
				Seeker->GetSkillComp()->SetCurAllowedSkillsMask(0);
				Seeker->GetSkillComp()->SetCurAllowedSkillsMask(static_cast<int16>(ESkillSlot::HealPotion));
			}
		}
	}
}

void UGS_HealSkill::InterruptSkill()
{
	Super::InterruptSkill();

	if (!OwnerCharacter)
	{
		return;
	}

	if (AGS_Seeker* Seeker = Cast<AGS_Seeker>(OwnerCharacter))
	{
		if (Seeker->IsDead())
		{
			return;
		}
		
		if (Seeker->GetSkillComp())
		{
			Seeker->Multicast_SetMontageSlot(ESeekerMontageSlot::None);
			Seeker->SetMoveControlValue(true, true);
			
			// Potion 떨구기
			SetIsActive(false);
			bIsCoolingDown = false; // hard coding // SJE

			AGS_HP_Potion* Potion = Cast<AGS_HP_Potion>(Seeker->GetItem(EItemType::HP_Potion));
			if (Potion)
			{
				Potion->DropFromSocket();
                	
				UStaticMeshComponent* Mesh = Potion->GetMeshComp();
				if (Mesh)
				{
					Mesh->SetSimulatePhysics(true);
					Mesh->SetEnableGravity(true);
					Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				}
			}
			Seeker->GetSkillComp()->ResetAllowedSkillsMask();
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
	//SetCoolingDown(true); 어차피 true 인데 왜 SEt 하는 거야? // SJE

	if (OwningComp)
	{
		OwningComp->Client_BroadcastSkillCooldownBlocked_Implementation(CurrentSkillType);
	}

	/*if (OwnerCharacter && OwnerCharacter->GetWorld())
	{
		FTimerHandle TimerHandle;
		OwnerCharacter->GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
		{
			SetCoolingDown(false);
		}, 2.0f, false);
	}*/
}

void UGS_HealSkill::InitializeDelegate()
{
	Super::InitializeDelegate();

	OwnerCharacter->OnTakeAnyDamage.AddDynamic(this, &UGS_HealSkill::OnOwnerDamaged);
}

float UGS_HealSkill::GetHealAmount()
{
	return HealAmount;
}

int32 UGS_HealSkill::GetCurrentHealCount()
{
	return CurrentHealCount;
}

void UGS_HealSkill::DecreaseCurrentHealCount()
{
	if (CurrentHealCount > 0)
	{
		CurrentHealCount--;
	}
}

int32 UGS_HealSkill::GetMaxHealCount()
{
	return MaxHealCount;
}

/*void UGS_HealSkill::CheckWeaponStateAndPlayWielding()
{
	AGS_Seeker* Seeker = Cast<AGS_Seeker>(OwnerCharacter);
	if (!Seeker)
	{
		return;
	}

	if (Seeker->GetWeaponHandlingState() == EWeaponHandlingState::Sheathing)
	{
		Seeker->Multicast_SetMontageSlot(ESeekerMontageSlot::UpperBody);
		Seeker->Multicast_PlaySkillMontage(SkillAnimMontages[2]); // // Hard coding // SJE
		Seeker->SetWeaponHandlingState(EWeaponHandlingState::Wielding);
	}
}*/

void UGS_HealSkill::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UGS_HealSkill, CurrentHealCount);
}

void UGS_HealSkill::OnRep_CurrentHealCount()
{
	// 포션이 0에서 증가하면 제한 상태 및 쿨다운 해제
	/*if (CurrentHealCount > 0)
	{
		bIsPotionDepletedOrHealthFull = false;
		SetCoolingDown(false);
	}

	UE_LOG(LogTemp, Error, TEXT("OnRep_CurrentHealCount")); // SJE*/

	/*if (OwningComp)
	{
		OwningComp->Client_BroadcastHealCountChanged(CurrentSkillType, CurrentHealCount, MaxHealCount);
	}*/ // SJE
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