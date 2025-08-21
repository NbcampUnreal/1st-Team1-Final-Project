// Copyright Epic Games, Inc. All Rights Reserved.


#include "RuneSystem/GS_ArcaneBoardLPS.h"
#include "RuneSystem/GS_ArcaneBoardManager.h"
#include "UI/RuneSystem/GS_ArcaneBoardWidget.h"
#include "RuneSystem/GS_ArcaneBoardSaveGame.h"
#include "Character/GS_Character.h"
#include "Kismet/GameplayStatics.h"

void UGS_ArcaneBoardLPS::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    CurrentPresetIndex = 1;
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

void UGS_ArcaneBoardLPS::OnPlayerJobChanged(ESeekerJob SeekerJob)
{
    if (!IsValid(BoardManager))
    {
        GetOrCreateBoardManager();
    }

    ECharacterClass NewCharacterClass = MapSeekerJobToCharacterClass(SeekerJob);

    if (IsValid(BoardManager) && BoardManager->GetCurrClass() != NewCharacterClass)
    {
        BoardManager->SetCurrClass(NewCharacterClass);
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

void UGS_ArcaneBoardLPS::RefreshBoardForCurrCharacter()
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
void UGS_ArcaneBoardLPS::ApplyBoardChanges()
{
    if (BoardManager)
    {
        if(BoardManager->bHasUnsavedChanges)
        {
            BoardManager->ApplyChanges();
        }
        SaveBoardConfig(CurrentPresetIndex);
    }
}

void UGS_ArcaneBoardLPS::OnBoardStatsChanged(const FArcaneBoardStats& NewStats)
{
    RuneSystemStats = NewStats;
    
    if (CurrentUIWidget.IsValid())
    {
        CurrentUIWidget->OnStatsChanged(RuneSystemStats);
    }
}

void UGS_ArcaneBoardLPS::SaveBoardConfig(int32 PresetIndex)
{
    if (!IsValid(BoardManager))
    {
        return;
    }

    int32 TargetPresetIndex = (PresetIndex == -1) ? CurrentPresetIndex : PresetIndex;

    if (TargetPresetIndex < 1 || TargetPresetIndex>3)
    {
        return;
    }

    UGS_ArcaneBoardSaveGame* SaveGameInstance = GetOrCreateSaveGame();
    if (!SaveGameInstance)
    {
        return;
    }

    ECharacterClass CurrClass = BoardManager->CurrClass;

    if (!SaveGameInstance->SavedRunesByClass.Contains(CurrClass))
    {
        SaveGameInstance->SavedRunesByClass.Add(CurrClass, FArcaneBoardPresets());
    }

    FArcaneBoardPresets& ClassPresets = SaveGameInstance->SavedRunesByClass[CurrClass];

    TArray<FPlacedRuneInfo>* TargetPreset = GetPresetArray(ClassPresets, TargetPresetIndex);
    if (TargetPreset)
    {
        *TargetPreset = BoardManager->PlacedRunes;
        ClassPresets.LastUsedPresetIndex = TargetPresetIndex;
        CurrentPresetIndex = TargetPresetIndex;
    }

    SaveGameInstance->OwnedRuneIDs = OwnedRuneIDs;

    const FString SaveSlotName = TEXT("ArcaneBoardSave");
    const int32 UserIdx = 0;
    bool bSaveSuccess = UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveSlotName, UserIdx);

    if (bSaveSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("SaveBoardConfig: 저장 성공 - 클래스: %s, 프리셋: %d, 룬 개수: %d"),
            *UGS_EnumUtils::GetEnumAsString(CurrClass),
            TargetPresetIndex,
            BoardManager->PlacedRunes.Num());

        BoardManager->bHasUnsavedChanges = false;
    }
}

void UGS_ArcaneBoardLPS::LoadBoardConfig(int32 PresetIndex)
{
    if (!IsValid(BoardManager))
    {
        UE_LOG(LogTemp, Error, TEXT("BoardManager가 유효하지 않음"));
        return;
    }

    EnsureRuneInvenInit();

    int32 TargetPresetIndex;
    if (PresetIndex == -1)
    {
        TargetPresetIndex = GetLastUsedPresetIndex();
        UE_LOG(LogTemp, Log, TEXT("최근 사용 프리셋 로드: %d"), TargetPresetIndex);
    }
    else
    {
        TargetPresetIndex = PresetIndex;
        UE_LOG(LogTemp, Log, TEXT("지정된 프리셋 로드: %d"), TargetPresetIndex);
    }

    if (TargetPresetIndex < 1 || TargetPresetIndex > 3)
    {
        UE_LOG(LogTemp, Warning, TEXT("잘못된 프리셋 인덱스, 기본값 1 사용: %d"), TargetPresetIndex);
        TargetPresetIndex = 1;
    }

    CurrentPresetIndex = TargetPresetIndex;

    ECharacterClass CurrClass = BoardManager->CurrClass;
    TArray<FPlacedRuneInfo> PresetData = LoadPresetData(CurrClass, TargetPresetIndex);

    ApplyPresetToBoard(CurrClass, PresetData);

    UE_LOG(LogTemp, Log, TEXT("프리셋 %d 로드 완료"), TargetPresetIndex);
}

