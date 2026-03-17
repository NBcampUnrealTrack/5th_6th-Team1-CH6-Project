// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

enum class EMoveRequestResult : uint8
{
    RequestAccepted,
    AlreadyArrived,
    Failed
};
