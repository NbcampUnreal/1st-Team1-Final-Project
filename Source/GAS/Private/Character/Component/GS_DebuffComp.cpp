// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Component/GS_DebuffComp.h"
#include "Character/Debuff/DebuffData.h"
#include "Character/GS_Character.h"
#include "Character/Debuff/GS_DebuffBase.h"


// Sets default values for this component's properties
UGS_DebuffComp::UGS_DebuffComp()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void UGS_DebuffComp::ApplyDebuff(EDebuffType Type)
{
	// 해당 디버프 타입의 데이터 가져오기
	const FDebuffData* Row = GetDebuffData(Type);
	if (!Row || !Row->DebuffClass) return;
	
	// 이미 적용중인 디버프라면
	if (UGS_DebuffBase* Existing = GetActiveDebuff(Type))
	{
		Existing->StartTime = GetWorld()->GetTimeSeconds(); // 시작 시간 재저장
		RefreshDebuffTimer(Existing, Row->Duration);
		return;
	}

	// 적용중이 아닌 디버프라면
	UGS_DebuffBase* NewDebuff = NewObject<UGS_DebuffBase>(this, Row->DebuffClass);
	NewDebuff->Initialize(Cast<AGS_Character>(GetOwner()), Row->Duration, Row->Priority, Type);
	
	// 우선순위와 관련 없다면
	if (Row->bIsConcurrent)
	{
		CreateAndApplyConcurrentDebuff(NewDebuff);
	}
	else // 우선순위가 관련있다면
	{
		AddDebuffToQueue(NewDebuff);
	}
}

bool UGS_DebuffComp::IsDebuffActive(EDebuffType Type)
{
	return GetActiveDebuff(Type) != nullptr;
}

const FDebuffData* UGS_DebuffComp::GetDebuffData(EDebuffType Type) const
{
	if (!DebuffDataTable) return nullptr;
	
	// 디버프 데이터 Row 반환
	FName RowName = *UEnum::GetValueAsString(Type).RightChop(13);
	return DebuffDataTable->FindRow<FDebuffData>(RowName, TEXT(""));
}

UGS_DebuffBase* UGS_DebuffComp::GetActiveDebuff(EDebuffType Type) const
{
	// ConcurrentDebuffs 내에 존재하는지 확인
	for (UGS_DebuffBase* Debuff : ConcurrentDebuffs)
	{
		if (Debuff && Debuff->GetDebuffType() == Type) return Debuff;
	}

	// CurrentDebuff로 존재하는지 확인
	if (CurrentDebuff && CurrentDebuff->GetDebuffType() == Type)
	{
		return CurrentDebuff;
	}
	
	// DebuffQueue 내에 존재하는지 확인
	for (UGS_DebuffBase* Debuff : DebuffQueue)
	{
		if (Debuff && Debuff->GetDebuffType() == Type) return Debuff;
	}

	// 존재하지 않음
	return nullptr;
}

void UGS_DebuffComp::RefreshDebuffTimer(UGS_DebuffBase* Debuff, float Duration)
{
	if (FTimerHandle* FoundHandle = DebuffTimers.Find(Debuff))
	{
		GetWorld()->GetTimerManager().ClearTimer(*FoundHandle);
		GetWorld()->GetTimerManager().SetTimer(*FoundHandle, [this, Debuff]()
			{
				Debuff->OnExpire();
				ConcurrentDebuffs.Remove(Debuff);
				DebuffTimers.Remove(Debuff);
			}, Duration, false);
	}
}

void UGS_DebuffComp::CreateAndApplyConcurrentDebuff(UGS_DebuffBase* Debuff)
{
	// ConcurrentDebuffs에 추가
	ConcurrentDebuffs.Add(Debuff);

	// Debuff 효과 실행
	Debuff->OnApply();

	// 타이머 설정
	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, [this, Debuff]()
		{
			Debuff->OnExpire();
			ConcurrentDebuffs.Remove(Debuff);
			DebuffTimers.Remove(Debuff);
		}, Debuff->GetDuration(), false);

	// 타이머 추가
	DebuffTimers.Add(Debuff, Handle);
}

void UGS_DebuffComp::AddDebuffToQueue(UGS_DebuffBase* Debuff)
{
	// DebuffQueue에 Debuff 추가
	DebuffQueue.Add(Debuff);

	// 우선순위에 따라 정렬
	DebuffQueue.Sort([](const UGS_DebuffBase& A, const UGS_DebuffBase& B)
		{
			return A.GetPriority() > B.GetPriority();
		});

	// 타이머 세팅
	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, [this, Debuff]()
		{
			DebuffQueue.Remove(Debuff);
			DebuffTimers.Remove(Debuff);
		}, Debuff->GetDuration(), false);

	DebuffTimers.Add(Debuff, Handle);

	// 현재 디버프가 없으면 바로 효과적용
	if (!CurrentDebuff)
	{
		ApplyNextDebuff();
	}
}

void UGS_DebuffComp::ApplyNextDebuff()
{
	// 디버프 큐에 없으면 리턴
	if (DebuffQueue.Num() == 0) return;

	// 현재 디버프 업데이트
	CurrentDebuff = DebuffQueue[0];
	DebuffQueue.RemoveAt(0);

	// 현재 디버프 효과 실행
	CurrentDebuff->OnApply();

	// 기존 타이머가 있으면 제거
	if (FTimerHandle* FoundHandle = DebuffTimers.Find(CurrentDebuff))
	{
		GetWorld()->GetTimerManager().ClearTimer(*FoundHandle);
	}

	// 새 타이머 핸들 생성
	FTimerHandle NewHandle;

	// 디버프 남은 시간 받기
	float Remaining = CurrentDebuff->GetRemainingTime(GetWorld()->GetTimeSeconds());

	// 남은 시간으로 다시 타이머 세팅
	GetWorld()->GetTimerManager().SetTimer(NewHandle, [this]()
		{
			CurrentDebuff->OnExpire();
			DebuffTimers.Remove(CurrentDebuff);
			CurrentDebuff = nullptr;
			ApplyNextDebuff();
		}, Remaining, false);

	DebuffTimers.Add(CurrentDebuff, NewHandle);
}
