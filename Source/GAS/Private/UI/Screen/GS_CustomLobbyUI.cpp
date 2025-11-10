#include "UI/Screen/GS_CustomLobbyUI.h"
#include "Components/TextBlock.h"
#include "System/PlayerController/GS_CustomLobbyPC.h"
#include "RuneSystem/GS_ArcaneBoardLPS.h"
#include "CommonUI/Public/CommonButtonBase.h"
#include "UI/Common/CustomCommonButton.h"
#include "UI/Common/GS_CommonTwoBtnPopup.h"
#include "UI/Popup/GS_FriendListWidget.h"
#include "Components/Overlay.h"
#include "System/GS_GameInstance.h"
#include "System/GS_PlayerState.h"
#include "Kismet/GameplayStatics.h"


void UGS_CustomLobbyUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (JobSelectionButton)
	{
		if (UCommonButtonBase* JobSelectionButtonBase = Cast<UCommonButtonBase>(JobSelectionButton))
		{
			JobSelectionButtonBase->OnClicked().AddUObject(this, &UGS_CustomLobbyUI::OnJobSelectionButtonClicked);
		}
	}

	if (PerkOrDungeonButton)
	{
		if (UCommonButtonBase* PerkOrDungeonButtonBase = Cast<UCommonButtonBase>(PerkOrDungeonButton))
		{
			PerkOrDungeonButtonBase->OnClicked().AddUObject(this, &UGS_CustomLobbyUI::OnPerkOrDungeonButtonClicked);
		}
	}

	if (RoleChangeButton)
	{
		if (UCommonButtonBase* RoleChangeButtonBase = Cast<UCommonButtonBase>(RoleChangeButton))
		{
			RoleChangeButtonBase->OnClicked().AddUObject(this, &UGS_CustomLobbyUI::OnRoleChangeButtonClicked);
		}
	}
	
	if (ReadyButton)
	{
		if (UCommonButtonBase* ReadyButtonBase = Cast<UCommonButtonBase>(ReadyButton))
		{
			ReadyButtonBase->OnClicked().AddUObject(this, &UGS_CustomLobbyUI::OnReadyButtonClicked);
		}
	}

	if (BackButton)
	{
		if (UCommonButtonBase* BackButtonBase = Cast<UCommonButtonBase>(BackButton))
		{
			BackButtonBase->OnClicked().AddUObject(this, &UGS_CustomLobbyUI::OnBackButtonClicked);
		}
	}

	if (FriendListButton)
	{
		if (UCommonButtonBase* FriendListButtonBase = Cast<UCommonButtonBase>(FriendListButton))
		{
			FriendListButtonBase->OnClicked().AddUObject(this, &UGS_CustomLobbyUI::OnFriendListButtonClicked);
		}
	}

	if (CommonPopUpUI)
	{
		CommonPopUpUI->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UGS_CustomLobbyUI::OnJobSelectionButtonClicked()
{
	AGS_CustomLobbyPC* PC = GetOwningPlayer<AGS_CustomLobbyPC>();
	if (PC)
	{
		PC->RequestOpenJobSelectionPopup();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get PlayerController"));
	}
}

void UGS_CustomLobbyUI::OnPerkOrDungeonButtonClicked()
{
	AGS_CustomLobbyPC* PC = GetOwningPlayer<AGS_CustomLobbyPC>();
	if (PC)
	{
		PC->RequestOpenPerkOrDungeonPopup();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get PlayerController"));
	}
}

void UGS_CustomLobbyUI::OnReadyButtonClicked()
{
	// 캐릭터별 준비 사운드 재생
	PlayCharacterReadySound();
	
	AGS_CustomLobbyPC* PC = GetOwningPlayer<AGS_CustomLobbyPC>();
	if (PC)
	{
		PC->RequestToggleReadyStatus();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get PlayerController"));
	}
}

void UGS_CustomLobbyUI::OnRoleChangeButtonClicked()
{
	AGS_CustomLobbyPC* PC = GetOwningPlayer<AGS_CustomLobbyPC>();
	if (PC)
	{
		PC->RequestToggleRole();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get PlayerController"));
	}
}

void UGS_CustomLobbyUI::OnBackButtonClicked()
{
	AGS_CustomLobbyPC* PC = GetOwningPlayer<AGS_CustomLobbyPC>();
	if (PC)
	{
		if (PC->HasCurrentModalWidget())
		{
			if (!PC->CheckAndShowUnsavedChangesConfirm())
			{
				PC->ClearCurrentModalWidget();
			}
			return;
		}
	}
	CommonPopUpUI->SetVisibility(ESlateVisibility::Visible);
	CommonPopUpUI->SetDescription(
		NSLOCTEXT("CustomLobby", "LeaveSessionConfirm", "Do you want to leave\nthe session?")
	);
	CommonPopUpUI->OnYesClicked.BindUObject(this, &UGS_CustomLobbyUI::OnBackPopupYesButtonClicked);
	CommonPopUpUI->OnNoClicked.BindUObject(this, &UGS_CustomLobbyUI::OnBackPopupNoButtonClicked);
}

void UGS_CustomLobbyUI::OnFriendListButtonClicked()
{
	if (!FriendListOverlay) return;

	if (!FriendListWidgetInstance)
	{
		if (FriendListWidgetClass)
		{
			FriendListWidgetInstance = CreateWidget<UGS_FriendListWidget>(GetOwningPlayer(), FriendListWidgetClass);
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

void UGS_CustomLobbyUI::UpdateRoleSpecificText(EPlayerRole NewRole)
{
	ChangeRoleBtnIcon(NewRole);
	
	if (PerkDungeonText)
	{
		FText NewTextToShow;
		switch (NewRole)
		{
		case EPlayerRole::PR_Seeker:
			NewTextToShow = FText::FromString(TEXT("Perk (Seeker)"));
			break;
		case EPlayerRole::PR_Guardian:
			NewTextToShow = FText::FromString(TEXT("Dungeon (Guardian)"));
			break;
		case EPlayerRole::PR_None:
		default:
			break;
		}
		PerkDungeonText->SetText(NewTextToShow);
		UE_LOG(LogTemp, Log, TEXT("UGS_CustomLobbyUI: PerkDungeonText updated for Role: %s"), *UEnum::GetValueAsString(NewRole));
	}
}

void UGS_CustomLobbyUI::ChangeRoleBtnIcon(EPlayerRole NewRole)
{
	if (RoleChangeButton)
	{
		if (UCustomCommonButton* CustomButtonBase = Cast<UCustomCommonButton>(RoleChangeButton))
		{
			int Idx = 0;
			if (NewRole == EPlayerRole::PR_Seeker)
				Idx = 0;
			else if (NewRole == EPlayerRole::PR_Guardian)
				Idx = 1;
			CustomButtonBase->ChangeLayerIconImage(Idx);
		}
	}

	if (JobSelectionButton)
	{
		if (UCustomCommonButton* CustomButtonBase = Cast<UCustomCommonButton>(JobSelectionButton))
		{
			int Idx = 0;
			if (NewRole == EPlayerRole::PR_Seeker)
				Idx = 0;
			else if (NewRole == EPlayerRole::PR_Guardian)
				Idx = 1;
			CustomButtonBase->ChangeLayerIconImage(Idx);
		}
	}

	if (PerkOrDungeonButton)
	{
		if (UCustomCommonButton* CustomButtonBase = Cast<UCustomCommonButton>(PerkOrDungeonButton))
		{
			int Idx = 0;
			if (NewRole == EPlayerRole::PR_Seeker)
				Idx = 0;
			else if (NewRole == EPlayerRole::PR_Guardian)
				Idx = 1;
			CustomButtonBase->ChangeLayerIconImage(Idx);
		}
	}
}

void UGS_CustomLobbyUI::UpdateReadyButtonText(bool bIsReady)
{
	if (ReadyButton)
	{
		if (UCustomCommonButton* CustomReadyButton = Cast<UCustomCommonButton>(ReadyButton))
		{
			CustomReadyButton->ChangeText(bIsReady ? 1 : 0);
		}
	}
}

void UGS_CustomLobbyUI::OnBackPopupYesButtonClicked()
{
	AGS_CustomLobbyPC* PC = GetOwningPlayer<AGS_CustomLobbyPC>();
	if (PC)
	{
		UGS_GameInstance* GI = Cast<UGS_GameInstance>(GetGameInstance());
		if (GI)
		{
			GI->GSLeaveSession(PC);
			UE_LOG(LogTemp, Log, TEXT("UGS_CustomLobbyUI: OnBackPopupYesButtonClicked - Leaving session."));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to get GameInstance"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to get PlayerController"));
	}
	CommonPopUpUI->SetVisibility(ESlateVisibility::Hidden);
}

void UGS_CustomLobbyUI::OnBackPopupNoButtonClicked()
{
	CommonPopUpUI->SetVisibility(ESlateVisibility::Hidden);
}

void UGS_CustomLobbyUI::ShowPerkSaveConfirmPopup()
{
	if (CommonPopUpUI)
	{
		CommonPopUpUI->SetVisibility(ESlateVisibility::Visible);
		CommonPopUpUI->SetDescription(
			NSLOCTEXT("CustomLobby", "PerkSaveConfirm", "Do you want to save\nthe changes?")
		);

		AGS_CustomLobbyPC* PC = GetOwningPlayer<AGS_CustomLobbyPC>();
		if (PC)
		{
			CommonPopUpUI->OnYesClicked.BindUObject(PC, &AGS_CustomLobbyPC::OnPerkSaveYes);
			CommonPopUpUI->OnNoClicked.BindUObject(PC, &AGS_CustomLobbyPC::OnPerkSaveNo);
		}
	}
}

void UGS_CustomLobbyUI::PlayCharacterReadySound()
{
	AGS_PlayerState* PS = GetOwningPlayerState<AGS_PlayerState>();
	if (!PS)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayCharacterReadySound: Failed to get PlayerState"));
		return;
	}

	USoundBase* SoundToPlay = nullptr;
	
	// 현재 준비 상태 확인 (버튼을 누르기 전 상태이므로 반대로 체크)
	bool bIsCurrentlyReady = PS->bIsReady;

	// 현재 역할에 따라 적절한 사운드 선택
	if (PS->CurrentPlayerRole == EPlayerRole::PR_Seeker)
	{
		switch (PS->CurrentSeekerJob)
		{
		case ESeekerJob::Ares:
			SoundToPlay = bIsCurrentlyReady ? AresCancelSound : AresReadySound;
			break;
		case ESeekerJob::Chan:
			SoundToPlay = bIsCurrentlyReady ? ChanCancelSound : ChanReadySound;
			break;
		case ESeekerJob::Merci:
			SoundToPlay = bIsCurrentlyReady ? MerciCancelSound : MerciReadySound;
			break;
		case ESeekerJob::Reina:
			SoundToPlay = bIsCurrentlyReady ? ReinaCancelSound : ReinaReadySound;
			break;
		default:
			break;
		}
	}
	else if (PS->CurrentPlayerRole == EPlayerRole::PR_Guardian)
	{
		switch (PS->CurrentGuardianJob)
		{
		case EGuardianJob::Drakhar:
			SoundToPlay = bIsCurrentlyReady ? DrakharCancelSound : DrakharReadySound;
			break;
		default:
			break;
		}
	}

	// 사운드 재생
	if (SoundToPlay)
	{
		UGameplayStatics::PlaySound2D(this, SoundToPlay);
		//UE_LOG(LogTemp, Log, TEXT("PlayCharacterReadySound: Playing %s sound for role %s"), 
		//	bIsCurrentlyReady ? TEXT("cancel") : TEXT("ready"),
		//	*UEnum::GetValueAsString(PS->CurrentPlayerRole));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayCharacterReadySound: No sound assigned for current character"));
	}
}
