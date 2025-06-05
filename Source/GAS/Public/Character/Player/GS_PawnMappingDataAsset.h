#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "System/GS_PlayerRole.h"
#include "Engine/SkeletalMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "GS_PawnMappingDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FAssetToSpawn
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
    TSubclassOf<APawn> PawnClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
    TSubclassOf<AActor> DefaultActor;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
    TObjectPtr<UAnimMontage> ReadyPose;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
    TObjectPtr<UAnimMontage> WinPose;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
    TObjectPtr<UAnimMontage> LosePose;
};

UCLASS(BlueprintType)
class GAS_API UGS_PawnMappingDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditDefaultsOnly, Category = "Pawn Mapping|Seeker")
    TMap<ESeekerJob, FAssetToSpawn> SeekerPawnClasses;

    UPROPERTY(EditDefaultsOnly, Category = "Pawn Mapping|Guardian")
    TMap<EGuardianJob, FAssetToSpawn> GuardianPawnClasses;

    // 필요하다면 역할에 따른 기본 폰 (직업 선택 전 또는 매핑 실패 시)
    UPROPERTY(EditDefaultsOnly, Category = "Pawn Mapping")
    TSubclassOf<APawn> DefaultSeekerPawn;

    UPROPERTY(EditDefaultsOnly, Category = "Pawn Mapping")
    TSubclassOf<APawn> DefaultGuardianPawn;
	
	
};
