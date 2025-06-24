#include "System/GS_BaseGM.h"

#include "NavigationSystem.h"
#include "Character/Player/Monster/GS_Monster.h"
#include "DungeonEditor/Data/GS_DungeonEditorSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "System/GS_GameInstance.h"

void AGS_BaseGM::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (UGS_GameInstance* GI = GetGameInstance<UGS_GameInstance>())
    {
        GI->OnPlayerCountChanged.Broadcast();
    }
}

void AGS_BaseGM::Logout(AController* Exiting)
{
    if (UGS_GameInstance* GI = GetGameInstance<UGS_GameInstance>())
    {
        GI->OnPlayerCountChanged.Broadcast();
    }

    Super::Logout(Exiting);
}

// --- [ LoadGame ] ---
void AGS_BaseGM::LoadGameLogic()
{
    // 1. 세이브 파일 존재 여부 확인
    if (!UGameplayStatics::DoesSaveGameExist(CurrentSaveSlotName, 0))
    {
        UE_LOG(LogTemp, Error, TEXT("SaveGame '%s' does not exist. Cannot load."), *CurrentSaveSlotName);
        return;
    }

    // 3. 파일로부터 데이터 로드 및 형변환(Cast)
    UGS_DungeonEditorSaveGame* LoadGameObject = Cast<UGS_DungeonEditorSaveGame>(UGameplayStatics::LoadGameFromSlot(CurrentSaveSlotName, 0));

    if (!IsValid(LoadGameObject))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load SaveGame from slot '%s'. File might be corrupted."), *CurrentSaveSlotName);
        return;
    }

    // 4. 로드한 데이터를 기반으로 "지형" 액터 스폰 (몬스터 제외)
    UWorld* World = GetWorld();
    if (IsValid(World))
    {
        for (const FDESaveData& ObjectData : LoadGameObject->GetSaveDatas())
        {
            if (ObjectData.SpawnActorClass)
            {
                if (!Cast<AGS_Monster>(ObjectData.SpawnActorClass))
                {
                    FActorSpawnParameters SpawnParams;
                    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
                    World->SpawnActor<AActor>(ObjectData.SpawnActorClass, ObjectData.SpawnTransform, SpawnParams);
                }
            }
        }
    }

    // 5. [핵심] 내비메시 재빌드 요청
    UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (IsValid(NavSystem))
    {
        UE_LOG(LogTemp, Warning, TEXT("Requesting NavMesh rebuild..."));
        // 모든 내비메시를 새로 빌드하도록 요청합니다. 이 작업은 즉시 끝나지 않습니다.
        NavSystem->Build();

        // 빌드가 완료되었는지 0.1초마다 확인하는 타이머를 시작합니다.
        GetWorld()->GetTimerManager().SetTimer(
            NavMeshBuildTimerHandle, 
            this, 
            &AGS_BaseGM::CheckNavMeshBuildStatus, 
            0.1f, 
            true); // true: 반복 실행
    }
}

void AGS_BaseGM::CheckNavMeshBuildStatus()
{
    UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    // 내비메시 빌드가 진행 중이 아닌지(=완료되었는지) 확인합니다.
    if (IsValid(NavSystem) && !NavSystem->IsNavigationBuildInProgress())
    {
        UE_LOG(LogTemp, Warning, TEXT("NavMesh build is complete!"));
        // 타이머를 정지시킵니다.
        GetWorld()->GetTimerManager().ClearTimer(NavMeshBuildTimerHandle);
        // 빌드가 완료되었으므로 다음 작업을 수행하는 함수를 호출합니다.
        OnNavMeshBuildComplete();
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("... Waiting for NavMesh build to finish ..."));
    }
}


void AGS_BaseGM::OnNavMeshBuildComplete()
{
    UE_LOG(LogTemp, Warning, TEXT("Spawning AI characters on the new NavMesh."));

    // 1. 세이브 파일 존재 여부 확인
    if (!UGameplayStatics::DoesSaveGameExist(CurrentSaveSlotName, 0))
    {
        UE_LOG(LogTemp, Error, TEXT("SaveGame '%s' does not exist. Cannot load."), *CurrentSaveSlotName);
        return;
    }

    // 3. 파일로부터 데이터 로드 및 형변환(Cast)
    UGS_DungeonEditorSaveGame* LoadGameObject = Cast<UGS_DungeonEditorSaveGame>(UGameplayStatics::LoadGameFromSlot(CurrentSaveSlotName, 0));

    if (!IsValid(LoadGameObject))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load SaveGame from slot '%s'. File might be corrupted."), *CurrentSaveSlotName);
        return;
    }

    // 4. 로드한 데이터를 기반으로 "몬스터" 액터 스폰
    UWorld* World = GetWorld();
    if (IsValid(World))
    {
        for (const FDESaveData& ObjectData : LoadGameObject->GetSaveDatas())
        {
            if (ObjectData.SpawnActorClass)
            {
                if (Cast<AGS_Monster>(ObjectData.SpawnActorClass))
                {
                    FActorSpawnParameters SpawnParams;
                    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
                    World->SpawnActor<AActor>(ObjectData.SpawnActorClass, ObjectData.SpawnTransform, SpawnParams);
                }
            }
        }
    }
}