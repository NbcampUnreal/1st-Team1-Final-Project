// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Character/GS_ChanAimingSkillBar.h"
#include "Components/ProgressBar.h"
#include "Character/Player/Seeker/GS_Chan.h"
#include "Kismet/KismetMathLibrary.h"

void UGS_ChanAimingSkillBar::NativeConstruct()
{
	Super::NativeConstruct();
	OwningCharacter = Cast<AGS_Character>(GetOwningPlayer()->GetPawn());
	AGS_Chan* ChanCharacter = Cast<AGS_Chan>(OwningCharacter);
	if(ChanCharacter)
	{
		ChanCharacter->SetChanAimingSkillBarWidget(this);
	}
	ShowSkillBar(true);

	TargetPercent = 1.0f;
	DelayedPercent = 1.0f;
	
	// 초기화
	if (AimingProgressBar)
	{
		AimingProgressBar->SetPercent(1.0f);
	}
	if (AimingProgressBar_back)
	{
		AimingProgressBar_back->SetPercent(1.0f);
	}
}

// void UGS_ChanAimingSkillBar::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
// {
// 	Super::NativeTick(MyGeometry, InDeltaTime);
//
// 	if (AimingProgressBar_back)
// 	{
// 		float Current = AimingProgressBar_back->GetPercent();
// 		float NewValue = FMath::FInterpTo(Current, TargetBackPercent, InDeltaTime, BackBarInterpSpeed);
// 		AimingProgressBar_back->SetPercent(NewValue);
// 	}
// }

void UGS_ChanAimingSkillBar::SetOwningActor(AGS_Character* InOwningCharacter)
{
	OwningCharacter = InOwningCharacter;
}

void UGS_ChanAimingSkillBar::SetAimingProgress(float Progress)
{
	// Progress = FMath::Clamp(Progress, 0.0f, 1.0f);
	//
	// // 시간 소모 → 앞뒤 둘 다 즉시
	// if (AimingProgressBar)
	// 	AimingProgressBar->SetPercent(Progress);
	// if (AimingProgressBar_back)
	// 	AimingProgressBar_back->SetPercent(Progress);
	//
	// TargetBackPercent = Progress; // 싱크 맞춰줌



	
	// Progress = FMath::Clamp(Progress, 0.0f, 1.0f);
	//
	// // 보간 중일 수 있으므로 타이머 정리
	// if (GetWorld())
	// {
	// 	GetWorld()->GetTimerManager().ClearTimer(InterpTimerHandle);
	// 	GetWorld()->GetTimerManager().ClearTimer(DelayBeforeInterpTimerHandle);
	// }
	//
	// TargetPercent = Progress;
	// DelayedPercent = Progress; // 싱크 맞춤
	//
	// // 시간 소모 → 앞뒤 둘 다 즉시
	// if (AimingProgressBar)
	// 	AimingProgressBar->SetPercent(TargetPercent);
	// if (AimingProgressBar_back)
	// 	AimingProgressBar_back->SetPercent(DelayedPercent);


	// 1. 목표값이 아주 약간만 변했거나 같다면, 불필요한 로직을 실행하지 않고 앞바만 업데이트
	// (참고: Client_Update...가 매 프레임 호출된다면 이 검사가 매우 중요합니다)
	if (FMath::IsNearlyEqual(TargetPercent, Progress))
	{
		// 혹시 모르니 앞바가 목표값과 다른지만 확인
		if (AimingProgressBar && AimingProgressBar->GetPercent() != TargetPercent)
		{
			AimingProgressBar->SetPercent(TargetPercent);
		}
		return; // 이미 같은 목표로 보간 중이거나 목표에 도달했으므로 종료
	}

	// 2. 새로운 목표값 설정 및 앞바 즉시 업데이트
	TargetPercent = Progress;
	if (AimingProgressBar)
		AimingProgressBar->SetPercent(TargetPercent);

	// 3. [중요] 함수 시작 부분의 타이머 클리어 로직 제거!
	// if (GetWorld())
	// {
	// 	GetWorld()->GetTimerManager().ClearTimer(InterpTimerHandle);
	// 	GetWorld()->GetTimerManager().ClearTimer(DelayBeforeInterpTimerHandle);
	// }

	// 4. 게이지 회복 로직 (뒤바가 앞바보다 작을 때)
	if (DelayedPercent < TargetPercent)
	{
		// 게이지가 회복되는 경우는 즉시 뒤바를 앞바로 맞춥니다.
		DelayedPercent = TargetPercent;
		if (AimingProgressBar_back)
		{
			AimingProgressBar_back->SetPercent(DelayedPercent);
		}
		
		// 회복 시에는, 혹시 (감소 중이던) 보간 타이머가 돌고 있었다면 정지시킵니다.
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(InterpTimerHandle);
			GetWorld()->GetTimerManager().ClearTimer(DelayBeforeInterpTimerHandle);
		}
	}
	// 5. 게이지 감소 로직 (원하는 애니메이션)
	else if (DelayedPercent > TargetPercent)
	{
		// 6. [핵심] 딜레이 또는 보간 타이머가 *현재 실행 중이 아닐 때만* 딜레이 타이머를 새로 시작
		if (GetWorld() && !GetWorld()->GetTimerManager().IsTimerActive(InterpTimerHandle) && !GetWorld()->GetTimerManager().IsTimerActive(DelayBeforeInterpTimerHandle))
		{
			GetWorld()->GetTimerManager().SetTimer(DelayBeforeInterpTimerHandle, this, &UGS_ChanAimingSkillBar::StartDelayBarInterp, InterpDelayTime, false);
		}
		// 7. 만약 타이머가 이미 실행 중이라면 (IsTimerActive == true)
		//    아무것도 하지 않습니다.
		//    진행 중이던 UpdateDelayedBackBar() 함수가 다음 틱에 새로고침된
		//    TargetPercent 값을 읽어서 계속 보간을 진행할 것입니다.
	}
}

