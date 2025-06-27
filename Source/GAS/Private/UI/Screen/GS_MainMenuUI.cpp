#include "UI/Screen/GS_MainMenuUI.h"

#include "CommonButtonBase.h"
#include "Blueprint/WidgetTree.h"
#include "Components/ScaleBox.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetSystemLibrary.h"
#include "System/PlayerController/GS_MainMenuPC.h"
#include "UI/Common/CustomCommonButton.h"
#include "UI/Common//GS_CommonTwoBtnPopup.h"
#include "UI/Screen/Option/GS_OptionMenuUI.h"
#include "UI/Popup/GS_JoinFriendListWidget.h"
#include "Components/Overlay.h"

void UGS_MainMenuUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (PlayButton)
	{
		if (UCommonButtonBase* PlayButtonBase = Cast<UCommonButtonBase>(PlayButton))
		{
			PlayButtonBase->OnClicked().AddUObject(this, &UGS_MainMenuUI::OnPlayButtonClicked);
		}
	}

	if (CustomGameButton)
	{
		if (UCommonButtonBase* CustomGameButtonBase = Cast<UCommonButtonBase>(CustomGameButton))
		{
			CustomGameButtonBase->OnClicked().AddUObject(this, &UGS_MainMenuUI::OnCustomGameButtonClicked);
		}
	}

	if (ExitButton)
	{
		if (UCommonButtonBase* ExitButtonBase = Cast<UCommonButtonBase>(ExitButton))
		{
			ExitButtonBase->OnClicked().AddUObject(this, &UGS_MainMenuUI::OnExitButtonClicked);
		}
	}

	if (FriendListButton)
	{
		if (UCommonButtonBase* FriendListButtonBase = Cast<UCommonButtonBase>(FriendListButton))
		{
			FriendListButtonBase->OnClicked().AddUObject(this, &UGS_MainMenuUI::OnFriendListButtonClicked);
		}
	}

	if (TutorialButton)
	{
		if (UCommonButtonBase* TutorialButtonBase = Cast<UCommonButtonBase>(TutorialButton))
		{
			TutorialButtonBase->OnClicked().AddUObject(this, &UGS_MainMenuUI::OnTutorialButtonClicked);
		}
	}
	
	//텍스트 초기 설정
	//if (PlayText) PlayText->SetText(FText::FromString(TEXT("Play")));

	//초기 visibility 설정
	if (PlayBtnPopUp)
	{
		PlayBtnPopUp->SetVisibility(ESlateVisibility::Hidden);
	}
	if (ExitPopUp)
	{
		ExitPopUp->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UGS_MainMenuUI::OnPlayButtonClicked()
{
	if (!bIsPlayButtonClicked)
	{
		if (UCustomCommonButton* CustomPlayButton = Cast<UCustomCommonButton>(PlayButton))
		{
			CustomPlayButton->ChangeText(1);
		}
		if (PlayBtnPopUp)
		{
			PlayBtnPopUp->SetVisibility(ESlateVisibility::Visible);
		}
		if (CreditButton)
		{
			CreditButton->SetVisibility(ESlateVisibility::Hidden);
		}
		bIsPlayButtonClicked = true;
	}
	else
	{
		if (UCustomCommonButton* CustomPlayButton = Cast<UCustomCommonButton>(PlayButton))
		{
			CustomPlayButton->ChangeText(0);
		}
		if (PlayBtnPopUp)
		{
			PlayBtnPopUp->SetVisibility(ESlateVisibility::Hidden);
		}
		if (CreditButton)
		{
			CreditButton->SetVisibility(ESlateVisibility::Visible);
		}
		bIsPlayButtonClicked = false;
	}
	
}

void UGS_MainMenuUI::OnCustomGameButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("UGS_MainMenuUI: OnCustomGameButtonClicked() CALLED."));
	AGS_MainMenuPC* PC = GetOwningPlayer<AGS_MainMenuPC>();
	if (PC)
	{
		UE_LOG(LogTemp, Log, TEXT("UGS_MainMenuUI: Owning PlayerController is VALID and cast to AGS_MainMenuPC. Calling HandleCustomGameRequest..."));
		PC->HandleCustomGameRequest();
	}
}

void UGS_MainMenuUI::OnExitButtonClicked()
{
	if (ExitPopUp)
	{
		ExitPopUp->SetDescription(FText::FromString(TEXT("게임을 종료하시겠습니까?")));
		ExitPopUp->OnYesClicked.BindUObject(this, &UGS_MainMenuUI::OnExitPopupYesButtonClicked);
		ExitPopUp->OnNoClicked.BindUObject(this, &UGS_MainMenuUI::OnExitPopupNoButtonClicked);
		ExitPopUp->SetVisibility(ESlateVisibility::Visible);
	}
}

void UGS_MainMenuUI::OnExitPopupYesButtonClicked()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), GetOwningPlayer(),EQuitPreference::Quit,false);
}

void UGS_MainMenuUI::OnExitPopupNoButtonClicked()
{
}

void UGS_MainMenuUI::OnFriendListButtonClicked()
{
	if (!FriendListOverlay) return;

	if (!FriendListWidgetInstance)
	{
		if (FriendListWidgetClass)
		{
			FriendListWidgetInstance = CreateWidget<UGS_JoinFriendListWidget>(GetOwningPlayer(), FriendListWidgetClass);
			if (FriendListWidgetInstance)
			{
				FriendListOverlay->AddChild(FriendListWidgetInstance);
				FriendListOverlay->SetVisibility(ESlateVisibility::Visible);
				return;
			}
		}
	}

	if (FriendListOverlay->GetVisibility() == ESlateVisibility::Collapsed)
	{
		FriendListOverlay->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		FriendListOverlay->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UGS_MainMenuUI::OnTutorialButtonClicked()
{
	AGS_MainMenuPC* PC = GetOwningPlayer<AGS_MainMenuPC>();
	if (PC)
	{
		PC->ShowTutorialUI();
	}
}
