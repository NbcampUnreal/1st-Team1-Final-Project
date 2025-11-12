#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GS_SeamlessTravelLoadingSubsystem.generated.h"

class UUserWidget;
class ULocalPlayer;

/**
 * Seamless Travel 구간 전체를 가리는 풀스크린 커버 전용 Subsystem.
 * GameInstance에 붙어 있어서 Level / GameMode / PlayerController 교체와 무관하게 유지됨.
 */
UCLASS()
class GAS_API UGS_SeamlessTravelLoadingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UGS_SeamlessTravelLoadingSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SeamlessLoading")
	TSubclassOf<UUserWidget> LoadingCoverWidgetClass;

	UFUNCTION(BlueprintCallable, Category="SeamlessLoading")
	void ShowLoadingCover(ULocalPlayer* LP);

	UFUNCTION(BlueprintCallable, Category="SeamlessLoading")
	void HideLoadingCover();

	bool IsCoverVisible() const { return bIsCoverVisible; }

private:
	UPROPERTY()
	UUserWidget* LoadingCoverWidget;

	bool bIsCoverVisible;

	ULocalPlayer* GetPrimaryLocalPlayer() const;
};