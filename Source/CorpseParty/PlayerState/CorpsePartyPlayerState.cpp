// Fill out your copyright notice in the Description page of Project Settings.


#include "CorpsePartyPlayerState.h"
#include "CorpseParty/Character/CorpsePartyCharacter.h"
#include "CorpseParty/PlayerController/CorpsePartyPlayerController.h"
#include "Net/UnrealNetwork.h"

void ACorpsePartyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACorpsePartyPlayerState, Defeats);
	DOREPLIFETIME(ACorpsePartyPlayerState, Team);
}

void ACorpsePartyPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);
	Character = Character == nullptr ? Cast<ACorpsePartyCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ACorpsePartyPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void ACorpsePartyPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = Character == nullptr ? Cast<ACorpsePartyCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ACorpsePartyPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void ACorpsePartyPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;
	
	Character = !IsValid(Character) ? Cast<ACorpsePartyCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = !IsValid(Controller) ? Cast<ACorpsePartyPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void ACorpsePartyPlayerState::OnRep_Defeats()
{
	Character = !IsValid(Character) ? Cast<ACorpsePartyCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = !IsValid(Controller) ? Cast<ACorpsePartyPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void ACorpsePartyPlayerState::SetTeam(ETeam TeamToSet)
{
	Team = TeamToSet;

	ACorpsePartyCharacter* BCharacter = Cast <ACorpsePartyCharacter>(GetPawn());
	if (BCharacter)
	{
		BCharacter->SetTeamColor(Team);
	}
}

void ACorpsePartyPlayerState::OnRep_Team()
{
	ACorpsePartyCharacter* BCharacter = Cast <ACorpsePartyCharacter>(GetPawn());
	if (BCharacter)
	{
		BCharacter->SetTeamColor(Team);
	}
}