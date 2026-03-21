#include "Multiplayer/PlayerColorSubsystem.h"

int32 UPlayerColorSubsystem::GetColorIndex(FUniqueNetIdRepl UserId)
{
	if (NetIdColorIdxMap.Contains(UserId) == true)
		return NetIdColorIdxMap[UserId];

	// 이미 8개 사용되고 있는 상태면
	if (UsingColorBitmap == 0xFF)
		return -1;

	int32 NewIdx = -1;
	for (int32 Idx = 0; Idx < 8; ++Idx)
	{
		if ((UsingColorBitmap & (1 << Idx)) == 0)
		{
			NewIdx = Idx;
			break;
		}
	}

	if (NewIdx != -1)
	{
		UsingColorBitmap |= (1 << NewIdx);
		NetIdColorIdxMap.Add(UserId, NewIdx);
	}

	return NewIdx;
}

void UPlayerColorSubsystem::ReleaseColorIndex(FUniqueNetIdRepl UserId)
{
	if (NetIdColorIdxMap.Contains(UserId) == false)
		return;

	int32 ColorIdx = NetIdColorIdxMap[UserId];

	UsingColorBitmap &= ~(1 << ColorIdx);
	NetIdColorIdxMap.Remove(UserId);
}
