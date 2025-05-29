#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GS_TrapData.generated.h"

//함정 배치 위치 분류
UENUM(BlueprintType)
enum class ETrapPlacement : uint8
{
    Floor      UMETA(DisplayName = "Floor"),
    Wall       UMETA(DisplayName = "Wall"),
    Ceiling    UMETA(DisplayName = "Ceiling")
};


//함정 효과 분류(우선 각각의 효과 체크 하는 것만)
USTRUCT(BlueprintType)
struct FTrapEffect
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Damage = 0.f;

    //slow
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSlow = false;

    //knockback
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bKnockback = false;

    //push
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPush = false;


    //dot damage
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bDoT = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="bDoT"))
    int32 DamageCount = 3;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="bDoT"))
    float DamageInterval = 1.0f;

};

USTRUCT(BlueprintType)
struct FTrapData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrapID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ETrapPlacement Placement = ETrapPlacement::Wall;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTrapEffect Effect;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//TSoftObjectPtr<UStaticMesh> TrapMesh;
};
