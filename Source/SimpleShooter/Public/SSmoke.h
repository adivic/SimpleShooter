// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SGrenade.h"
#include "SSmoke.generated.h"

/**
 * 
 */
UCLASS()
class SIMPLESHOOTER_API ASSmoke : public ASGrenade
{
	GENERATED_BODY()
	

public:
	ASSmoke();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	virtual void Explode() override;

	void DestroySelf();
};
