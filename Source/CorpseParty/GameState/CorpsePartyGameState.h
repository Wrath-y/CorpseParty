// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "CorpsePartyGameState.generated.h"

/**
 * 
 */
UCLASS()
class CORPSEPARTY_API ACorpsePartyGameState : public AGameState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void UpdateTopScore(class ACorpsePartyPlayerState* ScoringPlayer);

	UPROPERTY(Replicated)
	TArray<ACorpsePartyPlayerState*> TopScoringPlayers;
	
	/** 
	* Teams
	*/

	void RedTeamScores();
	void BlueTeamScores();
	
	TArray<ACorpsePartyPlayerState*> RedTeam;
	TArray<ACorpsePartyPlayerState*> BlueTeam;

	UPROPERTY(ReplicatedUsing = OnRep_RedTeamScore)
	float RedTeamScore = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
	float BlueTeamScore = 0.f;

	UFUNCTION()
	void OnRep_RedTeamScore();

	UFUNCTION()
	void OnRep_BlueTeamScore();
private:

	float TopScore = 0.f;
};
