#include "Animation/Notifies/GS_ANS_DrkharDash.h"

#include "Character/Player/Guardian/GS_Drakhar.h"

void UGS_ANS_DrkharDash::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (AGS_Drakhar* Drakhar = Cast<AGS_Drakhar>(Owner))
		{
			Drakhar->GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Drakhar->ServerRPCCalculateDashLocation();
		}
	}
}

void UGS_ANS_DrkharDash::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
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

void UGS_ANS_DrkharDash::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
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
