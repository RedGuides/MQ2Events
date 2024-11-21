// MQ2Events
// ini driven events plugin
//
// commands:
//    /event <load|delete <name>|settrigger <name> <trigger text>|setcommand <name> <command text>|list>
//
// variables:
//    EventArg1  these are set acording to last event triggered and correspond to #1# .. #9#
//    EventArg2  these are not cleared so you can have one event use #9# and once set it will stay set until another #9# is encountered.
//    EventArg3
//    EventArg4
//    EventArg5
//    EventArg6
//    EventArg7
//    EventArg8
//    EventArg9
//
// ini file: MQ2Events_CharacterName.ini
// [eventname]
// trigger=trigger text
// command=command to execute when triggered
//
// ini example entries
// [enrage]
// trigger=|${Target.DisplayName}| has become ENRAGED#2#
// command=/attack off
//
// [relaytell]
// trigger=#1# tells you, #2#
// command=/tell relaytargetname ${EventArg1} told me, '${EventArg2.Mid[2,${Math.Calc[${EventArg2.Length}-2]}]}'
//
// [group]
// trigger=#1#To join the group, click on the 'FOLLOW' option, or 'DISBAND' to cancel.#2#
// command=/timed ${Math.Calc[3+${Math.Rand[4]}].Int}s /keypress ctrl+i
//
// [raid]
// trigger=#1#To join the raid click the accept button in the raid window or type /raidaccept.#2#
// command=/timed ${Math.Calc[3+${Math.Rand[4]}].Int}s /raidaccept
//
//

#include <mq/Plugin.h>

PreSetup("MQ2Events");
PLUGIN_VERSION(2.2);

static void LoadMyEvents();
static void LoadMyEvents(std::string_view name);
static void DeleteTrigger(std::string_view name);

constexpr auto MY_MAX_STRING = 25;
constexpr int NUM_EVENT_ARGS = 9;

static char TriggerVar[NUM_EVENT_ARGS][MAX_STRING] = { 0 };

Blech* pEventsEvent = nullptr;
bool InitOnce = true;
bool DEBUGGING = false;
bool EventCommandReady = false;
bool EventsActive = true;
std::string EventCommandBuffer;
unsigned int eventID;

struct EventTrigger
{
	std::string name;
	std::string trigger;

	EventTrigger(std::string&& name, std::string&& trigger)
		: name(std::move(name))
		, trigger(std::move(trigger))
	{
	}
};
std::map<int, EventTrigger> s_eventTriggers;

static void EventsDebug(PlayerClient*, const char* Cmd)
{
	char zParm[MAX_STRING];
	GetArg(zParm, Cmd, 1);

	if (zParm[0] == 0) {
		DEBUGGING = !DEBUGGING;
	}
	else if (!_strnicmp(zParm, "on", 2)) {
		DEBUGGING = true;
	}
	else if (!_strnicmp(zParm, "off", 2)) {
		DEBUGGING = false;
	}
	else {
		DEBUGGING = !DEBUGGING;
	}

	WriteChatf("\arMQ2Events\ax::\amDEBUGGING is now %s\ax.", DEBUGGING ? "\aoON" : "\agOFF");
}

static unsigned int CALLBACK MQ2DataVariableLookup(char * VarName, char * Value, size_t ValueLen)
{
	strcpy_s(Value, ValueLen, VarName);
	if (!pLocalPC)
		return static_cast<unsigned int>(strlen(Value));
	return static_cast<unsigned int>(strlen(ParseMacroParameter(Value, ValueLen)));
}

static void Update_INIFileName()
{
	if (pLocalPC && gGameState == GAMESTATE_INGAME)
	{
		sprintf_s(INIFileName, "%s\\MQ2Events_%s.ini", gPathConfig, pLocalPC->Name);

		if (_FileExists(INIFileName))
			return;
	}
	
	sprintf_s(INIFileName, "%s\\MQ2Events.ini", gPathConfig);
}

