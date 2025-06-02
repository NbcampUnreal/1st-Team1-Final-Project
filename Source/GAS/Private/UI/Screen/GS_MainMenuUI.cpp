#include "UI/Screen/GS_MainMenuUI.h"

#include "CommonButtonBase.h"
#include "Components/Button.h"
#include "Components/ScaleBox.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "System/PlayerController/GS_MainMenuPC.h"
#include "UI/Common/CustomCommonButton.h"

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
	else UE_LOG(LogTemp, Error, TEXT("UGS_MainMenuUI: PlayButton is not bound in Blueprint!"));

	if (CustomGameButton)
	{
		if (UCommonButtonBase* CustomGameButtonBase = Cast<UCommonButtonBase>(CustomGameButton))
		{
			CustomGameButtonBase->OnClicked().AddUObject(this, &UGS_MainMenuUI::OnCustomGameButtonClicked);
		}
	}
	else UE_LOG(LogTemp, Error, TEXT("UGS_MainMenuUI: CustomGameButton is not bound in Blueprint!"));

	if (ExitButton)
	{
		if (UCommonButtonBase* ExitButtonBase = Cast<UCommonButtonBase>(ExitButton))
		{
			ExitButtonBase->OnClicked().AddUObject(this, &UGS_MainMenuUI::OnExitButtonClicked);
		}
	}
	else UE_LOG(LogTemp, Error, TEXT("UGS_MainMenuUI: ExitButton is not bound in Blueprint!"));


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

void UGS_MainMenuUI::OnExitButtonClicked()
{
	if (ExitPopUp)
	{
		ExitPopUp->SetVisibility(ESlateVisibility::Visible);
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

