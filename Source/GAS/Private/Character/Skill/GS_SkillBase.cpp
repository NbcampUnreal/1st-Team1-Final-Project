#include "Character/Skill/GS_SkillBase.h"
#include "Character/GS_Character.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Logging/StructuredLogFormat.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

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

void UGS_SkillBase::InitSkill(AGS_Character* InOwner, UGS_SkillComp* InOwningComp)
{
	OwnerCharacter = InOwner;
	OwningComp = InOwningComp;
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

void UGS_SkillBase::PlayCastVFX(FVector Location, FRotator Rotation)
{
	if (SkillCastVFX && OwnerCharacter)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			SkillCastVFX,
			OwnerCharacter->GetRootComponent(),
			NAME_None,
			CastVFXOffset, // 데이터 테이블에서 설정된 오프셋 사용
			Rotation,
			EAttachLocation::KeepRelativeOffset,
			true
		);
	}
}

void UGS_SkillBase::PlayRangeVFX(FVector Location, float Radius)
{
	if (SkillRangeVFX && OwnerCharacter)
	{
		UNiagaraComponent* SpawnedVFX = UNiagaraFunctionLibrary::SpawnSystemAttached(
			SkillRangeVFX,
			OwnerCharacter->GetRootComponent(),
			NAME_None,
			RangeVFXOffset, // 데이터 테이블에서 설정된 오프셋 사용
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true
		);

		if (SpawnedVFX)
		{
			SpawnedVFX->SetWorldScale3D(SkillVFXScale);
			SpawnedVFX->SetFloatParameter(FName("Radius"), Radius);
			if (SkillVFXDuration > 0.0f)
			{
				SpawnedVFX->SetFloatParameter(FName("Duration"), SkillVFXDuration);
			}
		}
	}
}

void UGS_SkillBase::PlayImpactVFX(FVector Location)
{
	if (SkillImpactVFX && OwnerCharacter)
	{
		// 이 함수는 타겟 정보를 받지 않으므로, 월드 위치에 생성.
		// 타겟에 부착하려면 별도 함수 필요.
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			SkillImpactVFX,
			Location,
			FRotator::ZeroRotator,
			SkillVFXScale,
			true, true, ENCPoolMethod::None
		);
	}
}

// 타겟에 직접 Impact VFX를 부착하는 새 함수
void UGS_SkillBase::PlayImpactVFXOnTarget(AActor* Target)
{
	if (SkillImpactVFX && Target)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			SkillImpactVFX,
			Target->GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true
		);
	}
}

void UGS_SkillBase::PlayEndVFX(FVector Location, FRotator Rotation)
{
	if (SkillEndVFX && OwnerCharacter)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			SkillEndVFX,
			OwnerCharacter->GetRootComponent(),
			NAME_None,
			EndVFXOffset, // 데이터 테이블에서 설정된 오프셋 사용
			Rotation,
			EAttachLocation::KeepRelativeOffset,
			true
		);
	}
}