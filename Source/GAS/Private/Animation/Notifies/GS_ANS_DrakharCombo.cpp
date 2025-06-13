#include "Animation/Notifies/GS_ANS_DrakharCombo.h"

#include "Character/Player/Guardian/GS_Drakhar.h"
#include "Kismet/KismetSystemLibrary.h"

void UGS_ANS_DrakharCombo::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                       float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	AGS_Drakhar* Drakhar = Cast<AGS_Drakhar>(MeshComp->GetOwner());
	if (IsValid(Drakhar))
	{
		Drakhar->SetNextComboAttackSection(SectionName);
		//Drakhar->IsAttacking = true;
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("@@@@@Is Attacking %d"),Drakhar->IsAttacking));
		//UE_LOG(LogTemp, Warning, TEXT("[SERVER] IS Attacking %d"), Drakhar->IsAttacking);
	}
}

void UGS_ANS_DrakharCombo::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	AGS_Drakhar* Drakhar = Cast<AGS_Drakhar>(MeshComp->GetOwner());
	if (IsValid(Drakhar))
	{
		Drakhar->ResetComboAttackSection();
		//Drakhar->IsAttacking = false;
		//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("@@@@@Is Attacking %d"),Drakhar->IsAttacking));
		//UE_LOG(LogTemp, Warning, TEXT("[SERVER] IS Attacking %d"), Drakhar->IsAttacking);
	}
}