#include "Character/Skill/Guardian/Drakhar/GS_DrakharWingRush.h"

#include "Character/GS_Character.h"
#include "Character/Component/GS_StatComp.h"
#include "Character/Player/Guardian/GS_Drakhar.h"
#include "Character/Skill/GS_SkillComp.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/RootMotionSource.h"
#include "Templates/SharedPointer.h"

UGS_DrakharWingRush::UGS_DrakharWingRush()
{
	CurrentSkillType = ESkillSlot::Moving;
	DashRemainTime = 1.2f; //not yet
	DashPower = 1000.f;
}

void UGS_DrakharWingRush::ActiveSkill()
{
	Super::ActiveSkill();
	
	AGS_Drakhar* Drakhar = Cast<AGS_Drakhar>(OwnerCharacter);
	if (IsValid(Drakhar))
	{
		Drakhar->ResetComboAttackVariables();
	}
	
	ExecuteSkillEffect();
	
	DashStartLocation = OwnerCharacter->GetActorLocation();
	DashEndLocation = DashStartLocation + OwnerCharacter->GetActorForwardVector() * DashPower;
}

void UGS_DrakharWingRush::ExecuteSkillEffect()
{
	Super::ExecuteSkillEffect();

	OwnerCharacter->MulticastRPCPlaySkillMontage(SkillAnimMontages[0]);
}