bool UGS_ArcaneBoardLPS::IsPresetEmpty(int32 PresetIndex) const
{
    if (PresetIndex < 1 || PresetIndex > 3)
    {
        return true;
    }

    const FString SaveSlotName = TEXT("ArcaneBoardSave");
    const int32 UserIdx = 0;

    if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, UserIdx))
    {
        return true;
    }

    UGS_ArcaneBoardSaveGame* LoadedSaveGame = Cast<UGS_ArcaneBoardSaveGame>(
        UGameplayStatics::LoadGameFromSlot(SaveSlotName, UserIdx));

    if (!LoadedSaveGame || !IsValid(BoardManager))
    {
        return true;
    }

    ECharacterClass CurrClass = BoardManager->CurrClass;

    if (!LoadedSaveGame->SavedRunesByClass.Contains(CurrClass))
    {
        return true;
    }

    const FArcaneBoardPresets& ClassPresets = LoadedSaveGame->SavedRunesByClass[CurrClass];
    const TArray<FPlacedRuneInfo>* TargetPreset = GetPresetArray(ClassPresets, PresetIndex);
    
    return !TargetPreset || TargetPreset->Num() == 0;
}

int32 UGS_ArcaneBoardLPS::GetCurrentPresetIndex() const
{
    return CurrentPresetIndex;
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

TArray<uint8> UGS_ArcaneBoardLPS::GetOwnedRunes() const
{
    return OwnedRuneIDs.Array();
}

void UGS_ArcaneBoardLPS::AddRuneToInventory(uint8 RuneID)
{
    if (RuneID > 0)
    {
        OwnedRuneIDs.Add(RuneID);
        UE_LOG(LogTemp, Log, TEXT("룬 획득: ID=%d, 총 소유 룬 개수: %d"), RuneID, OwnedRuneIDs.Num());

        // 변경사항 저장
        SaveBoardConfig();
    }
}

void UGS_ArcaneBoardLPS::InitializeDefaultRunes()
{
    OwnedRuneIDs.Empty();

    // 기본 룬 2개 (ID 1, 2번 룬)
    OwnedRuneIDs.Add(1);
    OwnedRuneIDs.Add(2);
}

void UGS_ArcaneBoardLPS::SetCurrUIWidget(UGS_ArcaneBoardWidget* Widget)
{
    CurrentUIWidget = Widget;

    if (Widget)
    {
        Widget->OnStatsChanged(RuneSystemStats);
    }
}

void UGS_ArcaneBoardLPS::ClearCurrUIWidget()
{
    CurrentUIWidget = nullptr;
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

void UGS_ArcaneBoardLPS::EnsureRuneInvenInit()
{
    if (OwnedRuneIDs.Num() > 0)
    {
        return;
    }

    FString SaveSlotName = TEXT("ArcaneBoardSave");
    const int32 UserIndex = 0;

    if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, UserIndex))
    {
        UGS_ArcaneBoardSaveGame* LoadedSaveGame = Cast<UGS_ArcaneBoardSaveGame>(
            UGameplayStatics::LoadGameFromSlot(SaveSlotName, UserIndex));

        if (LoadedSaveGame && LoadedSaveGame->OwnedRuneIDs.Num() > 0)
        {
            OwnedRuneIDs = LoadedSaveGame->OwnedRuneIDs;
            UE_LOG(LogTemp, Log, TEXT("기존 룬 인벤토리 로드 성공: %d개"), OwnedRuneIDs.Num());
            return;
        }
    }

    InitializeDefaultRunes();
    UE_LOG(LogTemp, Log, TEXT("기본 룬으로 인벤토리 초기화 완료"));
}

