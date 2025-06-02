#include "UI/Popup/GS_CharacterSelectList.h"

#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/ScrollBox.h"
#include "Components/Spacer.h"
#include "RuneSystem/GS_EnumUtils.h"
#include "System/PlayerController/GS_CustomLobbyPC.h"
#include "UI/Common/CustomCommonButton.h"

void UGS_CharacterSelectList::NativeConstruct()
{
	Super::NativeConstruct();
	
}

void UGS_CharacterSelectList::CreateChildWidgets(EPlayerRole PlayerRole)
{
	AddSpacerInVerticalBox();

	int LoopCount = 0;
	if (PlayerRole == EPlayerRole::PR_Seeker)
	{
		LoopCount = UGS_EnumUtils::GetEnumCount<ESeekerJob>() - 1;
	}
	else if (PlayerRole == EPlayerRole::PR_Guardian)
	{
		LoopCount = UGS_EnumUtils::GetEnumCount<EGuardianJob>() - 1;
	}
	
	for (int32 i = 0; i < LoopCount; ++i)
	{
		if (i % SlotCount == 0)
		{
			UHorizontalBox* NewHBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
			HorizontalBoxes.Add(NewHBox);

			CharacterSlotList->AddChild(NewHBox);
			AddSpacerInVerticalBox();

			AddSpacerInHorizeontalBox();
		}

		UCustomCommonButton* NewBtn = CreateWidget<UCustomCommonButton>(GetWorld(), ButtonSlotWidgetClass);
		if (!NewBtn) continue;

		HorizontalBoxes.Last()->AddChild(NewBtn);

		AddSpacerInHorizeontalBox();
	}
}

void UGS_CharacterSelectList::AddSpacerInHorizeontalBox()
{
	if (!HorizontalBoxes.IsEmpty())
	{
		if (USpacer* NewSpacer = WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass()))
		{
			NewSpacer->SetSize(FVector2D(20,0));
			HorizontalBoxes.Last()->AddChild(NewSpacer);

		}
	}
}

void UGS_CharacterSelectList::AddSpacerInVerticalBox()
{
	if (USpacer* NewSpacer = WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass()))
	{
		NewSpacer->SetSize(FVector2D(0,20));
		CharacterSlotList->AddChild(NewSpacer);

	}
}
