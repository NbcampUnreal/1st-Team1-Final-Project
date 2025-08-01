// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Component/GS_DebuffComp.h"
#include "Character/Debuff/DebuffData.h"
#include "Character/GS_Character.h"
#include "Character/Debuff/GS_DebuffBase.h"
#include "Net/UnrealNetwork.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "Character/Component/GS_DebuffVFXComponent.h"


UGS_DebuffComp::UGS_DebuffComp()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UGS_DebuffComp::ApplyDebuff(EDebuffType Type, AActor* Attacker)
{
	if (!GetOwner()->HasAuthority())
	{
		Server_ApplyDebuff(Type, Attacker);
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

		
		// ===============================
		// VFX 트리거 (기존 디버프 갱신 시에도)
		// ===============================
		TriggerDebuffVFX(Type);
		
		return;
	}

	// 적용중이 아닌 디버프라면
	UGS_DebuffBase* NewDebuff = NewObject<UGS_DebuffBase>(this, Row->DebuffClass);
	NewDebuff->Initialize(Cast<AGS_Character>(GetOwner()), Attacker, Row->Duration, Row->Priority, Row->Damage, Row->DamageInterval, Type);
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
	
	// ===============================
	// VFX 트리거 (새로운 디버프 적용 시)
	// ===============================
	TriggerDebuffVFX(Type);
}

void UGS_DebuffComp::RemoveDebuff(EDebuffType Type)
{
	if (!GetOwner()->HasAuthority())
	{
		Server_RemoveDebuff(Type);
		return;
	}



	UGS_DebuffBase* Debuff = GetActiveDebuff(Type);
	if (!Debuff)
	{
		return;
	}

	if (FTimerHandle* Handle = DebuffTimers.Find(Debuff))
	{
		GetWorld()->GetTimerManager().ClearTimer(*Handle);
		DebuffTimers.Remove(Debuff);
	}

	if (ConcurrentDebuffs.Contains(Debuff))
	{
		Debuff->OnExpire();
		ConcurrentDebuffs.Remove(Debuff);
	}
	else if (DebuffQueue.Contains(Debuff))
	{
		Debuff->OnExpire();
		DebuffQueue.Remove(Debuff);
	}
	else if (Debuff == CurrentDebuff)
	{
		CurrentDebuff->OnExpire();
		CurrentDebuff = nullptr;
		ApplyNextDebuff();
	}

	UpdateReplicatedDebuffList();
}

bool UGS_DebuffComp::IsDebuffActive(EDebuffType Type)
{
	return GetActiveDebuff(Type) != nullptr;
}

void UGS_DebuffComp::OnRep_DebuffList()
{
	OnDebuffListUpdated.Broadcast(ReplicatedDebuffs);
}

void UGS_DebuffComp::ClearAllDebuffs()
{
	if (!GetOwner()->HasAuthority())
	{
		Server_ClearAllDebuffs();
		return;
	}



	// 타이머 먼저 제거
	for (auto& Elem : DebuffTimers)
	{
		GetWorld()->GetTimerManager().ClearTimer(Elem.Value);
	}

	// 이후 안전하게 OnExpire 호출
	for (auto& Elem : DebuffTimers)
	{
		if (Elem.Key && IsValid(Elem.Key))
		{
			Elem.Key->OnExpire();
		}
	}
	DebuffTimers.Empty();

	// 모든 디버프 컨테이너 초기화
	ConcurrentDebuffs.Empty();
	DebuffQueue.Empty();
	CurrentDebuff = nullptr;

	// 복제 목록 갱신
	UpdateReplicatedDebuffList();
}

void UGS_DebuffComp::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	ClearAllDebuffs();
}

const FDebuffData* UGS_DebuffComp::GetDebuffData(EDebuffType Type) const
{
	if (!DebuffDataTable) return nullptr;

	// 디버프 데이터 Row 반환
	FName RowName = *UEnum::GetValueAsString(Type).RightChop(13);
	/*UE_LOG(LogTemp, Warning, TEXT("Get Debuff Data"));*/
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

		TWeakObjectPtr<UGS_DebuffBase> WeakDebuff(Debuff);

		GetWorld()->GetTimerManager().SetTimer(*FoundHandle, [this, WeakDebuff]()
			{
				if (!IsValid(WeakDebuff.Get())) return;
				UGS_DebuffBase* ValidDebuff = WeakDebuff.Get();
				
				// ===============================
				// 디버프 만료 VFX 재생
				// ===============================
				TriggerDebuffExpireVFX(ValidDebuff->GetDebuffType());
				
				ValidDebuff->OnExpire();
				DebuffTimers.Remove(ValidDebuff);
				if (ConcurrentDebuffs.Contains(ValidDebuff))
				{
					ConcurrentDebuffs.Remove(ValidDebuff);
				}
				else if (DebuffQueue.Contains(ValidDebuff))
				{
					DebuffQueue.Remove(ValidDebuff);
				}
				else if (ValidDebuff == CurrentDebuff)
				{
					CurrentDebuff = nullptr;
					ApplyNextDebuff();
				}
				UpdateReplicatedDebuffList();
			}, Duration, false);
	}
}

void UGS_DebuffComp::CreateAndApplyConcurrentDebuff(UGS_DebuffBase* Debuff)
{
	ConcurrentDebuffs.Add(Debuff);
	Debuff->OnApply();

	FTimerHandle Handle;
	TWeakObjectPtr<UGS_DebuffBase> WeakDebuff(Debuff);

	GetWorld()->GetTimerManager().SetTimer(Handle, [this, WeakDebuff]()
		{
			if (!IsValid(WeakDebuff.Get())) return;
			UGS_DebuffBase* ValidDebuff = WeakDebuff.Get();
			
			// ===============================
			// 디버프 만료 VFX 재생 (Concurrent 디버프용)
			// ===============================
			TriggerDebuffExpireVFX(ValidDebuff->GetDebuffType());
			
			ValidDebuff->OnExpire();
			ConcurrentDebuffs.Remove(ValidDebuff);
			DebuffTimers.Remove(ValidDebuff);
			UpdateReplicatedDebuffList();
		}, Debuff->GetDuration(), false);

	DebuffTimers.Add(Debuff, Handle);
}

