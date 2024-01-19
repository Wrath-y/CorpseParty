// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamsGameMode.h"
#include "CorpseParty/CorpsePartyTypes/Team.h"
#include "CorpseParty/GameState/CorpsePartyGameState.h"
#include "CorpseParty/PlayerController/CorpsePartyPlayerController.h"
#include "CorpseParty/PlayerState/CorpsePartyPlayerState.h"
#include "Kismet/GameplayStatics.h"

ATeamsGameMode::ATeamsGameMode()
{
	bTeamsMatch = true;
}

void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ACorpsePartyGameState* BGameState = Cast<ACorpsePartyGameState>(UGameplayStatics::GetGameState(this));
	if (BGameState)
	{
		ACorpsePartyPlayerState* BPState = NewPlayer->GetPlayerState<ACorpsePartyPlayerState>();
		if (BPState && BPState->GetTeam() == ETeam::ET_NoTeam)
		{
			if (BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
			{
				BGameState->RedTeam.AddUnique(BPState);
				BPState->SetTeam(ETeam::ET_RedTeam);
			}
			else
			{
				BGameState->BlueTeam.AddUnique(BPState);
				BPState->SetTeam(ETeam::ET_BlueTeam);
			}
		}
	}
}

void ATeamsGameMode::Logout(AController* Exiting)
{
	ACorpsePartyGameState* BGameState = Cast<ACorpsePartyGameState>(UGameplayStatics::GetGameState(this));
	ACorpsePartyPlayerState* BPState = Exiting->GetPlayerState<ACorpsePartyPlayerState>();
	if (BGameState && BPState)
	{
		if (BGameState->RedTeam.Contains(BPState))
		{
			BGameState->RedTeam.Remove(BPState);
		}
		if (BGameState->BlueTeam.Contains(BPState))
		{
			BGameState->BlueTeam.Remove(BPState);
		}
	}

}

void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	ACorpsePartyGameState* BGameState = Cast<ACorpsePartyGameState>(UGameplayStatics::GetGameState(this));
	if (BGameState)
	{
		for (auto PState : BGameState->PlayerArray)
		{
			ACorpsePartyPlayerState* BPState = Cast<ACorpsePartyPlayerState>(PState.Get());
			if (BPState && BPState->GetTeam() == ETeam::ET_NoTeam)
			{
				if (BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
				{
					BGameState->RedTeam.AddUnique(BPState);
					BPState->SetTeam(ETeam::ET_RedTeam);
				}
				else
				{
					BGameState->BlueTeam.AddUnique(BPState);
					BPState->SetTeam(ETeam::ET_BlueTeam);
				}
			}
		}
	}
}

float ATeamsGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	ACorpsePartyPlayerState* AttackerPState = Attacker->GetPlayerState<ACorpsePartyPlayerState>();
	ACorpsePartyPlayerState* VictimPState = Victim->GetPlayerState<ACorpsePartyPlayerState>();
	if (AttackerPState == nullptr || VictimPState == nullptr) return BaseDamage;
	if (VictimPState == AttackerPState)
	{
		return BaseDamage;
	}
	if (AttackerPState->GetTeam() == VictimPState->GetTeam())
	{
		return 0.f;
	}
	return BaseDamage;
}

void ATeamsGameMode::PlayerEliminated(ACorpsePartyCharacter* ElimmedCharacter, ACorpsePartyPlayerController* VictimController, ACorpsePartyPlayerController* AttackerController)
{
	Super::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);

	ACorpsePartyGameState* BGameState = Cast<ACorpsePartyGameState>(UGameplayStatics::GetGameState(this));
	ACorpsePartyPlayerState* AttackerPlayerState = AttackerController ? Cast<ACorpsePartyPlayerState>(AttackerController->PlayerState) : nullptr;
	if (BGameState && AttackerPlayerState)
	{
		if (AttackerPlayerState->GetTeam() == ETeam::ET_BlueTeam)
		{
			BGameState->BlueTeamScores();
		}
		if (AttackerPlayerState->GetTeam() == ETeam::ET_RedTeam)
		{
			BGameState->RedTeamScores();
		}
	}
}