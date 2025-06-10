// Copyright Epic Games, Inc. All Rights Reserved.


#include "RuneSystem/GS_ArcaneBoardLPS.h"
#include "RuneSystem/GS_ArcaneBoardManager.h"
#include "RuneSystem/GS_ArcaneBoardSaveGame.h"
#include "Character/GS_Character.h"
#include "System/GS_PlayerState.h"
#include "System/GS_PlayerRole.h"
#include "Character/Component/GS_StatComp.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PanelWidget.h"

void UGS_ArcaneBoardLPS::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    BindPlayerStateEvents();
}

ECharacterClass UGS_ArcaneBoardLPS::GetPlayerCharacterClass() const
{
    if (APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld()))
    {
        if (AGS_PlayerState* GSPlayerState = PC->GetPlayerState<AGS_PlayerState>())
        {
            if (GSPlayerState->CurrentPlayerRole == EPlayerRole::PR_Seeker)
            {
                ESeekerJob SeekerJob = GSPlayerState->CurrentSeekerJob;
                return MapSeekerJobToCharacterClass(SeekerJob);
            }
        }
    }

    return ECharacterClass::Ares;
}

void UGS_ArcaneBoardLPS::OnPlayerJobChanged(EPlayerRole CurrentRole)
{
    UE_LOG(LogTemp, Error, TEXT("=== 델리게이트 콜백 호출됨! Role: %d ==="), (int32)CurrentRole);

    ECharacterClass NewClass = GetPlayerCharacterClass();

    if (!IsValid(BoardManager))
    {
        GetOrCreateBoardManager();
    }

    if (IsValid(BoardManager) && BoardManager->GetCurrClass() != NewClass)
    {
        UE_LOG(LogTemp, Error, TEXT("즉시 BoardManager 업데이트: %d → %d"),
            (int32)BoardManager->GetCurrClass(), (int32)NewClass);

        BoardManager->SetCurrClass(NewClass);
        LoadBoardConfig();
    }
}

void UGS_ArcaneBoardLPS::InitializeRunes()
{
    if (!IsValid(BoardManager))
    {
        GetOrCreateBoardManager();
    }

    LoadBoardConfig();
}

void UGS_ArcaneBoardLPS::RefreshBoardForCurrentCharacter()
{
    ECharacterClass CurrentClass = GetPlayerCharacterClass();

    if (!IsValid(BoardManager))
    {
        GetOrCreateBoardManager();
    }

    if (IsValid(BoardManager))
    {
        if (BoardManager->GetCurrClass() != CurrentClass)
        {
            BoardManager->SetCurrClass(CurrentClass);
            LoadBoardConfig();
        }
    }
}

void UGS_ArcaneBoardLPS::UpdateStatsUI()
{
}

void UGS_ArcaneBoardLPS::ApplyBoardChanges()
{
    if (BoardManager)
    {
        BoardManager->ApplyChanges();
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

    BoardManager->PlacedRunes.Empty();
    BoardManager->InitGridState();
    BoardManager->CurrStatEffects = FGS_StatRow();
    BoardManager->bHasUnsavedChanges = false;

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

    UE_LOG(LogTemp, Log, TEXT("LoadBoardConfig: 로드 성공 - 직업: %s, 룬 개수: %d"),
        *UGS_EnumUtils::GetEnumAsString(CurrentClass),
        SavedPlacementData.PlacedRunes.Num());

    BoardManager->bHasUnsavedChanges = false;
}

bool UGS_ArcaneBoardLPS::HasUnsavedChanges() const
{
    if(IsValid(BoardManager))
    {
        return BoardManager->bHasUnsavedChanges;
    }

    return false;
}

UGS_ArcaneBoardManager* UGS_ArcaneBoardLPS::GetOrCreateBoardManager()
{
    if (!IsValid(BoardManager))
    {
        BoardManager = NewObject<UGS_ArcaneBoardManager>(this);
        BoardManager->OnStatsChanged.AddDynamic(this, &UGS_ArcaneBoardLPS::OnBoardStatsChanged);

        ECharacterClass CurrPlayerClass = GetPlayerCharacterClass();
        BoardManager->SetCurrClass(CurrPlayerClass);
    }

    return BoardManager;
}

void UGS_ArcaneBoardLPS::ForceApplyChanges()
{
    if (IsValid(BoardManager))
    {
        BoardManager->ApplyChanges();
        SaveBoardConfig();
    }
}

void UGS_ArcaneBoardLPS::RequestServerStatsUpdate()
{
}

void UGS_ArcaneBoardLPS::BindPlayerStateEvents()
{
    if (APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld()))
    {
        if (AGS_PlayerState* GSPlayerState = PC->GetPlayerState<AGS_PlayerState>())
        {
            UnbindPlayerStateEvents();
            GSPlayerState->OnJobChangedDelegate.AddDynamic(this, &UGS_ArcaneBoardLPS::OnPlayerJobChanged);
            BoundPlayerState = GSPlayerState;
            UE_LOG(LogTemp, Log, TEXT("PlayerState 델리게이트 바인딩 완료"));
        }
    }
}

void UGS_ArcaneBoardLPS::UnbindPlayerStateEvents()
{
    if (BoundPlayerState.IsValid())
    {
        BoundPlayerState->OnJobChangedDelegate.RemoveDynamic(this, &UGS_ArcaneBoardLPS::OnPlayerJobChanged);
        BoundPlayerState.Reset();
    }
}

ECharacterClass UGS_ArcaneBoardLPS::MapSeekerJobToCharacterClass(ESeekerJob SeekerJob) const
{
    switch (SeekerJob)
    {
    case ESeekerJob::Ares: return ECharacterClass::Ares;
    case ESeekerJob::Chan: return ECharacterClass::Chan;
    case ESeekerJob::Merci: return ECharacterClass::Merci;
    default:
        return ECharacterClass::Ares;
    }
}
