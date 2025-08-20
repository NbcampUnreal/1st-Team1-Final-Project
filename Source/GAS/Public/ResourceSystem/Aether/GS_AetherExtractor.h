#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ResourceSystem/Aether/GS_AetherComp.h"
#include "System/GS_PlayerState.h"
#include "Character/Component/GS_StatComp.h"
#include "UI/Character/GS_HPTextWidgetComp.h"
#include "UI/Character/GS_HPText.h"
#include "UI/Character/GS_HPWidget.h"
#include "GS_AetherExtractor.generated.h"


class UGS_StatComp;
class UGS_HPTextWidgetComp;
UCLASS()
class GAS_API AGS_AetherExtractor : public AActor
{
	GENERATED_BODY()
	
public:	
	AGS_AetherExtractor();

protected:
	virtual void BeginPlay() override;

	FTimerHandle AetherExtractTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aether")
	float ExtractionInterval = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aether")
	float ExtractionAmount = 5.f;
	
	UPROPERTY()
	UGS_AetherComp* CachedAetherComp = nullptr;

public:
	UGS_StatComp* StatComp;
	USceneComponent* RootSceneComp;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stat", meta = (AllowPrivateAccess))
	UGS_HPTextWidgetComp* HPTextWidgetComp;

	UFUNCTION()
	void ExtractAether();
	void InitializeAetherComp();
	UGS_AetherComp* FindGuardianAetherComp();
	void TakeDamageBySeeker(float DamageAmount, AActor* DamageCauser);
	void DestroyAetherExtractor();

	//HPwidget
	void SetHPTextWidget(UGS_HPText* InHPTextWidget);
	//void SetHPBarWidget(UGS_HPWidget* InHPBarWidget);

	//getter
	FORCEINLINE UGS_StatComp* GetStatComp() const { return StatComp; }
};
