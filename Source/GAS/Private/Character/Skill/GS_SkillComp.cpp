// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/GS_SkillComp.h"
#include "Character/Player/GS_Player.h"
#include "Character/Skill/GS_SkillBase.h"
#include "Character/Skill/GS_SkillSet.h"
#include "UI/Character/GS_SkillWidget.h"
#include "Net/UnrealNetwork.h"
#include "Character/Player/Seeker/GS_Seeker.h"
#include "Character/E_Character.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"


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
	uint16 BitFlag = 0;
	BitFlag |= (1 << static_cast<int32>(CompareSkillType));
	UE_LOG(LogTemp, Warning, TEXT("UGS_SkillComp::IsSkillAllowed BitFlag :%d"), BitFlag);
	UE_LOG(LogTemp, Warning, TEXT("UGS_SkillComp::IsSkillAllowed CurAllowedSkillsMask :%d"), CurAllowedSkillsMask);
	return CurAllowedSkillsMask & BitFlag;
}

void UGS_SkillComp::SetCurAllowedSkillsMask(int16 BitMask)
{
	CurAllowedSkillsMask = BitMask;
}

int16 UGS_SkillComp::GetCurAllowedSkillsMask()
{
	return CurAllowedSkillsMask;
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
		SetSkill(ESkillSlot::Combo, SkillSet->ComboSkill);
		SetSkill(ESkillSlot::HealPotion, SkillSet->HealPotionSkill);
	}
}

void UGS_SkillComp::ResetAllowedSkillsMask()
{
	UE_LOG(LogTemp, Warning, TEXT("ResetAllowedSkillsMask"));
	CurAllowedSkillsMask = DefaultAllowedSkillsMask;
}

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
	}
	
	Skill-> AllowControlValue = Info.AllowControlValue;
	
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

	// Init Delegate
	Skill->InitializeDelegate();
}

void UGS_SkillComp::Server_TryActivateSkill_Implementation(ESkillSlot Slot)
{
	if (!bCanUseSkill)
	{
		UE_LOG(LogTemp, Warning, TEXT("TryActivateSkill failed: bCanUseSkill = false"));
		return;
	}
	
	if (SkillMap.Contains(Slot))
	{
		if (SkillMap[Slot]->CanActive())
		{
			AGS_Player* OwnerPlayer = Cast<AGS_Player>(GetOwner());
			if(OwnerPlayer)
			{
				if (IsSkillAllowed(Slot))
				{
					UE_LOG(LogTemp, Warning, TEXT("허용된 스킬이 Active 되기를 원한다."));
					
					SkillsInterrupt();
					SkillMap[Slot]->ActiveSkill();
					UE_LOG(LogTemp, Warning, TEXT("AllowSkillMask : %d"), SkillMap[Slot]->AllowSkillsMask);
					ResetAllowedSkillsMask();
					SetCurAllowedSkillsMask(SkillMap[Slot]->AllowSkillsMask);
					UE_LOG(LogTemp, Warning, TEXT("CurAllowedSkillsMask : %d"),GetCurAllowedSkillsMask());

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
			}
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

void UGS_SkillComp::Client_BroadcastHealCountChanged_Implementation(ESkillSlot Slot, int32 CurrentCount, int32 MaxCount)
{
	OnHealCountChanged.Broadcast(Slot, CurrentCount, MaxCount);
}

void UGS_SkillComp::Client_BroadcastSkillCooldownBlocked_Implementation(ESkillSlot Slot)
{
	OnSkillCooldownBlocked.Broadcast(Slot);
}

void UGS_SkillComp::Server_TryDeactiveSkill_Implementation(ESkillSlot Slot)
{
	if (SkillMap.Contains(Slot))
	{
		SkillMap[Slot]->DeactiveSkill();
	}
}

void UGS_SkillComp::Server_TrySkillCanceledByDebuff_Implementation(ESkillSlot Slot)
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

UGS_SkillBase* UGS_SkillComp::GetActiveSkill() const
{
	for (const auto& Pair : SkillStates)
	{
		if (Pair.Value.bIsActive)
		{
			if (UGS_SkillBase* const* SkillPtr = SkillMap.Find(Pair.Key))
			{
				return *SkillPtr;
			}
		}
	}

	return nullptr;
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
		//UE_LOG(LogTemp, Warning, TEXT("%s"), *slot.Value->GetName());

		if (Seeker->GetSkillComp()->IsSkillActive(slot.Key))
		{
			slot.Value->InterruptSkill();
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
			OnHealCountChanged.AddUObject(InSkillWidget, &UGS_SkillWidget::OnHealCountChanged);
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

void UGS_SkillComp::Multicast_PlayLoopVFX_Implementation(ESkillSlot Slot, AActor* AttachTarget)
{
	// 데디케이티드 서버에서는 VFX 재생하지 않음
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	// 유효성 검사
	if (!AttachTarget || !IsValid(AttachTarget))
	{
		UE_LOG(LogTemp, Error, TEXT("[SkillComp] Multicast_PlayLoopVFX: AttachTarget이 유효하지 않습니다!"));
		return;
	}

	// SkillInfo에서 LoopVFX 가져오기
	UGS_SkillBase* Skill = GetSkillFromSkillMap(Slot);
	if (!Skill)
	{
		UE_LOG(LogTemp, Error, TEXT("[SkillComp] Multicast_PlayLoopVFX: Skill을 찾을 수 없습니다!"));
		return;
	}

	const FSkillInfo* SkillInfo = Skill->GetCurrentSkillInfo();
	if (!SkillInfo || !SkillInfo->SkillLoopVFX)
	{
		UE_LOG(LogTemp, Error, TEXT("[SkillComp] Multicast_PlayLoopVFX: SkillLoopVFX가 DT_SkillSet에 할당되지 않았습니다!"));
		return;
	}

	// 기존 Loop VFX가 있으면 먼저 정리
	Multicast_StopLoopVFX(Slot);

	// 새로운 Loop VFX 생성 (AttachTarget에 부착)
	UNiagaraComponent* LoopVFXComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		SkillInfo->SkillLoopVFX,
		AttachTarget->GetRootComponent(),
		NAME_None,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		EAttachLocation::KeepRelativeOffset,
		true  // bAutoDestroy
	);

	if (LoopVFXComponent)
	{
		ActiveLoopVFXComponents.Add(Slot, LoopVFXComponent);
		UE_LOG(LogTemp, Warning, TEXT("[SkillComp] Loop VFX 생성 성공: %s"), *SkillInfo->SkillLoopVFX->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[SkillComp] Loop VFX 컴포넌트 생성 실패!"));
	}
}

void UGS_SkillComp::Multicast_StopLoopVFX_Implementation(ESkillSlot Slot)
{
	// 데디케이티드 서버에서는 처리하지 않음
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	// 해당 슬롯의 Loop VFX 컴포넌트 찾기
	if (UNiagaraComponent** FoundComponent = ActiveLoopVFXComponents.Find(Slot))
	{
		if (*FoundComponent && IsValid(*FoundComponent))
		{
			(*FoundComponent)->Deactivate();
			(*FoundComponent)->DestroyComponent();
			UE_LOG(LogTemp, Warning, TEXT("[SkillComp] Loop VFX 정지 성공"));
		}
		ActiveLoopVFXComponents.Remove(Slot);
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
