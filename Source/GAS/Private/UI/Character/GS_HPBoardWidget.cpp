#include "UI/Character/GS_HPBoardWidget.h"

#include "Character/GS_Character.h"
#include "Character/Player/GS_Player.h"
#include "Components/VerticalBox.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "System/GS_PlayerState.h"
#include "UI/Character/GS_HPWidget.h"
#include "UI/Character/GS_PlayerInfoWidget.h"

void UGS_HPBoardWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(GetOwningPlayer()->GetPawn()))
	{
		OwningCharacter = Cast<AGS_Character>(GetOwningPlayer()->GetPawn());
	}
	
	FTimerHandle WidgetTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(WidgetTimerHandle, this, &UGS_HPBoardWidget::InitBoardWidget, 3.f);
}

void UGS_HPBoardWidget::InitBoardWidget()
{
	if (!IsValid(PlayerInfoWidgetClass) || !IsValid(PlayerInfoWidgetList))
	{
		return;
	}
	
	PlayerInfoWidgetList->ClearChildren();

	AGameStateBase* GS = UGameplayStatics::GetGameState(this);
	if (IsValid(GS))
	{
		TArray<APlayerState*> PSA = GS->PlayerArray; 
		for (APlayerState* PS : PSA)
		{
			AGS_PlayerState* GSPS = Cast<AGS_PlayerState>(PS);
			if (IsValid(GSPS))
			{
				UGS_PlayerInfoWidget* PlayerInfoWidget = CreateWidget<UGS_PlayerInfoWidget>(this, PlayerInfoWidgetClass);
				if (IsValid(PlayerInfoWidget))
				{
					AGS_Player* Player = Cast<AGS_Player>(GSPS->GetPawn());
					if (IsValid(Player) && Player != OwningCharacter)
					{
						if (GSPS->CurrentPlayerRole == EPlayerRole::PR_Guardian)
						{
							continue;
						}
						PlayerInfoWidget->SetOwningActor(Player);
						PlayerInfoWidget->InitializePlayerInfoWidget(Player);
						PlayerInfoWidgetList->AddChildToVerticalBox(PlayerInfoWidget);
					}
				}
			}
		}
	}
	
}