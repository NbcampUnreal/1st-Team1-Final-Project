// Copyright Epic Games, Inc. All Rights Reserved.


#include "RuneSystem/GS_ArcaneBoardLPS.h"
#include "RuneSystem/GS_ArcaneBoardManager.h"
#include "RuneSystem/GS_ArcaneBoardSaveGame.h"
#include "UI/RuneSystem/GS_ArcaneBoardWidget.h"
#include "Character/GS_Character.h"
#include "Character/Component/GS_StatComp.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PanelWidget.h"

void UGS_ArcaneBoardLPS::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    //임시
    if (!IsValid(ArcaneBoardWidgetClass))
    {
        FSoftClassPath ArcaneBoardWidgetClassPath(TEXT("/Game/UI/RuneSystem/WBP_ArcaneBoard.WBP_ArcaneBoard_C"));
        UClass* LoadedClass = ArcaneBoardWidgetClassPath.TryLoadClass<UGS_ArcaneBoardWidget>();

        if (LoadedClass)
        {
            ArcaneBoardWidgetClass = LoadedClass;
        }
    }
}

void UGS_ArcaneBoardLPS::ShowArcaneBoardUI(UWidget* ParentWidget)
{
    UGS_ArcaneBoardWidget* BoardWidget = CreateArcaneBoardWidget();
    if (!BoardWidget)
    {
        UE_LOG(LogTemp, Error, TEXT("아케인 보드 위젯 생성 실패"));
        return;
    }

    if (ParentWidget)
    {
        if (UPanelWidget* ParentPanel = Cast<UPanelWidget>(ParentWidget))
        {
            ParentPanel->AddChild(BoardWidget);
        }
    }
}

UGS_ArcaneBoardWidget* UGS_ArcaneBoardLPS::CreateArcaneBoardWidget()
{
    if (CurrBoardWidget)
    {
        DestroyArcaneBoardWidget();
    }

    if (!BoardManager)
    {
        BoardManager = NewObject<UGS_ArcaneBoardManager>(this);
        BoardManager->OnStatsChanged.AddDynamic(this, &UGS_ArcaneBoardLPS::OnBoardStatsChanged);
    }

    ECharacterClass CurrPlayerClass = GetCurrPlayerClass();

    if (!BoardManager->SetCurrClass(CurrPlayerClass))
    {
        UE_LOG(LogTemp, Error, TEXT("플레이어 직업(%s) 설정 실패"),
            *UGS_EnumUtils::GetEnumAsString(CurrPlayerClass));
        return nullptr;
    }

    if (APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld()))
    {
        if (ArcaneBoardWidgetClass)
        {
            CurrBoardWidget = CreateWidget<UGS_ArcaneBoardWidget>(PC, ArcaneBoardWidgetClass);
            if (CurrBoardWidget)
            {
                UE_LOG(LogTemp, Log, TEXT("아케인 보드 위젯 생성 완료"));

                CurrBoardWidget->SetBoardManager(BoardManager);
                LoadBoardConfig();

                return CurrBoardWidget;
            }
        }
    }
    return nullptr;
}

void UGS_ArcaneBoardLPS::DestroyArcaneBoardWidget()
{
    if (CurrBoardWidget)
    {
        CurrBoardWidget->RemoveFromParent();
        CurrBoardWidget = nullptr;
    }
}

bool UGS_ArcaneBoardLPS::TryCloseArcaneBoardUI()
{
    //if (BoardManager && BoardManager->bHasUnsavedChanges)
    //{
    //    // 저장 확인 팝업 표시
    //    return false;
    //}
    //else
    //{
    //    DestroyArcaneBoardWidget();
    //    return true;
    //}
    DestroyArcaneBoardWidget();
    return true;
}

ECharacterClass UGS_ArcaneBoardLPS::GetCurrPlayerClass() const
{
    /*if (APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld()))
    {
        if (AGS_Character* PlayerCharacter = Cast<AGS_Character>(PC->GetPawn()))
        {
            ECharacterClass CharacterClass = PlayerCharacter->GetCharacterClass();

            UE_LOG(LogTemp, Log, TEXT("현재 플레이어 직업: %s"),
                *UGS_EnumUtils::GetEnumAsString(CharacterClass));

            return CharacterClass;
        }
    }*/

    return ECharacterClass::Ares;
}

void UGS_ArcaneBoardLPS::UpdateStatsUI()
{
}

void UGS_ArcaneBoardLPS::ApplyBoardChanges()
{
    if (BoardManager)
    {
        BoardManager->ApplyChanges();
        UpdateCharacterStats();
        SaveBoardConfig();
    }
}

