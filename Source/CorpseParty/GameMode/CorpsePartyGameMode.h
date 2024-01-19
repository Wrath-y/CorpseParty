// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "CorpsePartyGameMode.generated.h"

namespace MatchState
{
	extern CORPSEPARTY_API const FName Cooldown; // Match duration has been reached. Display winner and begin cooldown timer.
}

/**
 * 
 */
UCLASS()
class CORPSEPARTY_API ACorpsePartyGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ACorpsePartyGameMode();
	virtual void Tick(float DeltaTime) override;
	virtual void PlayerEliminated(class ACorpsePartyCharacter* ElimmedCharacter, class ACorpsePartyPlayerController* VictimController, ACorpsePartyPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);

	void PlayerLeftGame(class ACorpsePartyPlayerState* PlayerLeaving);
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage);
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

	float LevelStartingTime = 0.f;

	bool bTeamsMatch = false;
protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;
	
private:
	float CountdownTime = 0.f;
};
