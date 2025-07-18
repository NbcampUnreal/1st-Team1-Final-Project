// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/GS_SkillComp.h"
#include "Character/Player/GS_Player.h"
#include "Character/Skill/GS_SkillBase.h"
#include "Character/Skill/GS_SkillSet.h"
#include "UI/Character/GS_SkillWidget.h"
#include "Net/UnrealNetwork.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Character/E_Character.h"


UGS_SkillComp::UGS_SkillComp()
{
	PrimaryComponentTick.bCanEverTick = false;
	
}

void UGS_SkillComp::ApplyCooldownModifier(ESkillSlot Slot, float Ratio)
{
	if (UGS_SkillBase* Skill = SkillMap.FindRef(Slot))
	{
		Skill->Cooltime *= Ratio;
	}

	// 현재 쿨다운이 있는 상태면 남은 시간도 줄여줌
	if (FSkillCooldownState* State = CooldownStates.Find(Slot))
	{
		if (State->bIsOnCooldown)
		{
			// 남은 시간 비례 축소
			State->CooldownRemaining *= Ratio;

			// 타이머 갱신
			float NewRemaining = State->CooldownRemaining;

			GetWorld()->GetTimerManager().ClearTimer(State->CooldownTimer);
			GetWorld()->GetTimerManager().ClearTimer(State->UIUpdateTimer);

			if (NewRemaining > 0.0f)
			{
				GetWorld()->GetTimerManager().SetTimer(
					State->CooldownTimer,
					[this, Slot]() { HandleCooldownComplete(Slot); },
					NewRemaining,
					false
				);

				GetWorld()->GetTimerManager().SetTimer(
					State->UIUpdateTimer,
					[this, Slot]() { HandleCooldownProgress(Slot); },
					0.1f,
					true
				);
			}

			UpdateReplicatedCooldownStates(); // UI에도 반영
		}
	}
}

void UGS_SkillComp::ResetCooldownModifier(ESkillSlot Slot)
{
	if (!SkillDataTable || !GetOwner()) return;

	UGS_SkillBase* Skill = GetSkillFromSkillMap(Slot);
	if (!Skill) return;

	// 캐릭터 타입을 기반으로 RowName 구하기
	AGS_Character* OwnerCharacter = Cast<AGS_Character>(GetOwner());
	if (!OwnerCharacter) return;

	FName RowName = FName(*UEnum::GetValueAsString(OwnerCharacter->GetCharacterType()).RightChop(
		UEnum::GetValueAsString(OwnerCharacter->GetCharacterType()).Find(TEXT("::")) + 2));

	FString Context;
	const FGS_SkillSet* SkillSet = SkillDataTable->FindRow<FGS_SkillSet>(RowName, Context);
	if (!SkillSet) return;

	float OriginalCooltime = 0.f;

	switch (Slot)
	{
	case ESkillSlot::Ready:
		OriginalCooltime = SkillSet->ReadySkill.Cooltime;
		break;
	case ESkillSlot::Moving:
		OriginalCooltime = SkillSet->MovingSkill.Cooltime;
		break;
	case ESkillSlot::Aiming:
		OriginalCooltime = SkillSet->AimingSkill.Cooltime;
		break;
	case ESkillSlot::Ultimate:
		OriginalCooltime = SkillSet->UltimateSkill.Cooltime;
		break;
	case ESkillSlot::Rolling:
		OriginalCooltime = SkillSet->RollingSkill.Cooltime;
		break;
	default:
		break;
	}

	if (OriginalCooltime > 0.f)
	{
		Skill->Cooltime = OriginalCooltime;
	}
}

void UGS_SkillComp::BeginPlay()
{
	Super::BeginPlay();
	
	SetIsReplicated(true);

	/*if (GetOwner()->HasAuthority())
	{
		//ServerRPCInitSkills();
		InitSkills();
	}*/
	InitSkills();
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
		SetSkill(ESkillSlot::Ready, SkillSet->ReadySkill);
		SetSkill(ESkillSlot::Aiming, SkillSet->AimingSkill);
		SetSkill(ESkillSlot::Moving, SkillSet->MovingSkill);
		SetSkill(ESkillSlot::Ultimate, SkillSet->UltimateSkill);
		SetSkill(ESkillSlot::Rolling, SkillSet->RollingSkill);
	}
}

