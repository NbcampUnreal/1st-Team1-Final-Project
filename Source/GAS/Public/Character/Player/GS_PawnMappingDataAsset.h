#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "System/GS_PlayerRole.h"
#include "GS_PawnMappingDataAsset.generated.h"

UCLASS(BlueprintType)
class GAS_API UGS_PawnMappingDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditDefaultsOnly, Category = "Pawn Mapping|Seeker")
    TMap<ESeekerJob, TSubclassOf<APawn>> SeekerPawnClasses;

    UPROPERTY(EditDefaultsOnly, Category = "Pawn Mapping|Guardian")
    TMap<EGuardianJob, TSubclassOf<APawn>> GuardianPawnClasses;

    // 필요하다면 역할에 따른 기본 폰 (직업 선택 전 또는 매핑 실패 시)
    UPROPERTY(EditDefaultsOnly, Category = "Pawn Mapping")
    TSubclassOf<APawn> DefaultSeekerPawn;

    UPROPERTY(EditDefaultsOnly, Category = "Pawn Mapping")
    TSubclassOf<APawn> DefaultGuardianPawn;
	
	
};
