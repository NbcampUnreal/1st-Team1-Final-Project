#include "UI/Character/GS_HPWidget.h"

#include "Character/GS_Character.h"
#include "Character/Component/GS_StatComp.h"
#include "Components/ProgressBar.h"
#include "UI/Component/GS_WidgetShakeHelper.h"

UGS_HPWidget::UGS_HPWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void UGS_HPWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!IsValid(OwningCharacter))
	{
		OwningCharacter = Cast<AGS_Character>(GetOwningPlayer()->GetPawn());
	}
	
	if (IsValid(OwningCharacter))
	{
		OwningCharacter->SetHPBarWidget(this);
	}
	
	// 흔들림 헬퍼 생성 및 초기화
	if (!ShakeHelper)
	{
		ShakeHelper = NewObject<UGS_WidgetShakeHelper>(this);
		ShakeHelper->Initialize(GetWorld());
		
		// 타겟 위젯 설정
		if (HPBarWidget)
		{
			ShakeHelper->SetTargetWidget(HPBarWidget);
		}
	}
}

void UGS_HPWidget::NativeDestruct()
{
	// 흔들림 헬퍼 정리
	if (ShakeHelper)
	{
		ShakeHelper->Cleanup();
	}
	
	Super::NativeDestruct();
}

void UGS_HPWidget::InitializeHPWidget(UGS_StatComp* InStatComp)
{
	if (HPBarWidget)
	{
		HPBarWidget->SetPercent(CurrentHPPercent);
	}
	if (HPDelayBarWidget)
	{
		HPDelayBarWidget->SetPercent(CurrentHPPercent);
	}
	if (HPSlider)
	{
		HPSlider->SetValue(CurrentHPPercent);
	}
	OnCurrentHPBarChanged(InStatComp);
}

void UGS_HPWidget::OnCurrentHPBarChanged(UGS_StatComp* InStatComp)
{
	if (!IsValid(InStatComp))
	{
		return;
	}

	TargetHPPercent = InStatComp->GetCurrentHealth() / InStatComp->GetMaxHealth();
	
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(InterpTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(DelayBeforInterpTimerHandle);
	}
	
	if (TargetHPPercent < CurrentHPPercent)
	{
		//데미지
		float DamageRatio = CurrentHPPercent - TargetHPPercent; // 피해 비율 계산
		float ActualDamage = DamageRatio * InStatComp->GetMaxHealth(); // 실제 피해량 계산
		
		CurrentHPPercent = TargetHPPercent; 
		if (HPBarWidget)
		{
			HPBarWidget->SetPercent(TargetHPPercent); 
		}
		if (HPSlider)
		{
			HPSlider->SetValue(TargetHPPercent); 
		}
		
		// 흔들림 헬퍼를 사용한 피해 효과 시작 (안전성 체크 포함)
		if (IsValid(ShakeHelper) && ActualDamage > 1.0f)
		{
			ShakeHelper->StartShakeEffect(DamageRatio, ActualDamage);
		}
	}
	else
	{
		//힐
		DelayedHPPercent = TargetHPPercent;

		if (HPDelayBarWidget)
		{
			HPDelayBarWidget->SetPercent(DelayedHPPercent);
		}
		if (HPSlider)
		{
			HPSlider->SetValue(DelayedHPPercent);
		}
		CurrentHPPercent = HPBarWidget ? HPBarWidget->GetPercent() : TargetHPPercent;
	}
	
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(DelayBeforInterpTimerHandle, this, &UGS_HPWidget::StartDelayBarInterp, 0.1f, false);
	}
}

void UGS_HPWidget::StartDelayBarInterp()
{
	if (!GetWorld()) return;

	if (DelayedHPPercent > TargetHPPercent)
	{
		//데미지
		const float Duration = 0.05f;
		float Delta = FMath::Abs(DelayedHPPercent - TargetHPPercent);
		InterpSpeed = FMath::Max(Delta / Duration, 0.01f);
		GetWorld()->GetTimerManager().SetTimer(InterpTimerHandle, this, &UGS_HPWidget::UpdateDelayedHP, 0.01f, true);
	}
	else
	{
		//힐
		const float Duration = 0.15f;
		float Delta = FMath::Abs(TargetHPPercent - CurrentHPPercent);
		InterpSpeed = FMath::Max(Delta / Duration, 0.5f);
		GetWorld()->GetTimerManager().SetTimer(InterpTimerHandle, this, &UGS_HPWidget::UpdateMainHP, 0.01f, true);
	}
}

void UGS_HPWidget::UpdateDelayedHP()
{
	if (DelayedHPPercent > TargetHPPercent)
	{
		DelayedHPPercent = FMath::FInterpTo(DelayedHPPercent, TargetHPPercent, 0.01f, InterpSpeed);
		if (HPDelayBarWidget)
		{
			HPDelayBarWidget->SetPercent(DelayedHPPercent);
		}
	}
	else
	{
		DelayedHPPercent = TargetHPPercent;
		if (HPDelayBarWidget)
		{
			HPDelayBarWidget->SetPercent(DelayedHPPercent);
		}
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(InterpTimerHandle);
		}
	}
}

void UGS_HPWidget::UpdateMainHP()
{
	if (CurrentHPPercent < TargetHPPercent)
	{
		CurrentHPPercent = FMath::FInterpTo(CurrentHPPercent, TargetHPPercent, 0.01f, InterpSpeed);
		if (HPBarWidget)
		{
			HPBarWidget->SetPercent(CurrentHPPercent);
		}
	}
	else
	{
		CurrentHPPercent = TargetHPPercent;
		if (HPBarWidget)
		{
			HPBarWidget->SetPercent(CurrentHPPercent);
		}
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(InterpTimerHandle);
		}
	}
}