static void EventCommand(PlayerClient*, const char* szLine)
{
	char Arg1[MAX_STRING];
	char Arg2[MAX_STRING];

	GetArg(Arg1, szLine, 1, false);
	GetArg(Arg2, szLine, 2, false);
	const char* Arg3 = GetNextArg(szLine, 2);

	if (ci_equals(Arg1, "on"))
	{
		WriteChatColor("MQ2Events::Events enabled", USERCOLOR_TELL);
		EventsActive = true;
		return;
	}
	else if (ci_equals(Arg1, "off"))
	{
		WriteChatColor("MQ2Events::Events disabled", USERCOLOR_TELL);
		EventsActive = false;
		return;
	}
	else if (ci_equals(Arg1, "load"))
	{
		WriteChatColor("MQ2Events::Reloading INI", USERCOLOR_TELL);
		LoadMyEvents();
	}
	else if (ci_equals(Arg1, "add"))
	{
		if (strlen(Arg2) >= MY_MAX_STRING)
		{
			WriteChatf("Event name too long. Must be < %d characters.", MY_MAX_STRING);
			return;
		}

		WriteChatColorf("MQ2Events::Adding [%s]", USERCOLOR_TELL, Arg2);

		WritePrivateProfileString(Arg2, "trigger", "", INIFileName);
		WritePrivateProfileString(Arg2, "command", "", INIFileName);
		DeleteTrigger(Arg2); // Overwrote any existing trigger, so remove until new values exist
	}
	else if (!_strnicmp(Arg1, "delete", 6))
	{
		if (strlen(Arg2) >= MY_MAX_STRING)
		{
			WriteChatf("Event name too long. Must be < %d characters.", MY_MAX_STRING);
			return;
		}

		WriteChatColorf("MQ2Events::Deleting [%s]", USERCOLOR_TELL, Arg2);

		WritePrivateProfileString(Arg2, nullptr, nullptr, INIFileName);
		DeleteTrigger(Arg2);
	}
	else if (ci_equals(Arg1, "settrigger"))
	{
		if (strlen(Arg2) >= MY_MAX_STRING)
		{
			WriteChatf("Event name too long. Must be < %d characters.", MY_MAX_STRING);
			return;
		}
	
		WriteChatColorf("MQ2Events::Setting [%s] Trigger == %s", USERCOLOR_TELL, Arg2, Arg3);

		WritePrivateProfileString(Arg2, "trigger", Arg3, INIFileName);
		LoadMyEvents(Arg2);
	}
	else if (ci_equals(Arg1, "setcommand"))
	{
		if (strlen(Arg2) >= MY_MAX_STRING)
		{
			WriteChatf("Event name too long. Must be < %d characters.", MY_MAX_STRING);
			return;
		}

		WriteChatColorf("MQ2Events::Setting [%s] Command == %s", USERCOLOR_TELL, Arg2, Arg3);

		WritePrivateProfileString(Arg2, "command", Arg3, INIFileName);
		LoadMyEvents(Arg2);
	}
	else if (ci_equals(Arg1, "list"))
	{
		int count = 1;

		for (const auto& [_, trigger] : s_eventTriggers)
		{
			WriteChatColorf("MQ2Events::Trigger %u--%s", USERCOLOR_TELL, count++, trigger.name.c_str());
		}
	}
	else
	{
		WriteChatColor("MQ2Events::Usage: /event <off|on|load|delete <name>|settrigger <name> <trigger text>|setcommand <name> <command text>|list>", CONCOLOR_YELLOW);
		WriteChatColor("MQ2Events::Adds Variables ${EventArg1..9} that correspond with #1#..#9# in trigger text", CONCOLOR_YELLOW);
		WriteChatColor("MQ2Events::Example: event = enrage", CONCOLOR_YELLOW);
		WriteChatColor("MQ2Events::Example: triggertext = |${Target.DisplayName}| has become ENRAGED#2#", CONCOLOR_YELLOW);
		WriteChatColor("MQ2Events::Example: commandtext = /attack off", CONCOLOR_YELLOW);
	}
}

