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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:
    FTimerHandle AetherExtractTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aether")
	float ExtractionInterval = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Aether")
	float ExtractionAmount = 5.f;

	UPROPERTY()
	TObjectPtr<UGS_AetherComp> CachedAetherComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UGS_StatComp> StatComp;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stat", meta = (AllowPrivateAccess))
	TObjectPtr<UGS_HPTextWidgetComp> HPTextWidgetComp;

	USceneComponent* RootSceneComp;

	UFUNCTION()
	void RegisterRTSController(AGS_RTSController* InController);

	UFUNCTION()
	void HandleAetherCompReady(UGS_AetherComp* AetherComp);

	UFUNCTION()
	void ExtractAether();

	UFUNCTION()
	void InitializeAetherComp();
	//UGS_AetherComp* FindGuardianAetherComp();
	void TakeDamageBySeeker(float DamageAmount, AActor* DamageCauser);
	void DestroyAetherExtractor();

	//HPwidget
	void SetHPTextWidget(UGS_HPText* InHPTextWidget);

	//getter
	FORCEINLINE UGS_StatComp* GetStatComp() const { return StatComp; }
};