void UGS_DebuffComp::AddDebuffToQueue(UGS_DebuffBase* Debuff)
{
	// 현재 디버프가 없으면 단순 큐 처리
	if (CurrentDebuff)
	{
		if (Debuff->GetPriority() > CurrentDebuff->GetPriority())
		{
	
			// 현재 디버프의 타이머 제거
			if (FTimerHandle* FoundHandle = DebuffTimers.Find(CurrentDebuff))
			{
				GetWorld()->GetTimerManager().ClearTimer(*FoundHandle);
				DebuffTimers.Remove(CurrentDebuff);
			}

			CurrentDebuff->OnExpire();

			// 현재 디버프의 남은 시간 저장
			float RemainingTime = CurrentDebuff->GetRemainingTime(GetWorld()->GetTimeSeconds());

			// 큐로 재삽입
			DebuffQueue.Add(CurrentDebuff);

			// 타이머 재설정
			FTimerHandle NewHandle;
			TWeakObjectPtr<UGS_DebuffBase> WeakDebuff(CurrentDebuff);
			GetWorld()->GetTimerManager().SetTimer(NewHandle, [this, WeakDebuff]()
				{
					if (!IsValid(WeakDebuff.Get())) return;
					UGS_DebuffBase* ValidDebuff = WeakDebuff.Get();
					DebuffQueue.Remove(ValidDebuff);
					DebuffTimers.Remove(ValidDebuff);
					UpdateReplicatedDebuffList();
				}, RemainingTime, false);

			DebuffTimers.Add(CurrentDebuff, NewHandle);
			CurrentDebuff = nullptr;
			UpdateReplicatedDebuffList();
		}
	}

	DebuffQueue.Add(Debuff);

	// 우선순위 정렬
	DebuffQueue.Sort([](const UGS_DebuffBase& A, const UGS_DebuffBase& B)
		{
			return A.GetPriority() > B.GetPriority();
		});

	// 디버프 만료 타이머 설정
	FTimerHandle Handle;
	TWeakObjectPtr<UGS_DebuffBase> WeakDebuff(Debuff);
	GetWorld()->GetTimerManager().SetTimer(Handle, [this, WeakDebuff]()
		{
			if (!IsValid(WeakDebuff.Get())) return;
			UGS_DebuffBase* ValidDebuff = WeakDebuff.Get();
			
			// ===============================
			// 디버프 만료 VFX 재생 (Queue 디버프용)
			// ===============================
			TriggerDebuffExpireVFX(ValidDebuff->GetDebuffType());
			
			DebuffQueue.Remove(ValidDebuff);
			DebuffTimers.Remove(ValidDebuff);
			UpdateReplicatedDebuffList();
		}, Debuff->GetDuration(), false);

	DebuffTimers.Add(Debuff, Handle);

	// 현재 디버프가 없는 경우에만 다음 디버프 적용
	if (!CurrentDebuff)
	{
		ApplyNextDebuff();
	}
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
	TWeakObjectPtr<UGS_DebuffBase> WeakDebuff(CurrentDebuff);
	// 디버프 남은 시간 받기
	float Remaining = CurrentDebuff->GetRemainingTime(GetWorld()->GetTimeSeconds());

	// 남은 시간으로 다시 타이머 세팅
	GetWorld()->GetTimerManager().SetTimer(NewHandle, [this, WeakDebuff]()
		{
			if (!IsValid(WeakDebuff.Get())) return;
			UGS_DebuffBase* ValidDebuff = WeakDebuff.Get();
			
			// ===============================
			// 디버프 만료 VFX 재생 (Current 디버프용)
			// ===============================
			TriggerDebuffExpireVFX(ValidDebuff->GetDebuffType());
			
			ValidDebuff->OnExpire();
			DebuffTimers.Remove(ValidDebuff);
			if (CurrentDebuff == ValidDebuff)
			{
				CurrentDebuff = nullptr;
			}
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
}

void UGS_DebuffComp::Server_ApplyDebuff_Implementation(EDebuffType Type, AActor* Attacker)
{
	ApplyDebuff(Type, Attacker);
}

void UGS_DebuffComp::Server_RemoveDebuff_Implementation(EDebuffType Type)
{
	RemoveDebuff(Type);
}

void UGS_DebuffComp::Server_ClearAllDebuffs_Implementation()
{
	ClearAllDebuffs();
}

void UGS_DebuffComp::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGS_DebuffComp, ReplicatedDebuffs);
}

void UGS_DebuffComp::TriggerDebuffVFX(EDebuffType Type)
{
	// 서버에서만 실행
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	// =======================
	// DebuffVFX 컴포넌트
	// =======================
	if (UGS_DebuffVFXComponent* VFXComponent = GetOwner()->FindComponentByClass<UGS_DebuffVFXComponent>())
	{
		VFXComponent->PlayDebuffVFX(Type);
	}
}

void UGS_DebuffComp::TriggerDebuffExpireVFX(EDebuffType Type)
{
	// 서버에서만 실행
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	// =======================
	// DebuffVFX 컴포넌트 - 만료 VFX
	// =======================
	if (UGS_DebuffVFXComponent* VFXComponent = GetOwner()->FindComponentByClass<UGS_DebuffVFXComponent>())
	{
		VFXComponent->PlayDebuffExpireVFX(Type);
	}
}