void CALLBACK MyEvent(unsigned int ID, void*, PBLECHVALUE pValues)
{
	if (!EventsActive)
		return;

	if (DEBUGGING)
		WriteChatf("MQ2Events::MyEvent(): ID=%u", ID);

	while (pValues)
	{
		if (DEBUGGING)
			WriteChatf("MQ2Events::MyEvent(): Processing pValues, Name='%s', Value='%s'", pValues->Name.c_str(), pValues->Value.c_str());

		int varNum = GetIntFromString(pValues->Name, 0) - 1;
		if (varNum >= 0 && varNum < NUM_EVENT_ARGS)
		{
			strcpy_s(TriggerVar[varNum], pValues->Value.c_str());
			StripTextLinks(TriggerVar[varNum]);
		}

		pValues = pValues->pNext;
	}

	auto iter = s_eventTriggers.find(ID);
	if (iter != s_eventTriggers.end())
	{
		if (DEBUGGING)
			WriteChatf("MQ2Events::MyEvent(): Found ID %u, setting event ready!", ID);

		EventCommandBuffer = GetPrivateProfileString(iter->second.name, "command", "", INIFileName);
		EventCommandReady = !EventCommandBuffer.empty();
	}
}

static void ClearTriggers()
{
	for (const auto& [id, trigger] : s_eventTriggers)
	{
		DebugSpewAlways("unloading trigger [%s] #%u", trigger.name.c_str(), id);

		pEventsEvent->RemoveEvent(id);
	}

	s_eventTriggers.clear();
}

static void DeleteTrigger(std::string_view name)
{
	for (auto iter = s_eventTriggers.begin(); iter != s_eventTriggers.end(); ++iter)
	{
		if (ci_equals(iter->second.name, name))
		{
			pEventsEvent->RemoveEvent(iter->first);

			s_eventTriggers.erase(iter);
			return;
		}
	}
}

static void AddTrigger(std::string&& name, std::string&& trigger)
{
	if (name.length() >= MY_MAX_STRING)
		return;

	unsigned int id = pEventsEvent->AddEvent(trigger.c_str(), MyEvent, nullptr);

	s_eventTriggers.emplace(std::piecewise_construct,
		std::forward_as_tuple(id),
		std::forward_as_tuple(std::move(name), std::move(trigger)));
}

static void LoadMyEvents()
{
	ClearTriggers();

	std::vector<std::string> sectionNames = GetPrivateProfileSections(INIFileName);
	for (auto&& sectionName : sectionNames)
	{
		if (sectionName.length() >= MY_MAX_STRING)
		{
			WriteChatf("Event name \ar%s \axtoo long. Must be < %d characters.", sectionName.c_str(), MY_MAX_STRING);
		}
		else if (!ci_equals(sectionName, "Settings"))
		{
			std::string trigger = GetPrivateProfileString(sectionName, "trigger", "", INIFileName);
			if (!trigger.empty())
			{
				AddTrigger(std::move(sectionName), std::move(trigger));
			}
		}
	}
}

static void LoadMyEvents(std::string_view name)
{
	// Load a single event
	DeleteTrigger(name);

	std::string eventName{ name };
	std::string trigger = GetPrivateProfileString(eventName, "trigger", "", INIFileName);

	if (!trigger.empty())
	{
		AddTrigger(std::move(eventName), std::move(trigger));
	}
}


static bool dataTriggerVar1(const char* szIndex, MQTypeVar& Ret)
{
	Ret.Ptr = TriggerVar[0];
	Ret.Type = mq::datatypes::pStringType;
	return true;
}

static bool dataTriggerVar2(const char* szIndex, MQTypeVar& Ret)
{
	Ret.Ptr = TriggerVar[1];
	Ret.Type = mq::datatypes::pStringType;
	return true;
}

static bool dataTriggerVar3(const char* szIndex, MQTypeVar& Ret)
{
	Ret.Ptr = TriggerVar[2];
	Ret.Type = mq::datatypes::pStringType;
	return true;
}

