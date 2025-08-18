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
	CurAllowedSkillsMask = DefaultAllowedSkillsMask;
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
	InitSkills();
}

bool UGS_SkillComp::IsSkillAllowed(ESkillSlot CompareSkillType)
{
	uint8 BitFlag = 0;
	BitFlag |= (1 << static_cast<int32>(CompareSkillType));
	return CurAllowedSkillsMask & BitFlag;
}  // SJE

void UGS_SkillComp::SetCurAllowedSkillsMask(int8 BitMask)
{
	CurAllowedSkillsMask = BitMask;	
} // SJE

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
		SetSkill(ESkillSlot::Combo, SkillSet->ComboSkill);
	}
}

void UGS_SkillComp::ResetAllowedSkillsMask()
{
	CurAllowedSkillsMask = DefaultAllowedSkillsMask;
}  // SJE

void UGS_SkillComp::Server_TrySkillAnimationEnd_Implementation(ESkillSlot Slot)
{
	if (SkillMap.Contains(Slot))
	{
		SkillMap[Slot]->OnSkillAnimationEnd();
	}
}

void UGS_SkillComp::TrySkillAnimationEnd(ESkillSlot Slot)
{
	if (GetOwner()->HasAuthority())
	{
		if (SkillMap.Contains(Slot))
		{
			SkillMap[Slot]->OnSkillAnimationEnd();
		}
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


	for (const FSkillAllow& Entry : Info.AllowSkillList)
	{
		const int32 Index = static_cast<int32>(Entry.Slot);

		if (Index >= 0 && Index < 8) // uint8 제한
		{
			if (Entry.bAllow)
			{
				Skill->AllowSkillsMask |= (1 << Index);
			}
		}
	} // SJE
	
	/*UE_LOG(LogTemp, Log, TEXT("Skill Name : %s / Flag (Binary): %s"), *UEnum::GetValueAsString(Slot), 
		*([](uint8 Value)
		{
			FString Result;
			for (int8 i = 7; i >= 0; --i)
			{
				Result.AppendChar((Value & (1 << i)) ? '1' : '0');
			}
			return Result;
		})(Skill->AllowSkillsMask)
	); // SJE*/
	
	Skill-> AllowControlValue = Info.AllowControlValue; // SJE
	
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


void UGS_SkillComp::Server_TryActivateSkill_Implementation(ESkillSlot Slot)
{
	if (!bCanUseSkill)
	{
		UE_LOG(LogTemp, Warning, TEXT("TryActivateSkill failed: bCanUseSkill = false"));
		return;
	}
	
	// IsSkillActive 는?
	if (SkillMap.Contains(Slot))
	{
		if (SkillMap[Slot]->CanActive()) // 지금 쿨다운 중이 아닌 경우 true
		{
			// 여기에서 검사해야 하네 지금 자신의 스킬은 사용할 수 있지만 이게 다른 스킬 도중에 호출된 건지는 알 수 없기 때문에

			// 여기에서 Control flag 들 검사. 
			AGS_Player* OwnerPlayer = Cast<AGS_Player>(GetOwner());
			if(OwnerPlayer)
			{
				if (IsSkillAllowed(Slot)) // 현재 내가 허용하고 있는 스킬인지 검색.
				{
					UE_LOG(LogTemp, Warning, TEXT("허용된 스킬이 Active 되기를 원한다."));
					
					SkillsInterrupt();
					SkillMap[Slot]->ActiveSkill();
					SetCurAllowedSkillsMask(SkillMap[Slot]->AllowSkillsMask);

					// 스킬 활성화 알림
					if (GetOwner()->GetLocalRole() == ROLE_Authority)
					{
						Client_BroadcastSkillActivation(Slot);
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Active 되기를 원하는 스킬이 불허되었다."));
				}
			} // SJE
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("CanActive() = false"));
			// 쿨타임 중이거나 사용할 수 없는 상태일 때 알림
			if (GetOwner()->GetLocalRole() == ROLE_Authority)
			{
				Client_BroadcastSkillCooldownBlocked(Slot);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillMap does not contain slot"));
	}
}

void UGS_SkillComp::Client_BroadcastSkillActivation_Implementation(ESkillSlot Slot)
{
	OnSkillActivated.Broadcast(Slot);
}

void UGS_SkillComp::Client_BroadcastSkillCooldownBlocked_Implementation(ESkillSlot Slot)
{
	OnSkillCooldownBlocked.Broadcast(Slot);
}

void UGS_SkillComp::Server_TryDeactiveSkill_Implementation(ESkillSlot Slot)
{	
	if (SkillMap.Contains(Slot))
	{
		SkillMap[Slot]->OnSkillCanceledByDebuff();
	}
}

void UGS_SkillComp::Server_TrySkillCommand_Implementation(ESkillSlot Slot)
{
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
			slot.Value->InterruptSkill(); // 모든 스킬 interruptSkill()
			/*if (Seeker->GetCharacterType() != ECharacterType::Merci)
			{
				//Seeker->SetSkillInputControl(false, false, false);
				
			}*/
		}
	}

	//Seeker->GetSkillComp()->ResetAllowedSkillsMask(); // 모든 입력 가능 상태.
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
			OnSkillActivated.AddDynamic(InSkillWidget, &UGS_SkillWidget::OnSkillActivated);
			OnSkillCooldownBlocked.AddDynamic(InSkillWidget, &UGS_SkillWidget::OnSkillCooldownBlocked);
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
