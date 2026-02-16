// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UIConfig.h"

bool UUIConfig::GetInfo(EUIType Type, FUIInfo& OutInfo) const
{
    if (const FUIInfo* Found = UIData.Find(Type))
    {
        OutInfo = *Found;
        return true;
    }
    return false;
}
