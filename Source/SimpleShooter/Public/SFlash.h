// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SGrenade.h"
#include "SFlash.generated.h"

/**
 * 
 */
UCLASS()
class SIMPLESHOOTER_API ASFlash : public ASGrenade
{
	GENERATED_BODY()
	
public:
	ASFlash();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	virtual void Explode() override;
};
