#pragma once
#include "FSampleActorState.h"

struct FSampleActorUpdate
{
	FSampleActorState State;
	FGuid UUID;
	int64 ServerTimestamp;
};