static bool dataTriggerVar4(const char* szIndex, MQTypeVar& Ret)
{
	Ret.Ptr = TriggerVar[3];
	Ret.Type = mq::datatypes::pStringType;
	return true;
}

static bool dataTriggerVar5(const char* szIndex, MQTypeVar& Ret)
{
	Ret.Ptr = TriggerVar[4];
	Ret.Type = mq::datatypes::pStringType;
	return true;
}

static bool dataTriggerVar6(const char* szIndex, MQTypeVar& Ret)
{
	Ret.Ptr = TriggerVar[5];
	Ret.Type = mq::datatypes::pStringType;
	return true;
}

static bool dataTriggerVar7(const char* szIndex, MQTypeVar& Ret)
{
	Ret.Ptr = &TriggerVar[6];
	Ret.Type = mq::datatypes::pStringType;
	return true;
}

static bool dataTriggerVar8(const char* szIndex, MQTypeVar& Ret)
{
	Ret.Ptr = TriggerVar[7];
	Ret.Type = mq::datatypes::pStringType;
	return true;
}

static bool dataTriggerVar9(const char* szIndex, MQTypeVar& Ret)
{
	Ret.Ptr = TriggerVar[8];
	Ret.Type = mq::datatypes::pStringType;
	return true;
}

PLUGIN_API void InitializePlugin()
{
	pEventsEvent = new Blech('#', '|', MQ2DataVariableLookup);

	AddCommand("/event", EventCommand);
	AddCommand("/eventdebug", EventsDebug);
	AddMQ2Data("EventArg1", dataTriggerVar1);
	AddMQ2Data("EventArg2", dataTriggerVar2);
	AddMQ2Data("EventArg3", dataTriggerVar3);
	AddMQ2Data("EventArg4", dataTriggerVar4);
	AddMQ2Data("EventArg5", dataTriggerVar5);
	AddMQ2Data("EventArg6", dataTriggerVar6);
	AddMQ2Data("EventArg7", dataTriggerVar7);
	AddMQ2Data("EventArg8", dataTriggerVar8);
	AddMQ2Data("EventArg9", dataTriggerVar9);

	Update_INIFileName();
}

PLUGIN_API void ShutdownPlugin()
{
	delete pEventsEvent;
	pEventsEvent = nullptr;

	RemoveCommand("/event");
	RemoveCommand("/eventdebug");
	RemoveMQ2Data("EventArg1");
	RemoveMQ2Data("EventArg2");
	RemoveMQ2Data("EventArg3");
	RemoveMQ2Data("EventArg4");
	RemoveMQ2Data("EventArg5");
	RemoveMQ2Data("EventArg6");
	RemoveMQ2Data("EventArg7");
	RemoveMQ2Data("EventArg8");
	RemoveMQ2Data("EventArg9");

	ClearTriggers();
}

// This is called every time MQ pulses
PLUGIN_API void OnPulse()
{
	if (EventCommandReady && gGameState == GAMESTATE_INGAME && EventsActive)
	{
		EventCommandReady = false;

		EzCommand(EventCommandBuffer.c_str());
		EventCommandBuffer.clear();
	}
}

PLUGIN_API void SetGameState(DWORD GameState)
{
	Update_INIFileName();

	if (GameState == GAMESTATE_INGAME && InitOnce)
	{
		LoadMyEvents();
		InitOnce = false;
	}

	if (GameState == GAMESTATE_CHARSELECT)
	{
		InitOnce = true;
	}
}

PLUGIN_API DWORD OnIncomingChat(const char* Line, DWORD Color)
{
	if (gbInZone && pEventsEvent)
	{
		char szLine[MAX_STRING] = { 0 };
		strcpy_s(szLine, Line);

		if (DEBUGGING)
			WriteChatf("MQ2Events::OnIncomingChat: Feed '%s'", Line);

		pEventsEvent->Feed(szLine);
	}

	return 0;
}
