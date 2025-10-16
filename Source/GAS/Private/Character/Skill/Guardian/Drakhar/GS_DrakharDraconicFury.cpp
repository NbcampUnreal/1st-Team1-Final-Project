#include "Character/Skill/Guardian/Drakhar/GS_DrakharDraconicFury.h"
#include "Character/Player/GS_Player.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Player/Guardian/GS_Drakhar.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

UGS_DrakharDraconicFury::UGS_DrakharDraconicFury()
{
	CurrentSkillType = ESkillSlot::Ultimate;
	CurrentIndicatorIndex = 0;
}

void UGS_DrakharDraconicFury::ActiveSkill()
{
	Super::ActiveSkill();

	// Early return: 스킬 활성화 가능 여부 체크
	if (!CanActive())
	{
		return;
	}

	// 서버에서만 타겟 위치 생성 (투사체가 사용)
	if (OwnerCharacter && OwnerCharacter->HasAuthority())
	{
		AGS_Drakhar* Drakhar = Cast<AGS_Drakhar>(OwnerCharacter);
		if (Drakhar)
		{
			Drakhar->GenerateDraconicFuryTargets();
		}
	}

	ExecuteSkillEffect();
}

void UGS_DrakharDraconicFury::ExecuteSkillEffect()
{
	// Early return: 서버가 아니면 실행 안함
	if (!OwnerCharacter || !OwnerCharacter->HasAuthority())
	{
		return;
	}

	// 가디언 상태를 궁극기 사용 중으로 변경
	AGS_Guardian* Guardian = Cast<AGS_Guardian>(OwnerCharacter);
	if (Guardian)
	{
		Guardian->GuardianDoSkillState = EGuardianDoSkill::Ultimate;
	}

	// 쿨다운 시작
	StartCoolDown();

	// 모든 클라이언트에서 몽타주 재생
	OwnerCharacter->MulticastRPCPlaySkillMontage(SkillAnimMontages[0]);
}

void UGS_DrakharDraconicFury::OnSkillAnimationEnd()
{
	Super::OnSkillAnimationEnd();

	// 타이머 정리
	if (OwnerCharacter && OwnerCharacter->GetWorld())
	{
		OwnerCharacter->GetWorld()->GetTimerManager().ClearTimer(IndicatorTimerHandle);
	}
}