void UGS_ChanAimingSkillBar::SetAimingProgressByDamage(float Progress)
{
	// Progress = FMath::Clamp(Progress, 0.0f, 1.0f);
	//
	// // 앞바는 즉시
	// if (AimingProgressBar)
	// 	AimingProgressBar->SetPercent(Progress);
	//
	// // 뒤바는 보간 목표만 변경
	// TargetBackPercent = Progress;

	Progress = FMath::Clamp(Progress, 0.0f, 1.0f);
	TargetPercent = Progress;

	// 앞바는 즉시
	if (AimingProgressBar)
		AimingProgressBar->SetPercent(TargetPercent);

	// 기존 타이머 클리어
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(InterpTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(DelayBeforeInterpTimerHandle);
	}

	// Drakhar 게이지 로직 참고
	if (DelayedPercent < TargetPercent)
	{
		// 게이지가 회복되는 경우 (드문 경우지만)
		DelayedPercent = TargetPercent;
		if (AimingProgressBar_back)
		{
			AimingProgressBar_back->SetPercent(DelayedPercent);
		}
	}
	else if (DelayedPercent > TargetPercent)
	{
		// 게이지가 감소하는 경우 (원하는 동작)
		if (GetWorld())
		{
			// InterpDelayTime(0.1초) 후에 보간 시작
			GetWorld()->GetTimerManager().SetTimer(DelayBeforeInterpTimerHandle, this, &UGS_ChanAimingSkillBar::StartDelayBarInterp, InterpDelayTime, false);
		}
	}
}

void UGS_ChanAimingSkillBar::ShowSkillBar(bool bShow)
{
	SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UGS_ChanAimingSkillBar::StartDelayBarInterp()
{
	if (!GetWorld()) return;
	
	GetWorld()->GetTimerManager().SetTimer(InterpTimerHandle, this, &UGS_ChanAimingSkillBar::UpdateDelayedBackBar, 0.01f, true);
}

void UGS_ChanAimingSkillBar::UpdateDelayedBackBar()
{
	// Drakhar의 UpdateDelayedStamina 로직 참고
	if (DelayedPercent > TargetPercent
		&& !FMath::IsNearlyEqual(DelayedPercent, TargetPercent, 0.001f))
	{
		// 0.01초(타이머 간격)와 저장된 BackBarInterpSpeed를 사용
		DelayedPercent = FMath::FInterpTo(DelayedPercent, TargetPercent, 0.01f, BackBarInterpSpeed);
		
		if (AimingProgressBar_back)
		{
			AimingProgressBar_back->SetPercent(DelayedPercent);
		}
	}
	else
	{
		// 목표 도달 시
		DelayedPercent = TargetPercent;
		if (AimingProgressBar_back)
		{
			AimingProgressBar_back->SetPercent(DelayedPercent);
		}
		
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(InterpTimerHandle);
		}
	}
}
