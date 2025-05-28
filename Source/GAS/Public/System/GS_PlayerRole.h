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
    SJ_Job1     UMETA(DisplayName = "Seeker Job 1"),
    SJ_Job2     UMETA(DisplayName = "Seeker Job 2"),
    SJ_Job3     UMETA(DisplayName = "Seeker Job 3"),
    SJ_Job4     UMETA(DisplayName = "Seeker Job 4")
};

UENUM(BlueprintType)
enum class EGuardianJob : uint8
{
    GJ_Job1     UMETA(DisplayName = "Guardian Job 1"),
    GJ_Job2     UMETA(DisplayName = "Guardian Job 2")
};

UENUM(BlueprintType)
enum class EGameResult : uint8
{
    GR_InProgress   UMETA(DisplayName = "In Progress"),
    GR_SeekersWon   UMETA(DisplayName = "Seekers Won"),
    GR_SeekersLost  UMETA(DisplayName = "Seekers Lost")
};