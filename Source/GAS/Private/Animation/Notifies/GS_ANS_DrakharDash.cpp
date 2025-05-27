#include "Animation/Notifies/GS_ANS_DrakharDash.h"

#include "Character/Player/Guardian/GS_Drakhar.h"

void UGS_ANS_DrakharDash::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (AGS_Drakhar* Drakhar = Cast<AGS_Drakhar>(Owner))
		{
			Drakhar->ServerRPCCalculateDashLocation();
		}
	}
}

void UGS_ANS_DrakharDash::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (AGS_Drakhar* Drakhar = Cast<AGS_Drakhar>(Owner))
		{
			//attack and moving
			Drakhar->ServerRPCDoDash(FrameDeltaTime);
		}
	}
}

void UGS_ANS_DrakharDash::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	
	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (AGS_Drakhar* Drakhar = Cast<AGS_Drakhar>(Owner))
		{
			Drakhar->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			//데미지 처리
			Drakhar->ServerRPCEndDash();
		}
	}
}
