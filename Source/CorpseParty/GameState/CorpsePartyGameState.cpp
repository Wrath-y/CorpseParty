// Fill out your copyright notice in the Description page of Project Settings.


#include "CorpsePartyGameState.h"
#include "Net/UnrealNetwork.h"
#include "CorpseParty/PlayerState/CorpsePartyPlayerState.h"
#include "CorpseParty/PlayerController/CorpsePartyPlayerController.h"

void ACorpsePartyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACorpsePartyGameState, TopScoringPlayers);
	DOREPLIFETIME(ACorpsePartyGameState, RedTeamScore);
	DOREPLIFETIME(ACorpsePartyGameState, BlueTeamScore);
}

void ACorpsePartyGameState::UpdateTopScore(class ACorpsePartyPlayerState* ScoringPlayer)
{
	if (TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}

void ACorpsePartyGameState::RedTeamScores()
{
	++RedTeamScore;
	ACorpsePartyPlayerController* BPlayer = Cast<ACorpsePartyPlayerController>(GetWorld()->GetFirstPlayerController());
	if (BPlayer)
	{
		BPlayer->SetHUDRedTeamScore(RedTeamScore);
	}
}

void ACorpsePartyGameState::BlueTeamScores()
{
	++BlueTeamScore;
	ACorpsePartyPlayerController* BPlayer = Cast<ACorpsePartyPlayerController>(GetWorld()->GetFirstPlayerController());
	if (BPlayer)
	{
		BPlayer->SetHUDBlueTeamScore(BlueTeamScore);
	}
}

void ACorpsePartyGameState::OnRep_RedTeamScore()
{
	ACorpsePartyPlayerController* BPlayer = Cast<ACorpsePartyPlayerController>(GetWorld()->GetFirstPlayerController());
	if (BPlayer)
	{
		BPlayer->SetHUDRedTeamScore(RedTeamScore);
	}
}

void ACorpsePartyGameState::OnRep_BlueTeamScore()
{
	ACorpsePartyPlayerController* BPlayer = Cast<ACorpsePartyPlayerController>(GetWorld()->GetFirstPlayerController());
	if (BPlayer)
	{
		BPlayer->SetHUDBlueTeamScore(BlueTeamScore);
	}
}