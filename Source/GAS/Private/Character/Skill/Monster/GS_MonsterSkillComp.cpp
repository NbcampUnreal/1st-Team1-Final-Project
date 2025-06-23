#include "Character/Skill/Monster/GS_MonsterSkillComp.h"

#include "Character/Player/Monster/GS_Monster.h"
#include "Character/Skill/Monster/GS_MonsterSkillBase.h"


UGS_MonsterSkillComp::UGS_MonsterSkillComp()
{
	PrimaryComponentTick.bCanEverTick = false;

	bCanUseSkill = true;
}

void UGS_MonsterSkillComp::BeginPlay()
{
	Super::BeginPlay();

	SetIsReplicated(true);

	if (GetOwner()->HasAuthority())
	{
		SetSkill();
	}
}

void UGS_MonsterSkillComp::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UGS_MonsterSkillComp, CooldownRemaining);
	DOREPLIFETIME(UGS_MonsterSkillComp, SkillCooltime);
	DOREPLIFETIME(UGS_MonsterSkillComp, SkillDamage);
}

void UGS_MonsterSkillComp::SetSkill()
{
	if (!MonsterSkill)
	{
		return;
	}

	AGS_Monster* OwnerMonster = Cast<AGS_Monster>(GetOwner());
	if (!OwnerMonster)
	{
		return;
	}
	
	UGS_MonsterSkillBase* SkillInstance = NewObject<UGS_MonsterSkillBase>(this, MonsterSkill);
	if (SkillInstance)
	{
		SkillInstance->InitSkill(OwnerMonster);
		
		SkillCooltime = SkillInstance->Cooltime; 
		SkillDamage = SkillInstance->Damage;
	}
}

void UGS_MonsterSkillComp::SetCanUseSkill(bool InCanUseSkill)
{
	bCanUseSkill = InCanUseSkill;
}

void UGS_MonsterSkillComp::TryActivateSkill()
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}
	
	if (bIsOnCooldown)
	{
		UE_LOG(LogTemp, Warning, TEXT("MonsterSkillComp: Skill is on cooldown (%.1f remaining)"), CooldownRemaining);
		return;
	}

	if (!bCanUseSkill)
	{
		UE_LOG(LogTemp, Warning, TEXT("TryActivateSkill failed: bCanUseSkill = false"));
		return;
	}
	
	if (!MonsterSkill)
	{
		UE_LOG(LogTemp, Error, TEXT("MonsterSkillComp: MonsterSkillClass not set"));
		return;
	}
	
	UGS_MonsterSkillBase* SkillInstance = NewObject<UGS_MonsterSkillBase>(this, MonsterSkill);
	if (SkillInstance)
	{
		AGS_Monster* OwnerMonster = Cast<AGS_Monster>(GetOwner());
		SkillInstance->InitSkill(OwnerMonster);
		SkillInstance->ActiveSkill();
		
		StartCooldown(SkillCooltime);
	}
}

void UGS_MonsterSkillComp::StartCooldown(float CooldownTime)
{
	UE_LOG(LogTemp, Warning, TEXT("MonsterSkillComp: StartCooldown %.1f"), CooldownTime);
	
	if (CooldownTime <= 0.0f)
	{
		return;
	}

	bIsOnCooldown = true;
	CooldownRemaining = CooldownTime;
	
	// 쿨다운 타이머 
	TWeakObjectPtr<UGS_MonsterSkillComp> WeakThis = this;
	GetWorld()->GetTimerManager().SetTimer(
		CooldownTimer,
		[WeakThis]()
		{
			if (WeakThis.IsValid())
			{
				WeakThis->CooldownFinished();
			}
		},
		CooldownTime,
		false
	);

	// UI 타이머
	GetWorld()->GetTimerManager().SetTimer(
		UIUpdateTimer,
		[WeakThis]()
		{
			if (WeakThis.IsValid())
			{
				WeakThis->BroadcastCooldownUpdate();
			}
		},
		1.0f,
		true
	);

	BroadcastCooldownUpdate();
}

void UGS_MonsterSkillComp::CooldownFinished()
{
	bIsOnCooldown = false;
	CooldownRemaining = 0.0f;
	GetWorld()->GetTimerManager().ClearTimer(UIUpdateTimer); 
	BroadcastCooldownUpdate();
}

void UGS_MonsterSkillComp::BroadcastCooldownUpdate()
{
	CooldownRemaining = GetWorld()->GetTimerManager().GetTimerRemaining(CooldownTimer);
	CooldownRemaining = FMath::Max(0.0f, CooldownRemaining); 
}

void UGS_MonsterSkillComp::OnRep_CooldownRemaining()
{
	if (!GetOwner()->HasAuthority())
	{
		OnMonsterSkillCooldownChanged.Broadcast(CooldownRemaining, SkillCooltime);
		UE_LOG(LogTemp, Log, TEXT("UGS_MonsterSkillComp::OnRep_CooldownRemaining : %.1f / %.1f"), CooldownRemaining, SkillCooltime);
	}
}

void UGS_MonsterSkillComp::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(CooldownTimer);
		GetWorld()->GetTimerManager().ClearTimer(UIUpdateTimer);
	}
}
