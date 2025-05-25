#include "System/GameMode/GS_InGameGM.h"
#include "System/GameState/GS_InGameGS.h"
#include "System/GS_PlayerState.h"
#include "System/GS_GameInstance.h"
#include "AI/RTS/GS_RTSController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"

AGS_InGameGM::AGS_InGameGM()
{
    DefaultPawnClass = nullptr;
	GameStateClass = AGS_InGameGS::StaticClass();
    PlayerStateClass = AGS_PlayerState::StaticClass();
    bUseSeamlessTravel = true;
}

void AGS_InGameGM::HandleSeamlessTravelPlayer(AController*& C)
{
    APlayerController* OldPC = Cast<APlayerController>(C);
    if (!OldPC) return;
    
    AGS_PlayerState* OldPS = OldPC->GetPlayerState<AGS_PlayerState>();
    if (!OldPS) return;
    UE_LOG(LogTemp, Warning, TEXT("OldPlayerRole: %s"), *UEnum::GetValueAsString(OldPS->CurrentPlayerRole));

    Super::HandleSeamlessTravelPlayer(C);
    AGS_PlayerState* SamplePS = C->GetPlayerState<AGS_PlayerState>();
    UE_LOG(LogTemp, Warning, TEXT("NewPlayerRole: %s"), *UEnum::GetValueAsString(SamplePS->CurrentPlayerRole));

    /* 2-1.  OldPS->CurrentPlayerRole 로 새 PC/Pawn 클래스 선택 */
    TSubclassOf<APlayerController> NewPCClass;
    TSubclassOf<APawn> NewPawnClass;
    if (OldPS)
    {
        if (OldPS->CurrentPlayerRole == EPlayerRole::PR_Seeker)
        {
            NewPCClass = SeekerControllerClass;
            NewPawnClass = SeekerPawnClass;
        }
        else
        {
            NewPCClass = RTSControllerClass;
            NewPawnClass = RTSPawnClass;
        }
    }
    

    if (!NewPCClass) return;

    /* 2-2.  새 PC 스폰 & PS 자동 생성 */
    APlayerController* NewPC = GetWorld()->SpawnActor<APlayerController>(NewPCClass, FVector(0),FRotator(0));

    if (!NewPC) return;

    /* 2-3.  OldPS → NewPS 로 값 복사 */
    if (AGS_PlayerState* NewPS = NewPC->GetPlayerState<AGS_PlayerState>())
    {
        OldPS->CopyProperties(NewPS);
    }

    /* 2-4.  새 Pawn 스폰 & 빙의 */
    APawn* NewPawn = GetWorld()->SpawnActor<APawn>(NewPawnClass, FVector(500.f, 0.f, 192.f), FRotator(0));
    if (NewPawn)
    {
        NewPC->Possess(NewPawn);
        UE_LOG(LogTemp, Warning, TEXT("##############################Possessed"));
    }

    /* 2-5.  네트워크 연결을 새 PC에 붙임 */
    SwapPlayerControllers(OldPC, NewPC);   // 내부에서 NetConnection 포인터 교체
    UE_LOG(LogTemp, Warning, TEXT("##############################Swaped"));
    C = NewPC;

    OldPC->UnPossess();
    OldPC->Destroy();
}

void AGS_InGameGM::PostSeamlessTravel()
{
    Super::PostSeamlessTravel();
}

void AGS_InGameGM::BeginPlay()
{
    Super::BeginPlay();
    /*if (auto* GI = GetGameInstance<UGS_GameInstance>())
    {
        if (auto* GS = GetGameState<AGS_InGameGS>())
        {
            GS->NumClientsExpected = GI->ExpectedPlayers;
            UE_LOG(LogTemp, Warning, TEXT("[InGameGM] Copied Expected=%d from GI"), GS->NumClientsExpected);
        }
    }

    if (!bSpawnDelegateBound)
    {
        if (auto* GS = GetGameState<AGS_InGameGS>())
        {
            GS->OnAllClientsLoaded.AddUObject(this, &AGS_InGameGM::SpawnAllPlayersSafely);
            UE_LOG(LogTemp, Log, TEXT("[InGameGM] Delegate bound IN HandleSeamlessTravelPlayer"));
            bSpawnDelegateBound = true;
        }
    }*/
}

