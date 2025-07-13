#include "Character/Skill/GS_SkillBase.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Skill/GS_SkillComp.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Animation/Character/GS_SeekerAnimInstance.h"
#include "Character/Player/Seeker/GS_Seeker.h"

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

void UGS_SkillBase::InitSkill(AGS_Player* InOwner, UGS_SkillComp* InOwningComp, ESkillSlot InSlot)
{
	OwnerCharacter = InOwner;
	OwningComp = InOwningComp;
	CurrentSkillType = InSlot;
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

void UGS_SkillBase::InterruptSkill()
{
	AGS_Seeker* Seeker = Cast<AGS_Seeker>(OwnerCharacter);
	
	if (UGS_SeekerAnimInstance* SeekerAnim = Cast<UGS_SeekerAnimInstance>(OwnerCharacter->GetMesh()->GetAnimInstance()))
	{
		Seeker->SetSkillInputControl(true, true, true);
		Seeker->SetSeekerGait(EGait::Run);
		Seeker->CanChangeSeekerGait = true;
	}
}

void UGS_SkillBase::StartCoolDown()
{
	if (OwningComp)
	{
		OwningComp->StartCooldownForSkill(CurrentSkillType);
	}
}


void UGS_SkillBase::PlayCastVFX(FVector Location, FRotator Rotation)
{
	if (SkillCastVFX && OwnerCharacter)
	{
		UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
			SkillCastVFX,
			OwnerCharacter->GetRootComponent(),
			NAME_None,
			CastVFXOffset, // 데이터 테이블에서 설정된 오프셋 사용
			Rotation,
			EAttachLocation::KeepRelativeOffset,
			true
		);

		if (NiagaraComp)
		{
			FVector Forward = OwnerCharacter->GetActorForwardVector();
			NiagaraComp->SetVectorParameter(FName("User.ForwardVector"), Forward);

			NiagaraComp->SetVectorParameter(FName("User.FixedVector"), FVector(1,0,0));
			
		}
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

const FSkillInfo* UGS_SkillBase::GetCurrentSkillInfo() const
{
	if (!OwningComp || !OwnerCharacter)
	{
		return nullptr;
	}

	UDataTable* SkillDataTable = OwningComp->GetSkillDataTable();
	if (!SkillDataTable)
	{
		return nullptr;
	}
	
	// 캐릭터 타입을 기반으로 RowName 구하기
	FName RowName = FName(*UEnum::GetValueAsString(OwnerCharacter->GetCharacterType()).RightChop(
		UEnum::GetValueAsString(OwnerCharacter->GetCharacterType()).Find(TEXT("::")) + 2));

	FString Context;
	const FGS_SkillSet* SkillSet = SkillDataTable->FindRow<FGS_SkillSet>(RowName, Context);
	if (!SkillSet)
	{
		return nullptr;
	}

	// 현재 스킬 슬롯에 따라 적절한 스킬 정보 반환
	switch (CurrentSkillType)
	{
	case ESkillSlot::Ready:
		return &SkillSet->ReadySkill;
	case ESkillSlot::Aiming:
		return &SkillSet->AimingSkill;
	case ESkillSlot::Moving:
		return &SkillSet->MovingSkill;
	case ESkillSlot::Ultimate:
		return &SkillSet->UltimateSkill;
	case ESkillSlot::Rolling:
		return &SkillSet->RollingSkill;
	default:
		return nullptr;
	}
}