// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SPlayerState.generated.h"

/**
 * Player State class, handles player score during game
 */
UCLASS()
class SIMPLESHOOTER_API ASPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, Category=PlayerState)
	void AddScore(float ScoreDelta);
	
};
