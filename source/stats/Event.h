#pragma once

enum EventType
{
	EVENT_ANY = -1,
	EVENT_ENTER,
	EVENT_PICKUP,
	EVENT_UPDATE,
	EVENT_TIMEOUT,
	EVENT_ENCOUNTER,
	EVENT_DIE,
	EVENT_CLEARED,
	EVENT_GENERATE
};

struct Event
{
	EventType type;
	Quest_Scripted* quest;
};
