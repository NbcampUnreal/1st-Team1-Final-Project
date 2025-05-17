// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Component/GS_DebuffComp.h"
#include "Character/Debuff/DebuffData.h"
#include "Character/GS_Character.h"
#include "Character/Debuff/GS_DebuffBase.h"
#include "Net/UnrealNetwork.h"


UGS_DebuffComp::UGS_DebuffComp()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UGS_DebuffComp::ApplyDebuff(EDebuffType Type, AGS_Character* Attacker)
{
	UE_LOG(LogTemp, Warning, TEXT("Apply Debuff"));
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	// 해당 디버프 타입의 데이터 가져오기
	const FDebuffData* Row = GetDebuffData(Type);
	if (!Row || !Row->DebuffClass) return;
	
	// 이미 적용중인 디버프라면
	UGS_DebuffBase* Existing = GetActiveDebuff(Type);

	if (Existing)
	{
		Existing->StartTime = GetWorld()->GetTimeSeconds(); // 시작 시간 재저장
		RefreshDebuffTimer(Existing, Row->Duration);
		UpdateReplicatedDebuffList(); // 복제 정보 갱신
		return;
	}

	// 적용중이 아닌 디버프라면
	UGS_DebuffBase* NewDebuff = NewObject<UGS_DebuffBase>(this, Row->DebuffClass);
	NewDebuff->Initialize(Cast<AGS_Character>(GetOwner()), Attacker, Row->Duration, Row->Priority, Type);
	NewDebuff->StartTime = GetWorld()->GetTimeSeconds();

	// 우선순위와 관련 없다면
	if (Row->bIsConcurrent)
	{
		CreateAndApplyConcurrentDebuff(NewDebuff);
	}
	else // 우선순위가 관련있다면
	{
		AddDebuffToQueue(NewDebuff);
	}
	UpdateReplicatedDebuffList(); // 복제 정보 갱신
}

bool UGS_DebuffComp::IsDebuffActive(EDebuffType Type)
{
	return GetActiveDebuff(Type) != nullptr;
}

void UGS_DebuffComp::OnRep_DebuffList()
{
	// TODO : 클라이언트 UI 갱신 로직 들어가는 곳
}

const FDebuffData* UGS_DebuffComp::GetDebuffData(EDebuffType Type) const
{
	if (!DebuffDataTable) return nullptr;
	
	// 디버프 데이터 Row 반환
	FName RowName = *UEnum::GetValueAsString(Type).RightChop(13);
	UE_LOG(LogTemp, Warning, TEXT("Get Debuff Data"));
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
				UpdateReplicatedDebuffList();
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
			UpdateReplicatedDebuffList();
		}, Debuff->GetDuration(), false);

	// 타이머 추가
	DebuffTimers.Add(Debuff, Handle);
	UE_LOG(LogTemp, Warning, TEXT("Create And Apply Concurrent Debuff"));
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

	UE_LOG(LogTemp, Warning, TEXT("Add Debuff Queue"));
}

void UGS_DebuffComp::ApplyNextDebuff()
{
	UE_LOG(LogTemp, Warning, TEXT("ApplyNextDebuff"));
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
			UpdateReplicatedDebuffList();
			ApplyNextDebuff();
		}, Remaining, false);

	DebuffTimers.Add(CurrentDebuff, NewHandle);
}

void UGS_DebuffComp::UpdateReplicatedDebuffList()
{
	ReplicatedDebuffs.Empty();
	float Now = GetWorld()->GetTimeSeconds();

	// Concurrent 디버프 추가
	for (UGS_DebuffBase* Debuff : ConcurrentDebuffs)
	{
		if (!Debuff) continue;
		FDebuffRepInfo Info;
		Info.Type = Debuff->GetDebuffType();
		Info.RemainingTime = Debuff->GetRemainingTime(Now);
		ReplicatedDebuffs.Add(Info);
	}

	// CurrentDebuff 디버프도 추가
	if (CurrentDebuff)
	{
		FDebuffRepInfo Info;
		Info.Type = CurrentDebuff->GetDebuffType();
		Info.RemainingTime = CurrentDebuff->GetRemainingTime(Now);
		ReplicatedDebuffs.Add(Info);
	}

	// DebuffQueue의 디버프 추가
	for (UGS_DebuffBase* Debuff : DebuffQueue)
	{
		if (!Debuff) continue;
		FDebuffRepInfo Info;
		Info.Type = Debuff->GetDebuffType();
		Info.RemainingTime = Debuff->GetRemainingTime(Now);
		ReplicatedDebuffs.Add(Info);
	}

	if (GetNetMode() != NM_DedicatedServer)
	{
		OnRep_DebuffList();
	}
}

void UGS_DebuffComp::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGS_DebuffComp, ReplicatedDebuffs);
}
