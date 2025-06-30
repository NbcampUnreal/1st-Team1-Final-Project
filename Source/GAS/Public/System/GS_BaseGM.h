#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GS_BaseGM.generated.h"

struct FDESaveData;

UCLASS()
class GAS_API AGS_BaseGM : public AGameMode
{
	GENERATED_BODY()

protected:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	UPROPERTY()
	TSet<TObjectPtr<APlayerState>> ReadyPlayers;
	
public:
	/**모든 플레이어 준비완료되면 게임 시작 브로드캐스트*/
	void StartMatchWhenAllReady();
	/** 개별 클라이언트가 완전히 준비되었을 때 호출되는 함수*/
	void NotifyPlayerIsReady(AController* PlayerController);
};