// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CorpsePartyGameMode.h"
#include "TeamsGameMode.generated.h"

/**
 * 
 */
UCLASS()
class CORPSEPARTY_API ATeamsGameMode : public ACorpsePartyGameMode
{
	GENERATED_BODY()
public:
	ATeamsGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage) override;
	virtual void PlayerEliminated(class ACorpsePartyCharacter* ElimmedCharacter, class ACorpsePartyPlayerController* VictimController, ACorpsePartyPlayerController* AttackerController) override;
protected:
	virtual void HandleMatchHasStarted() override;
};