void UGS_ArcaneBoardLPS::OnBoardStatsChanged(const FGS_StatRow& NewStats)
{
    RuneSystemStats = NewStats;
    UpdateStatsUI();
}

void UGS_ArcaneBoardLPS::SaveBoardConfig()
{
    if (!IsValid(BoardManager))
    {
        return;
    }

    const FString SaveSlotName = TEXT("ArcaneBoardSave");
    const int32 UserIdx = 0;

    UGS_ArcaneBoardSaveGame* SaveGameInstance = nullptr;

    if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, UserIdx))
    {
        SaveGameInstance = Cast<UGS_ArcaneBoardSaveGame>(
            UGameplayStatics::LoadGameFromSlot(SaveSlotName, UserIdx));
    }

    if (!SaveGameInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("SaveBoardConfig: 기존 세이브 파일 로드 실패, 새로 생성합니다."));

        SaveGameInstance = Cast<UGS_ArcaneBoardSaveGame>(
            UGameplayStatics::CreateSaveGameObject(UGS_ArcaneBoardSaveGame::StaticClass()));
    }

    if (!SaveGameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("SaveBoardConfig: 세이브 게임 객체 생성 실패"));
        return;
    }

    ECharacterClass CurrentClass = BoardManager->CurrClass;
    FRunePlacementData PlacementData;
    PlacementData.PlacedRunes = BoardManager->PlacedRunes;

    SaveGameInstance->SavedRunesByClass.Add(CurrentClass, PlacementData);

    bool bSaveSuccess = UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveSlotName, UserIdx);

    if (bSaveSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("SaveBoardConfig: 세이브 성공 - 직업: %s, 룬 개수: %d"),
            *UGS_EnumUtils::GetEnumAsString(CurrentClass),
            PlacementData.PlacedRunes.Num());

        BoardManager->bHasUnsavedChanges = false;
    }
}

void UGS_ArcaneBoardLPS::LoadBoardConfig()
{
    if (!IsValid(BoardManager))
    {
        return;
    }

    const FString SaveSlotName = TEXT("ArcaneBoardSave");
    const int32 UserIndex = 0;

    if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, UserIndex))
    {
        UE_LOG(LogTemp, Log, TEXT("LoadBoardConfig: 세이브 파일이 존재하지 않습니다."));
        return;
    }

    UGS_ArcaneBoardSaveGame* LoadedSaveGame = Cast<UGS_ArcaneBoardSaveGame>(
        UGameplayStatics::LoadGameFromSlot(SaveSlotName, UserIndex));

    if (!LoadedSaveGame)
    {
        UE_LOG(LogTemp, Error, TEXT("LoadBoardConfig: 세이브 파일 로드 실패"));
        return;
    }

    ECharacterClass CurrentClass = BoardManager->CurrClass;

    if (!LoadedSaveGame->SavedRunesByClass.Contains(CurrentClass))
    {
        UE_LOG(LogTemp, Log, TEXT("LoadBoardConfig: 현재 직업(%s)에 대한 세이브 데이터가 없습니다."),
            *UGS_EnumUtils::GetEnumAsString(CurrentClass));
        return;
    }

    const FRunePlacementData& SavedPlacementData = LoadedSaveGame->SavedRunesByClass[CurrentClass];

    // 데이터 유효성 검증
    if (SavedPlacementData.PlacedRunes.Num() == 0)
    {
        UE_LOG(LogTemp, Log, TEXT("LoadBoardConfig: 저장된 룬 배치 데이터가 비어있습니다."));
        return;
    }

    BoardManager->LoadSavedData(CurrentClass, SavedPlacementData.PlacedRunes);

    if (CurrBoardWidget)
    {
        CurrBoardWidget->UpdateGridVisuals();
        CurrBoardWidget->InitInventory();
    }

    UE_LOG(LogTemp, Log, TEXT("LoadBoardConfig: 로드 성공 - 직업: %s, 룬 개수: %d"),
        *UGS_EnumUtils::GetEnumAsString(CurrentClass),
        SavedPlacementData.PlacedRunes.Num());

    BoardManager->bHasUnsavedChanges = false;
}

void UGS_ArcaneBoardLPS::UpdateCharacterStats()
{
    if (APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld()))
    {
        if (AGS_Character* Character = Cast<AGS_Character>(PC->GetPawn()))
        {
            if (UGS_StatComp* StatComp = Character->GetStatComp())
            {
                //StatComp->UpdateStat();
            }
        }
    }
}

void UGS_ArcaneBoardLPS::RequestServerStatsUpdate()
{
}
