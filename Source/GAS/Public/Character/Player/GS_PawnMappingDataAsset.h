#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "System/GS_PlayerRole.h"
#include "Engine/SkeletalMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimMontage.h"
#include "GS_PawnMappingDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FWeaponMeshPair
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName SocketName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USkeletalMesh* WeaponSkeletalMeshClass;
};

USTRUCT(BlueprintType)
struct FAssetToSpawn
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
    TSubclassOf<APawn> PawnClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
    TSubclassOf<AActor> DisplayActorClass; // SKM 레플리케이션 용

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
    USkeletalMesh* SkeletalMeshClass;
	
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
    TSubclassOf<UAnimInstance> Lobby_AnimBlueprintClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
    TObjectPtr<UAnimationAsset> WinPose;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
    TObjectPtr<UAnimationAsset> LosePose;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
    TObjectPtr<UTexture2D> AvatarTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup")
	TArray<FWeaponMeshPair> WeaponMeshList;
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
