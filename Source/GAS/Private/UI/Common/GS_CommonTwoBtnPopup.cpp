#include "UI/Common//GS_CommonTwoBtnPopup.h"
#include "CommonButtonBase.h"
#include "Components/TextBlock.h"

void UGS_CommonTwoBtnPopup::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (YesButton)
	{
		if (UCommonButtonBase* YesButtonBase = Cast<UCommonButtonBase>(YesButton))
		{
			YesButtonBase->OnClicked().AddUObject(this, &UGS_CommonTwoBtnPopup::OnYesButtonClicked);
		}
	}

	if (NoButton)
	{
		if (UCommonButtonBase* NoButtonBase = Cast<UCommonButtonBase>(NoButton))
		{
			NoButtonBase->OnClicked().AddUObject(this, &UGS_CommonTwoBtnPopup::OnNoButtonClicked);
		}
	}
}

void UGS_CommonTwoBtnPopup::SetDescription(const FText& Description)
{
	if (DescriptionText)
	{
		DescriptionText->SetText(Description);
	}
}

void UGS_CommonTwoBtnPopup::OnYesButtonClicked()
{
	OnYesClicked.ExecuteIfBound();
	// UKismetSystemLibrary::QuitGame(GetWorld(), GetOwningPlayer(),EQuitPreference::Quit,false);
	SetVisibility(ESlateVisibility::Hidden);
}

void UGS_CommonTwoBtnPopup::OnNoButtonClicked()
{
	OnNoClicked.ExecuteIfBound();
	SetVisibility(ESlateVisibility::Hidden);
}
