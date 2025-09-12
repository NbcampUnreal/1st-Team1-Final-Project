#include "Animation/Notifies/GS_ANS_ComboStartEnd.h"

#include "Character/Player/Guardian/GS_Drakhar.h"

void UGS_ANS_ComboStartEnd::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                        float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	AGS_Drakhar* Drakhar = Cast<AGS_Drakhar>(MeshComp->GetOwner());
	if (IsValid(Drakhar))
	{
		Drakhar->IsAttacking = true;
	}
}

void UGS_ANS_ComboStartEnd::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	AGS_Drakhar* Drakhar = Cast<AGS_Drakhar>(MeshComp->GetOwner());
	if (IsValid(Drakhar))
	{
		Drakhar->IsAttacking = false;
	}
}