void UGS_SkillComp::SetSkill(ESkillSlot Slot, const FSkillInfo& Info)
{
	if (!Info.SkillClass)
	{
		UE_LOG(LogTemp, Error, TEXT(">>> SetSkill: Invalid SkillClass"));
		return;
	}

	UGS_SkillBase* Skill = NewObject<UGS_SkillBase>(this, Info.SkillClass);
	if (!Skill)
	{
		UE_LOG(LogTemp, Error, TEXT(">>> SetSkill: Failed to create skill object"));
		return;
	}

	Skill->InitSkill(Cast<AGS_Player>(GetOwner()), this, Slot);
	Skill->Cooltime = Info.Cooltime;
	Skill->Damage = Info.Damage;
	Skill->SkillAnimMontages = Info.Montages;
	Skill->SkillImage = Info.Image;
	
	// VFX 정보 설정
	Skill->SkillCastVFX = Info.SkillCastVFX;
	Skill->SkillRangeVFX = Info.SkillRangeVFX;
	Skill->SkillImpactVFX = Info.SkillImpactVFX;
	Skill->SkillEndVFX = Info.SkillEndVFX;
	Skill->SkillVFXScale = Info.SkillVFXScale;
	Skill->SkillVFXDuration = Info.SkillVFXDuration;
	
	// VFX 오프셋 정보 설정
	Skill->CastVFXOffset = Info.CastVFXOffset;
	Skill->RangeVFXOffset = Info.RangeVFXOffset;
	Skill->ImpactVFXOffset = Info.ImpactVFXOffset;
	Skill->EndVFXOffset = Info.EndVFXOffset;
	
	SkillMap.Add(Slot, Skill);
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
		if (SkillMap[Slot]->CanActive())
		{
			SkillsInterrupt();
			SkillMap[Slot]->ActiveSkill();
			
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("CanActive() = false"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillMap does not contain slot"));
	}
}

void UGS_SkillComp::TryDeactiveSkill(ESkillSlot Slot)
{
	if (!GetOwner()->HasAuthority())
	{
		Server_TryDeactiveSkill(Slot);
		return;
	}
	
	if (SkillMap.Contains(Slot))
	{
		SkillMap[Slot]->DeactiveSkill();
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

void UGS_SkillComp::StartCooldownForSkill(ESkillSlot Slot)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	UGS_SkillBase* Skill = SkillMap.FindRef(Slot);
	if (!Skill)
	{
		return;
	}

	float CooldownTime = Skill->GetCoolTime();
	if (CooldownTime <= 0.0f)
	{
		return;
	}
	
	FSkillCooldownState& State = CooldownStates.FindOrAdd(Slot);
	State.Slot = Slot;
	State.CooldownRemaining = CooldownTime;
	State.bIsOnCooldown = true;

	// 쿨다운 타이머
	TWeakObjectPtr<UGS_SkillComp> WeakThis = this;
	GetWorld()->GetTimerManager().SetTimer(
		State.CooldownTimer,
		[WeakThis, Slot]() 
		{ 
			if (WeakThis.IsValid())
			{
				WeakThis->HandleCooldownComplete(Slot); 
			}
		},
		CooldownTime,
		false
	);

	// UI 업데이트 타이머
	GetWorld()->GetTimerManager().SetTimer(
		State.UIUpdateTimer,
		[WeakThis, Slot]() 
		{ 
			if (WeakThis.IsValid())
			{
				WeakThis->HandleCooldownProgress(Slot); 
			}
		},
		0.1f,
		true
	);
	
	Skill->SetCoolingDown(true);
	UpdateReplicatedCooldownStates();
}

void UGS_SkillComp::SkillsInterrupt()
{
	AGS_Seeker* Seeker = Cast<AGS_Seeker>(GetOwner());
	if (Seeker == nullptr)
	{return;}
	
	if (Seeker->CurrentComboIndex > 0)
	{
		Seeker->CurrentComboIndex = 0;
		Seeker->CanAcceptComboInput = true;
		Seeker->bNextCombo = false;
	}
	
	for (TPair<ESkillSlot, UGS_SkillBase*> slot : SkillMap)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s"), *slot.Value->GetName());

		if (Seeker->GetSkillComp()->IsSkillActive(slot.Key))
		{
			slot.Value->InterruptSkill();
			if (Seeker->GetCharacterType() != ECharacterType::Merci)
			{
				Seeker->SetSkillInputControl(false, false, false);
			}
		}
	}
}

void UGS_SkillComp::HandleCooldownComplete(ESkillSlot Slot)
{
	FSkillCooldownState* State = CooldownStates.Find(Slot);
	if (!State)
	{
		return;
	}
	
	GetWorld()->GetTimerManager().ClearTimer(State->CooldownTimer);
	GetWorld()->GetTimerManager().ClearTimer(State->UIUpdateTimer);
	
	State->CooldownRemaining = 0.0f;
	State->bIsOnCooldown = false;

	UpdateReplicatedCooldownStates();
	
	if (UGS_SkillBase* Skill = SkillMap.FindRef(Slot))
	{
		Skill->SetCoolingDown(false);
	}
}

void UGS_SkillComp::HandleCooldownProgress(ESkillSlot Slot)
{
	FSkillCooldownState* State = CooldownStates.Find(Slot);
	if (!State || !State->bIsOnCooldown)
	{
		return;
	}
	
	float RemainingTime = GetWorld()->GetTimerManager().GetTimerRemaining(State->CooldownTimer);
	RemainingTime = FMath::Max(0.0f, RemainingTime);

	State->CooldownRemaining = RemainingTime;
	UpdateReplicatedCooldownStates();
}

void UGS_SkillComp::UpdateReplicatedCooldownStates()
{
	ReplicatedCooldownStates.Empty();
	for (const auto& Pair : CooldownStates)
	{
		ReplicatedCooldownStates.Add(Pair.Value);
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

void UGS_SkillComp::OnRep_CooldownStates()
{
	for (const FSkillCooldownState& State : ReplicatedCooldownStates)
	{
		ESkillSlot Slot = State.Slot;
		
		OnSkillCooldownChanged.Broadcast(Slot, State.CooldownRemaining);
		
		if (UGS_SkillBase* Skill = SkillMap.FindRef(Slot))
		{
			Skill->SetCoolingDown(State.bIsOnCooldown);
		}
	}
}

void UGS_SkillComp::InitializeSkillWidget(UGS_SkillWidget* InSkillWidget)
{
	if (IsValid(InSkillWidget))
	{
		//client
		ESkillSlot Slot = InSkillWidget->GetSkillSlot();
		
		InitSkills();
		
		if (SkillMap.Contains(Slot))
		{
			InSkillWidget->Initialize(SkillMap[Slot]);
			
			OnSkillCooldownChanged.AddUObject(InSkillWidget, &UGS_SkillWidget::OnSkillCoolTimeChanged);
		}
	}
}


UGS_SkillBase* UGS_SkillComp::GetSkillFromSkillMap(ESkillSlot Slot)
{
	if (SkillMap.Contains(Slot))
	{
		return SkillMap[Slot];
	}
	return nullptr;
}

void UGS_SkillComp::Server_TryDeactiveSkill_Implementation(ESkillSlot Slot)
{
	TryDeactiveSkill(Slot);
}

void UGS_SkillComp::Server_TryActiveSkill_Implementation(ESkillSlot Slot)
{
	TryActivateSkill(Slot);
}

void UGS_SkillComp::Server_TrySkillCommand_Implementation(ESkillSlot Slot)
{
	TrySkillCommand(Slot);
}

void UGS_SkillComp::Multicast_PlayCastVFX_Implementation(ESkillSlot Slot, FVector Location, FRotator Rotation)
{
	if (UGS_SkillBase* Skill = GetSkillFromSkillMap(Slot))
	{
		Skill->PlayCastVFX(Location, Rotation);
	}
}

void UGS_SkillComp::Multicast_PlayRangeVFX_Implementation(ESkillSlot Slot, FVector Location, float Radius)
{
	if (UGS_SkillBase* Skill = GetSkillFromSkillMap(Slot))
	{
		Skill->PlayRangeVFX(Location, Radius);
	}
}

void UGS_SkillComp::Multicast_PlayImpactVFX_Implementation(ESkillSlot Slot, FVector Location)
{
	if (UGS_SkillBase* Skill = GetSkillFromSkillMap(Slot))
	{
		Skill->PlayImpactVFX(Location);
	}
}

void UGS_SkillComp::Multicast_PlayEndVFX_Implementation(ESkillSlot Slot, FVector Location, FRotator Rotation)
{
	if (UGS_SkillBase* Skill = GetSkillFromSkillMap(Slot))
	{
		Skill->PlayEndVFX(Location, Rotation);
	}
}

void UGS_SkillComp::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UGS_SkillComp, ReplicatedSkillStates);
	DOREPLIFETIME(UGS_SkillComp, bCanUseSkill);
	DOREPLIFETIME(UGS_SkillComp, ReplicatedCooldownStates);
}

void UGS_SkillComp::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (GetWorld())
	{
		for (auto& Pair : CooldownStates)
		{
			FSkillCooldownState& State = Pair.Value;
			GetWorld()->GetTimerManager().ClearTimer(State.CooldownTimer);
			GetWorld()->GetTimerManager().ClearTimer(State.UIUpdateTimer);
		}
	}
}