TArray<FPlacedRuneInfo> UGS_ArcaneBoardLPS::LoadPresetData(ECharacterClass CharClass, int32 PresetIndex)
{
    FString SaveSlotName = TEXT("ArcaneBoardSave");
    const int32 UserIndex = 0;

    // 세이브 파일이 없으면 빈 배열 반환
    if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, UserIndex))
    {
        UE_LOG(LogTemp, Log, TEXT("세이브 파일이 없음 - 프리셋 %d를 빈 상태로 설정"), PresetIndex);
        return TArray<FPlacedRuneInfo>();
    }

    UGS_ArcaneBoardSaveGame* LoadedSaveGame = Cast<UGS_ArcaneBoardSaveGame>(
        UGameplayStatics::LoadGameFromSlot(SaveSlotName, UserIndex));

    // 로드 실패하면 빈 배열 반환
    if (!LoadedSaveGame)
    {
        UE_LOG(LogTemp, Warning, TEXT("세이브 파일 로드 실패 - 프리셋 %d를 빈 상태로 설정"), PresetIndex);
        return TArray<FPlacedRuneInfo>();
    }

    // 해당 캐릭터 클래스 데이터가 없으면 빈 배열 반환
    if (!LoadedSaveGame->SavedRunesByClass.Contains(CharClass))
    {
        UE_LOG(LogTemp, Log, TEXT("캐릭터 클래스 %s의 프리셋 데이터가 없음 - 프리셋 %d를 빈 상태로 설정"),
            *UGS_EnumUtils::GetEnumAsString(CharClass), PresetIndex);
        return TArray<FPlacedRuneInfo>();
    }

    const FArcaneBoardPresets& ClassPresets = LoadedSaveGame->SavedRunesByClass[CharClass];
    const TArray<FPlacedRuneInfo>* TargetPreset = GetPresetArray(ClassPresets, PresetIndex);

    if (TargetPreset && TargetPreset->Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("프리셋 데이터 로드 성공 - 클래스: %s, 프리셋: %d, 룬 개수: %d"),
            *UGS_EnumUtils::GetEnumAsString(CharClass), PresetIndex, TargetPreset->Num());
        return *TargetPreset;
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("프리셋 %d가 비어있음 - 빈 상태로 설정"), PresetIndex);
        return TArray<FPlacedRuneInfo>();
    }
}

void UGS_ArcaneBoardLPS::ApplyPresetToBoard(ECharacterClass CharClass, const TArray<FPlacedRuneInfo>& PresetData)
{
    if (!IsValid(BoardManager))
    {
        UE_LOG(LogTemp, Error, TEXT("BoardManager가 유효하지 않음"));
        return;
    }

    BoardManager->LoadSavedData(CharClass, PresetData);
    BoardManager->bHasUnsavedChanges = false;

    UE_LOG(LogTemp, Log, TEXT("보드에 프리셋 적용 완료: %d개 룬"), PresetData.Num());
}

int32 UGS_ArcaneBoardLPS::GetLastUsedPresetIndex()
{
    if (!IsValid(BoardManager))
    {
        return 1;
    }

    FString SaveSlotName = TEXT("ArcaneBoardSave");
    const int32 UserIndex = 0;

    if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, UserIndex))
    {
        UE_LOG(LogTemp, Log, TEXT("세이브 파일이 없음 - 기본 프리셋 1 반환"));
        return 1;
    }

    UGS_ArcaneBoardSaveGame* LoadedSaveGame = Cast<UGS_ArcaneBoardSaveGame>(
        UGameplayStatics::LoadGameFromSlot(SaveSlotName, UserIndex));

    if (!LoadedSaveGame)
    {
        UE_LOG(LogTemp, Warning, TEXT("세이브 파일 로드 실패 - 기본 프리셋 1 반환"));
        return 1;
    }

    ECharacterClass CurrClass = BoardManager->CurrClass;
    if (!LoadedSaveGame->SavedRunesByClass.Contains(CurrClass))
    {
        UE_LOG(LogTemp, Log, TEXT("캐릭터 클래스 데이터가 없음 - 기본 프리셋 1 반환"));
        return 1;
    }

    const FArcaneBoardPresets& ClassPresets = LoadedSaveGame->SavedRunesByClass[CurrClass];
    int32 LastUsedIndex = ClassPresets.LastUsedPresetIndex;

    if (LastUsedIndex >= 1 && LastUsedIndex <= 3)
    {
        UE_LOG(LogTemp, Log, TEXT("최근 사용 프리셋: %d"), LastUsedIndex);
        return LastUsedIndex;
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("잘못된 최근 사용 프리셋 값: %d - 기본 프리셋 1 반환"), LastUsedIndex);
        return 1;
    }
}

UGS_ArcaneBoardSaveGame* UGS_ArcaneBoardLPS::GetOrCreateSaveGame()
{
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
        SaveGameInstance = Cast<UGS_ArcaneBoardSaveGame>(
            UGameplayStatics::CreateSaveGameObject(UGS_ArcaneBoardSaveGame::StaticClass()));
    }

    return SaveGameInstance;
}

const TArray<FPlacedRuneInfo>* UGS_ArcaneBoardLPS::GetPresetArray(const FArcaneBoardPresets& Presets, int32 PresetIndex) const
{
    switch (PresetIndex)
    {
    case 1: return &Presets.Preset1;
    case 2: return &Presets.Preset2;
    case 3: return &Presets.Preset3;
    default: return nullptr;
    }
}

TArray<FPlacedRuneInfo>* UGS_ArcaneBoardLPS::GetPresetArray(FArcaneBoardPresets& Presets, int32 PresetIndex)
{
    return const_cast<TArray<FPlacedRuneInfo>*>(
        static_cast<const UGS_ArcaneBoardLPS*>(this)->GetPresetArray(
            static_cast<const FArcaneBoardPresets&>(Presets), PresetIndex));
}
