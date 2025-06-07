#include "Character/Skill/GS_SkillBase.h"
#include "Character/GS_Character.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Logging/StructuredLogFormat.h"

UTexture2D* UGS_SkillBase::GetSkillImage()
{
	if (SkillImage)
	{
		return SkillImage;
	}
	return nullptr;
}

float UGS_SkillBase::GetCoolTime()
{
	return Cooltime;
}

void UGS_SkillBase::InitSkill(AGS_Character* InOwner)
{
	OwnerCharacter = InOwner;
}

void UGS_SkillBase::ActiveSkill()
{
	if (!CanActive()) return;

	StartCoolDown();
}

void UGS_SkillBase::DeactiveSkill()
{
}

void UGS_SkillBase::ExecuteSkillEffect()
{
}

void UGS_SkillBase::OnSkillCommand()
{
}

bool UGS_SkillBase::CanActive() const
{
	return OwnerCharacter && !bIsCoolingDown;
}

bool UGS_SkillBase::IsActive() const
{
	return bIsActive;
}

void UGS_SkillBase::StartCoolDown()
{
	//server logic

	if (Cooltime <= 0.f)
	{
		bIsCoolingDown = false;
		return;
	}

	bIsCoolingDown = true;
	
	TWeakObjectPtr<UGS_SkillBase> WeakThis(this);
	if (OwnerCharacter)
	{
		OwnerCharacter->GetWorldTimerManager().SetTimer(CooldownHandle, [WeakThis]()
			{
				if (!WeakThis.IsValid())
				{
					return;
				}

				WeakThis->bIsCoolingDown = false;
				UE_LOG(LogTemp, Warning, TEXT("Cool Down Complete"));

			}, Cooltime, false);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(LogTimerHandle, [WeakThis]()
			{
				if (!WeakThis.IsValid())
				{
					return;
				}

				WeakThis->LogRemainingTime();

			}, 0.07f, true);
	}

	// //guardian state setting
	// AGS_Guardian* Guardian = Cast<AGS_Guardian>(OwnerCharacter);
	// if (IsValid(Guardian))
	// {
	// 	Guardian->GuardianState = EGuardianState::SKillCoolDown;
	// }
}

void UGS_SkillBase::LogRemainingTime()
{
	if (!GetWorld()) return;
	//server logic
	
	LeftCoolTime = GetWorld()->GetTimerManager().GetTimerRemaining(CooldownHandle);
	
	SetCoolTime(LeftCoolTime);

	//end cool time
	if (LeftCoolTime <= 0.f)
	{
		GetWorld()->GetTimerManager().ClearTimer(LogTimerHandle);

		SetCoolTime(Cooltime);
	}
}

void UGS_SkillBase::SetCoolTime(float InCoolTime)
{
	if (CurrentSkillType==ESkillSlot::Moving)
	{
		OwnerCharacter->GetSkillComp()->Skill1LeftCoolTime = InCoolTime;
	}
	if (CurrentSkillType==ESkillSlot::Aiming)
	{
		OwnerCharacter->GetSkillComp()->Skill2LeftCoolTime = InCoolTime;
	}
	if (CurrentSkillType==ESkillSlot::Ultimate)
	{
		OwnerCharacter->GetSkillComp()->Skill3LeftCoolTime = InCoolTime;
	}
}