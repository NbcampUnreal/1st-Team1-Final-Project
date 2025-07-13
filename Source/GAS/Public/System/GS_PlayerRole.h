#pragma once

#include "CoreMinimal.h"
#include "GS_PlayerRole.generated.h"

UENUM(BlueprintType)
enum class EPlayerRole : uint8
{
    PR_None     UMETA(DisplayName = "None"),
    PR_Seeker   UMETA(DisplayName = "Seeker"),
    PR_Guardian UMETA(DisplayName = "Guardian")
};

UENUM(BlueprintType)
enum class ESeekerJob : uint8
{
    Ares        UMETA(DisplayName = "Ares"),
    Chan        UMETA(DisplayName = "Chan"),
    Merci       UMETA(DisplayName = "Merci"),
    Reina       UMETA(DisplayName = "Reina"),
    End
};

UENUM(BlueprintType)
enum class EGuardianJob : uint8
{
    Drakhar     UMETA(DisplayName = "Drakhar"),
    End
};

UENUM(BlueprintType)
enum class EGameResult : uint8
{
    GR_InProgress   UMETA(DisplayName = "In Progress"),
    GR_SeekersWon   UMETA(DisplayName = "Seekers Won"),
    GR_SeekersLost  UMETA(DisplayName = "Seekers Lost")
};