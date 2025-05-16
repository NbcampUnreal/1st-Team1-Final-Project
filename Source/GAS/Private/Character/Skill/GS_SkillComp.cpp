// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/GS_SkillComp.h"
#include "Character/GS_Character.h"
#include "Character/Skill/GS_SkillBase.h"
#include "Character/Skill/Seeker/Chan/GS_ChanAimingSkill.h"
#include "Character/Skill/Seeker/Chan/GS_ChanMovingSkill.h"
#include "Character/Skill/Seeker/Chan/GS_ChanUltimateSkill.h"
#include "Character/Skill/GS_SkillSet.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UGS_SkillComp::UGS_SkillComp()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGS_SkillComp::TryActivateSkill(ESkillSlot Slot)
{
	if (!bCanUseSkill)
	{
		UE_LOG(LogTemp, Warning, TEXT("TryActivateSkill failed: bCanUseSkill = false"));
		return;
	}

	if (!GetOwner()->HasAuthority())
	{
		Server_TryActiveSkill(Slot);
		return;
	}

	if (SkillMap.Contains(Slot))
	{
		UGS_SkillBase* Skill = SkillMap[Slot];
		if (Skill)
		{
			if (Skill->CanActive())
			{
				Skill->ActiveSkill();
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("CanActive() = false"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Skill is nullptr"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillMap does not contain slot"));
	}
}

void UGS_SkillComp::TrySkillCommand(ESkillSlot Slot)
{
	if (!GetOwner()->HasAuthority())
	{
		Server_TrySkillCommand(Slot);
		return;
	}

	if (SkillMap.Contains(Slot))
	{
		if (UGS_SkillBase* Skill = SkillMap[Slot])
		{
			Skill->OnSkillCommand();
		}
	}
}

void UGS_SkillComp::SetSkill(ESkillSlot Slot, TSubclassOf<UGS_SkillBase> SkillClass)
{
	if (!SkillClass)
	{
		UE_LOG(LogTemp, Error, TEXT(">>> SetSkill: Invalid SkillClass"));
		return;
	}

	UGS_SkillBase* Skill = NewObject<UGS_SkillBase>(this, SkillClass);
	if (!Skill)
	{
		UE_LOG(LogTemp, Error, TEXT(">>> SetSkill: Failed to create skill object"));
		return;
	}

	Skill->InitSkill(Cast<AGS_Character>(GetOwner()));
	SkillMap.Add(Slot, Skill);
}

void UGS_SkillComp::SetCanUseSkill(bool InCanUseSkill)
{
	bCanUseSkill = InCanUseSkill;
}

void UGS_SkillComp::SetSkillActiveState(ESkillSlot Slot, bool InIsActive)
{
	bool bFound = false;

	// ReplicatedSkillStates 갱신
	for (FSkillRuntimeState& State : ReplicatedSkillStates)
	{
		if (State.Slot == Slot)
		{
			State.bIsActive = InIsActive;
			bFound = true;
			break;
		}
	}
	if (!bFound)
	{
		ReplicatedSkillStates.Add({ Slot, InIsActive });
	}

	// SkillStates는 항상 갱신 (Standalone 대응)
	SkillStates.FindOrAdd(Slot).Slot = Slot;
	SkillStates[Slot].bIsActive = InIsActive;
}

bool UGS_SkillComp::IsSkillActive(ESkillSlot Slot) const
{
	if (const FSkillRuntimeState* State = SkillStates.Find(Slot))
	{
		return State->bIsActive;
	}
	return false;
}


// Called when the game starts
void UGS_SkillComp::BeginPlay()
{
	Super::BeginPlay();
	SetIsReplicated(true);

	if (GetOwner()->HasAuthority())
	{
		InitSkills();
	}
}

void UGS_SkillComp::OnRep_SkillStates()
{
	SkillStates.Empty();
	for (const FSkillRuntimeState& State : ReplicatedSkillStates)
	{
		SkillStates.Add(State.Slot, State);
	}
}

void UGS_SkillComp::Server_TryActiveSkill_Implementation(ESkillSlot Slot)
{
	TryActivateSkill(Slot);
}

void UGS_SkillComp::Server_TrySkillCommand_Implementation(ESkillSlot Slot)
{
	TrySkillCommand(Slot);
}

void UGS_SkillComp::InitSkills()
{
	AGS_Character* OwnerCharacter = Cast<AGS_Character>(GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT(">>> InitSkills: Invalid OwnerCharacter"));
		return;
	}

	if (!SkillDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT(">>> SkillDataTable is null!"));
		return;
	}

	FName RowName = FName(*UEnum::GetValueAsString(OwnerCharacter->GetCharacterType()).RightChop(
		UEnum::GetValueAsString(OwnerCharacter->GetCharacterType()).Find(TEXT("::")) + 2));
	FString Context;

	FGS_SkillSet* SkillSet = SkillDataTable->FindRow<FGS_SkillSet>(RowName, Context);
	if (SkillSet)
	{
		// 스킬 세팅
		SetSkill(ESkillSlot::Aiming, SkillSet->AimingSkill);
		SetSkill(ESkillSlot::Moving, SkillSet->MovingSkill);
		SetSkill(ESkillSlot::Ultimate, SkillSet->UltimateSkill);
	}
}

void UGS_SkillComp::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGS_SkillComp, ReplicatedSkillStates);
}

