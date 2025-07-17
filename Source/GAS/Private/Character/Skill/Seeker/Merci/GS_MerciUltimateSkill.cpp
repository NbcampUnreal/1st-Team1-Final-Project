// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Skill/Seeker/Merci/GS_MerciUltimateSkill.h"
#include "Character/Player/Seeker/GS_Merci.h"
#include "DrawDebugHelpers.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "Character/Player/Guardian/GS_Guardian.h"
#include "Kismet/GameplayStatics.h"

UGS_MerciUltimateSkill::UGS_MerciUltimateSkill()
{
	CurrentSkillType = ESkillSlot::Ultimate;
}

void UGS_MerciUltimateSkill::ActiveSkill()
{
	if (!CanActive()) 
	{
		return;
	}

	// 스킬 상태 업데이트
	Super::ActiveSkill();

	// 쿨타임 측정 시작
	StartCoolDown();

	if(OwnerCharacter)
	{
		OwnerCharacter->Server_SetCanHitReact(false); // 서버에 전달
		// 스킬 시작 사운드 재생
		const FSkillInfo* SkillInfo = GetCurrentSkillInfo();
		if (AGS_Seeker* OwnerPlayer = Cast<AGS_Seeker>(OwnerCharacter))
		{
			if (SkillInfo && SkillInfo->SkillStartSound)
			{
				OwnerPlayer->Multicast_PlaySkillSound(SkillInfo->SkillStartSound);
			}
		}

		AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter);
		if (MerciCharacter)
		{
			MerciCharacter->Client_StartZoom();
		}

		OwnerCharacter->GetWorldTimerManager().SetTimer(AutoAimingHandle, this, &UGS_MerciUltimateSkill::DeactiveSkill, AutoAimingStateTime, false);
		OwnerCharacter->GetWorldTimerManager().SetTimer(AutoAimTickHandle, this, &UGS_MerciUltimateSkill::TickAutoAimTarget, AutoAimTickInterval, true);
		OwnerCharacter->SetSkillInputControl(true, true, false, false);
	}

	UpdateMonsterList();
	AutoAimingStart();
}

void UGS_MerciUltimateSkill::OnSkillAnimationEnd()
{
}

void UGS_MerciUltimateSkill::InterruptSkill()
{
	Super::InterruptSkill();

	AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter);
	if (MerciCharacter->GetSkillComp())
	{
		//MerciCharacter->GetSkillComp()->SetSkillActiveState(ESkillSlot::Ultimate, false);
	}
}

void UGS_MerciUltimateSkill::AutoAimingStart()
{
	AActor* Target = FindCloseTarget();
	AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter);
	if (Target && OwnerCharacter->HasAuthority())
	{
		if (MerciCharacter)
		{
			MerciCharacter->SetAutoAimTarget(Target);
		}
	}
}

AActor* UGS_MerciUltimateSkill::FindCloseTarget()
{
	if (!OwnerCharacter) return nullptr;
	
	AController* Controller = OwnerCharacter->GetController();
	if (!Controller) return nullptr;

	FVector CamLoc;
	FRotator CamRot;
	Controller->GetPlayerViewPoint(CamLoc, CamRot);
	FVector ViewDir = CamRot.Vector();

	float CloseDot = 0.8f; // 최소 허용 Dot
	AActor* BestTarget = nullptr;

	for (AActor* Target : AllMonsterActors)
	{
		if (!IsValid(Target)) continue;

		// 죽은 타겟은 건너뛰기
		if (AGS_Character* CharacterTarget = Cast<AGS_Character>(Target))
		{
			if (CharacterTarget->IsDead()) continue;
		}

		FVector ToTarget = (Target->GetActorLocation() - CamLoc).GetSafeNormal();
		float Dot = FVector::DotProduct(ViewDir, ToTarget);

		if (Dot > CloseDot)
		{
			// LineTrace로 시야 확인
			FHitResult Hit;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(OwnerCharacter);

			bool bHit = OwnerCharacter->GetWorld()->LineTraceSingleByChannel(
				Hit,
				CamLoc,
				Target->GetActorLocation(),
				ECC_Visibility,
				Params
			);

			// 벽에 가려져 있다면 무시
			if (bHit && Hit.GetActor() != Target)
			{
				continue;
			}

			// 타겟 선택
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

	if (!GetIsActive())
	{
		UE_LOG(LogTemp, Warning, TEXT("IsActive is false"));
		return;
	}

	AActor* NewTarget = FindCloseTarget();

	if (OwnerCharacter->HasAuthority())
	{
		// 화살에 타겟 전달
		if (AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter))
		{
			MerciCharacter->SetAutoAimTarget(NewTarget);

			if (NewTarget != CurrentTarget)
			{
				// 플레이어를 통해 클라이언트로 전송
				MerciCharacter->Client_UpdateTargetUI(NewTarget, CurrentTarget);
				CurrentTarget = NewTarget;
			}
		}
	}
}

void UGS_MerciUltimateSkill::UpdateMonsterList()
{
	AllMonsterActors.Empty();

	TArray<AActor*> FoundActors;

	// AGS_Monster 찾기
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGS_Monster::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		if (IsValid(Actor)) AllMonsterActors.Add(Actor);
	}

	FoundActors.Empty(); // 배열 재사용

	// AGS_Guardian 찾기
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGS_Guardian::StaticClass(), FoundActors);
	for (AActor* Actor : FoundActors)
	{
		if (IsValid(Actor)) AllMonsterActors.Add(Actor);
	}
}

void UGS_MerciUltimateSkill::DeactiveSkill()
{
	if (OwnerCharacter)
	{
		// 현재 표시된 타겟 UI 정리
		if (CurrentTarget && OwnerCharacter->HasAuthority())
		{
			if (AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter))
			{
				MerciCharacter->Client_UpdateTargetUI(nullptr, CurrentTarget);
			}
			CurrentTarget = nullptr;
		}

		// 줌 아웃
		AGS_Merci* MerciCharacter = Cast<AGS_Merci>(OwnerCharacter);
		if (MerciCharacter)
		{
			MerciCharacter->Client_StopZoom();
		}

		// 타이머 정리
		OwnerCharacter->GetWorldTimerManager().ClearTimer(AutoAimTickHandle);
		OwnerCharacter->GetWorldTimerManager().ClearTimer(AutoAimingHandle);

		// 스킬 Input 수정
		OwnerCharacter->SetSkillInputControl(true, true, true);
		OwnerCharacter->Server_SetCanHitReact(true); // 서버에 전달
	}

	Super::DeactiveSkill();
}