//void AGS_InGameGM::SpawnAllPlayersSafely()
//{
//    UE_LOG(LogTemp, Warning, TEXT("InGameGM: === SpawnAllPlayersSafely() Executed ==="));
//    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
//    {
//        APlayerController* PC = It->Get();
//        auto* PS = Cast<AGS_PlayerState>(PC->PlayerState);
//        if (!PS) continue;
//
//        /* ---------- (A) 컨트롤러 스왑 ---------- */
//        APlayerController* OldPC = PC; // 이전 PC를 임시 저장
//        if (PS->CurrentPlayerRole == EPlayerRole::PR_Seeker && !PC->IsA(SeekerControllerClass))
//        {
//            APlayerController* NewPC = GetWorld()->SpawnActor<APlayerController>(SeekerControllerClass);
//            if (NewPC)
//            {
//                SwapPlayerControllers(OldPC, NewPC);
//                // OldPC->Destroy();
//                PC = NewPC;
//            }
//            else
//            {
//                UE_LOG(LogTemp, Error, TEXT("Failed to spawn SeekerControllerClass for %s"), *PS->GetPlayerName());
//                continue;
//            }
//        }
//        else if (PS->CurrentPlayerRole == EPlayerRole::PR_Guardian && !PC->IsA(RTSControllerClass))
//        {
//            APlayerController* NewPC = GetWorld()->SpawnActor<APlayerController>(RTSControllerClass);
//            if (NewPC)
//            {
//                SwapPlayerControllers(OldPC, NewPC);
//                // OldPC->Destroy();
//                PC = NewPC;
//            }
//            else
//            {
//                UE_LOG(LogTemp, Error, TEXT("Failed to spawn RTSControllerClass for %s"), *PS->GetPlayerName());
//                continue;
//            }
//        }
//
//        UE_LOG(LogTemp, Log, TEXT("    ▶ %s Role=%s  Pawn? %s (Controller: %s) IsServer: %s"),
//            *PS->GetPlayerName(),
//            *UEnum::GetValueAsString(PS->CurrentPlayerRole),
//            PC->GetPawn() ? TEXT("YES") : TEXT("NO"),
//            *PC->GetName()
//            ,PC->HasAuthority() ? TEXT("YES") : TEXT("NO")
//        );
//
//        /* ---------- (B) Pawn 스폰 / 빙의 ---------- */
//        if (PS->CurrentPlayerRole == EPlayerRole::PR_Seeker)
//        {
//            if (!PC->GetPawn())
//            {
//                AActor* Start = ChoosePlayerStart(PC);
//                FVector Loc = Start ? Start->GetActorLocation() : FVector(0.f, 0.f, 200.f);
//                FRotator Rot = Start ? Start->GetActorRotation() : FRotator::ZeroRotator;
//
//                FActorSpawnParameters SpawnParams;
//                SpawnParams.Owner = PC; // 소유자 설정
//
//                if (APawn* P = GetWorld()->SpawnActor<APawn>(SeekerPawnClass, Loc, Rot, SpawnParams))
//                {
//                    PC->Possess(P);
//                    UE_LOG(LogTemp, Warning, TEXT("        Spawned %s at %s and Possessed by %s"),
//                        *P->GetName(), *Loc.ToString(), *PC->GetName());
//                }
//                else
//                {
//                    UE_LOG(LogTemp, Error, TEXT("        Spawn FAILED for %s"), *PS->GetPlayerName());
//                }
//            }
//        }
//        else
//        {
//            if (PC->GetPawn())
//            {
//                PC->UnPossess();
//            }
//        }
//    }
//}

//void AGS_InGameGM::HandleControllerChangeTriggerEvent() // 트리거 되면 컨트롤러 갈아끼우는 함수
//{
//	if (!HasAuthority())
//	{
//		return;
//	}
//
//	if (!GuardianControllerClass)
//	{
//		UE_LOG(LogTemp, Error, TEXT("GS_InGameGM: GuardianControllerClass is null"));
//		return;
//	}
//
//	UE_LOG(LogTemp, Log, TEXT("GS_InGameGM: Seeker triggered event. Attempting to swap Guardian contoller."));
//	AGS_InGameGS* InGameGS = Cast<AGS_InGameGS>(GetWorld()->GetGameState());
//	if (!InGameGS)
//	{
//		UE_LOG(LogTemp, Error, TEXT("GS_InGameGM: Casting failed to GS_InGameGS"));
//		return;
//	}
//
//	for (APlayerState* PS : InGameGS->PlayerArray)
//	{
//		AGS_PlayerState* SpecificPlayerState = Cast<AGS_PlayerState>(PS);
//		if (SpecificPlayerState && SpecificPlayerState->CurrentPlayerRole == EPlayerRole::PR_Guardian)
//		{
//			APlayerController* OldRTSController = SpecificPlayerState->GetPlayerController();
//			if (OldRTSController && !OldRTSController->IsA(GuardianControllerClass))
//			{
//				APawn* OldRTSPawn = OldRTSController->GetPawn();
//
//			}
//		}
//	}
//}
