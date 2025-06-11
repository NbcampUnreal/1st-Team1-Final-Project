// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Merci/GS_MerciUltimateSkill.h"
#include "Character/Player/Seeker/GS_Merci.h"
#include "DrawDebugHelpers.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Kismet/GameplayStatics.h"

UGS_MerciUltimateSkill::UGS_MerciUltimateSkill()
{
	CurrentSkillType = ESkillSlot::Ultimate;
}

void UGS_MerciUltimateSkill::ActiveSkill()
{
	if (!CanActive()) return;

	Super::ActiveSkill();

	if(OwnerCharacter)
	{
		// Skill State
		if (OwnerCharacter->GetSkillComp())
		{
			bIsAutoAimingState = true;
			OwnerCharacter->GetSkillComp()->SetSkillActiveState(ESkillSlot::Ultimate, true);
		}

		OwnerCharacter->GetWorldTimerManager().SetTimer(AutoAimingHandle, this, &UGS_MerciUltimateSkill::DeActiveAutoAimingState, AutoAimingStateTime, false);
		OwnerCharacter->GetWorldTimerManager().SetTimer(AutoAimTickHandle, this, &UGS_MerciUltimateSkill::TickAutoAimTarget, AutoAimTickInterval, true);
	}

	UpdateMonsterList();
	ExecuteSkillEffect();
}

void UGS_MerciUltimateSkill::ExecuteSkillEffect()
{
	AActor* Target = FindCloseTarget();
	AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter);
	if (Target && OwnerCharacter->HasAuthority())
	{
		if(MerciCharacter)
		{
			MerciCharacter->SetAutoAimTarget(Target);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("AutoAimingMode Start"));
}

bool UGS_MerciUltimateSkill::IsActive() const
{
	return bIsAutoAimingState;
}

void UGS_MerciUltimateSkill::DeActiveAutoAimingState()
{
	UE_LOG(LogTemp, Warning, TEXT("AutoAimingMode End"));

	if (OwnerCharacter)
	{
		// Skill State
		if (OwnerCharacter->GetSkillComp())
		{
			bIsAutoAimingState = false;
			OwnerCharacter->GetSkillComp()->SetSkillActiveState(ESkillSlot::Ultimate, false);
		}

		OwnerCharacter->GetWorldTimerManager().ClearTimer(AutoAimTickHandle);
		OwnerCharacter->GetWorldTimerManager().ClearTimer(AutoAimingHandle);
	}
}

AActor* UGS_MerciUltimateSkill::FindCloseTarget()
{
	if (!OwnerCharacter) return nullptr;
	
	FVector CamLoc;
	FRotator CamRot;
	OwnerCharacter->GetController()->GetPlayerViewPoint(CamLoc, CamRot);
	FVector ViewDir = CamRot.Vector();

	float CloseDot = 0.8f; // 최소 허용 Dot
	AActor* BestTarget = nullptr;

	for (AActor* Target : AllMonsterActors)
	{
		if (!IsValid(Target)) continue;

		FVector ToTarget = (Target->GetActorLocation() - CamLoc).GetSafeNormal();
		float Dot = FVector::DotProduct(ViewDir, ToTarget);

		if (Dot > CloseDot)
		{
			CloseDot = Dot;
			BestTarget = Target;
		}
	}

	return BestTarget;
}

void UGS_MerciUltimateSkill::TickAutoAimTarget()
{
	if (!OwnerCharacter) 
	{
		UE_LOG(LogTemp, Warning, TEXT("OwnerCharacter null"));
		return;
	}

	if (!IsActive())
	{
		UE_LOG(LogTemp, Warning, TEXT("IsActive is false"));
		return;
	}

	AActor* Target = FindCloseTarget();
	if (!OwnerCharacter->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("IsClient!!!!!!!!!!!!!!!!"));
	}
	if (Target && OwnerCharacter->HasAuthority())
	{
		if (AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter))
		{
			UE_LOG(LogTemp, Warning, TEXT("TickAutoAimTarget"));
			MerciCharacter->SetAutoAimTarget(Target);
		}
	}
}

void UGS_MerciUltimateSkill::UpdateMonsterList()
{
	AllMonsterActors.Empty();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGS_Monster::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		if (IsValid(Actor)) AllMonsterActors.Add(Actor);
	}
}
