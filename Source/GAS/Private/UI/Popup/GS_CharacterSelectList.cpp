#include "UI/Popup/GS_CharacterSelectList.h"

#include "Blueprint/WidgetTree.h"
#include "Components/HorizontalBox.h"
#include "Components/ScrollBox.h"
#include "Components/Spacer.h"
#include "RuneSystem/GS_EnumUtils.h"
#include "RuneSystem/GS_ArcaneBoardLPS.h"
#include "System/GS_PlayerState.h"
#include "System/PlayerController/GS_CustomLobbyPC.h"
#include "UI/Common/CustomCommonButton.h"
#include "UI/Data/GS_UICharacterInfoRow.h"
#include "Kismet/GameplayStatics.h"

void UGS_CharacterSelectList::NativeConstruct()
{
	Super::NativeConstruct();
	
}

void UGS_CharacterSelectList::NativeDestruct()
{
	Super::NativeDestruct();

	for (int i = 0; i < ButtonRefs.Num(); ++i)
	{
		if (ButtonRefs[i])
		{
			ButtonRefs[i]->OnClicked().Clear();
		}
	}
}

void UGS_CharacterSelectList::CreateChildWidgets(EPlayerRole PlayerRole)
{
	// 새로운 역할로 위젯을 생성할 때 이전 선택 상태 리셋
	LastSelectedCharacterID = -1;
	LastSelectedPlayerRole = static_cast<EPlayerRole>(0);

	AddSpacerInVerticalBox();

	int LoopCount = 0;
	if (PlayerRole == EPlayerRole::PR_Seeker)
	{
		LoopCount = static_cast<int> (ESeekerJob::End);
	}
	else if (PlayerRole == EPlayerRole::PR_Guardian)
	{
		LoopCount = static_cast<int> (EGuardianJob::End);
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
		ButtonRefs.Add(NewBtn);
		if (PlayerRole == EPlayerRole::PR_Seeker)
		{
			const ESeekerJob Job = static_cast<ESeekerJob>(i);
			const FName RowName = FName(*UGS_EnumUtils::GetEnumAsString<ESeekerJob>(Job));
			const FGS_UICharacterInfoRow* RowPtr = CharacterInfoDataTable->FindRow<FGS_UICharacterInfoRow>(RowName, TEXT("Lookup Character Info Row"));
			if (RowPtr)
			{
				NewBtn->IconTexture0 = RowPtr->Portrait;
				NewBtn->ChangeLayerIconImage(0);
			}
		}
		if (PlayerRole == EPlayerRole::PR_Guardian)
		{
			const EGuardianJob Job = static_cast<EGuardianJob>(i);
			const FName RowName = FName(*UGS_EnumUtils::GetEnumAsString<EGuardianJob>(Job));
			const FGS_UICharacterInfoRow* RowPtr = CharacterInfoDataTable->FindRow<FGS_UICharacterInfoRow>(RowName, TEXT("Lookup Character Info Row"));
			if (RowPtr)
			{
				NewBtn->IconTexture0 = RowPtr->Portrait;
				NewBtn->ChangeLayerIconImage(0);
			}
		}
		NewBtn->OnClicked().AddLambda([this, CharacterID = i, InPlayerRole = PlayerRole]()
		{
			OnCharacterSelectClicked(CharacterID, InPlayerRole);
		});
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

void UGS_CharacterSelectList::OnCharacterSelectClicked(int32 CharacterID, EPlayerRole PlayerRole)
{
	// 캐릭터 선택 사운드 재생
	PlayCharacterSelectSound(CharacterID, PlayerRole);
	
	AGS_PlayerState* GSPlayerState = GetOwningPlayerState<AGS_PlayerState>();
	if (!GSPlayerState)
		return;
	
	if (PlayerRole == EPlayerRole::PR_Guardian)
	{
		const FName RowName = FName(*UGS_EnumUtils::GetEnumAsString<EGuardianJob>(static_cast<EGuardianJob>(CharacterID)));
		if (const FGS_UICharacterInfoRow* RowPtr = CharacterInfoDataTable->FindRow<FGS_UICharacterInfoRow>(RowName, TEXT("Lookup Character Info Row")))
		{
			UE_LOG(LogTemp, Warning, TEXT("Guardian Job %d"), CharacterID);
			GSPlayerState->Server_SetGuardianJob((EGuardianJob)CharacterID);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("This UI is Locked"));
			return;
		}
	}
	else if (PlayerRole == EPlayerRole::PR_Seeker)
	{
		const FName RowName = FName(*UGS_EnumUtils::GetEnumAsString<ESeekerJob>(static_cast<ESeekerJob>(CharacterID)));
		if (const FGS_UICharacterInfoRow* RowPtr = CharacterInfoDataTable->FindRow<FGS_UICharacterInfoRow>(RowName, TEXT("Lookup Character Info Row")))
		{
			UE_LOG(LogTemp, Warning, TEXT("Seeker Job %d"), CharacterID);
			
			if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
			{
				if (UGS_ArcaneBoardLPS* LPS = LocalPlayer->GetSubsystem<UGS_ArcaneBoardLPS>())
				{
					LPS->OnPlayerJobChanged((ESeekerJob)CharacterID);
				}
			}
			
			GSPlayerState->Server_SetSeekerJob((ESeekerJob)CharacterID);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("This UI is Locked"));
			return;
		}
	}
}

void UGS_CharacterSelectList::PlayCharacterSelectSound(int32 CharacterID, EPlayerRole PlayerRole)
{
	// 같은 캐릭터를 다시 선택한 경우 사운드 재생하지 않음
	if (LastSelectedCharacterID == CharacterID && LastSelectedPlayerRole == PlayerRole)
	{
		return;
	}

	// 현재 선택을 저장
	LastSelectedCharacterID = CharacterID;
	LastSelectedPlayerRole = PlayerRole;

	USoundBase* SoundToPlay = nullptr;
	
	if (PlayerRole == EPlayerRole::PR_Guardian)
	{
		// 가디언 캐릭터 사운드
		switch (static_cast<EGuardianJob>(CharacterID))
		{
			case EGuardianJob::Drakhar:
				SoundToPlay = DrakharSelectSound;
				break;
			default:
				break;
		}
	}
	else if (PlayerRole == EPlayerRole::PR_Seeker)
	{
		// 시커 캐릭터 사운드
		switch (static_cast<ESeekerJob>(CharacterID))
		{
			case ESeekerJob::Ares:
				SoundToPlay = AresSelectSound;
				break;
			case ESeekerJob::Chan:
				SoundToPlay = ChanSelectSound;
				break;
			case ESeekerJob::Merci:
				SoundToPlay = MerciSelectSound;
				break;
			case ESeekerJob::Reina:
				SoundToPlay = ReinaSelectSound;
				break;
			default:
				break;
		}
	}

	if (SoundToPlay)
	{
		UGameplayStatics::PlaySound2D(this, SoundToPlay);
	}
}
