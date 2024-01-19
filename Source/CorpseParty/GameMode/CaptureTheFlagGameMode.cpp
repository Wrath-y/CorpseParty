// Fill out your copyright notice in the Description page of Project Settings.


#include "CaptureTheFlagGameMode.h"
#include "CorpseParty/Weapon/Flag.h"
#include "CorpseParty/CaptureTheFlag/FlagZone.h"
#include "CorpseParty/GameState/CorpsePartyGameState.h"

void ACaptureTheFlagGameMode::PlayerEliminated(class ACorpsePartyCharacter* ElimmedCharacter, class ACorpsePartyPlayerController* VictimController, ACorpsePartyPlayerController* AttackerController)
{
	ACorpsePartyGameMode::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);
}

void ACaptureTheFlagGameMode::FlagCaptured(AFlag* Flag, AFlagZone* Zone)
{
	bool bValidCapture = Flag->GetTeam() != Zone->Team;
	ACorpsePartyGameState* BGameState = Cast<ACorpsePartyGameState>(GameState);
	if (BGameState)
	{
		if (Zone->Team == ETeam::ET_BlueTeam)
		{
			BGameState->BlueTeamScores();
		}
		if (Zone->Team == ETeam::ET_RedTeam)
		{
			BGameState->RedTeamScores();
		}
	}
}