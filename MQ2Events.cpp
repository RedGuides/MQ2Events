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

#include "../MQ2Plugin.h"
PreSetup("MQ2Events");
PLUGIN_VERSION(2.2);

#define MY_MAX_STRING 25

VOID EventCommand(PSPAWNINFO pChar, PCHAR szLine);
void __stdcall MyEvent(unsigned int ID, void *pData, PBLECHVALUE pValues);
VOID LoadMyEvents();
char TriggerVar1[MAX_STRING], TriggerVar2[MAX_STRING], TriggerVar3[MAX_STRING], TriggerVar4[MAX_STRING], TriggerVar5[MAX_STRING], TriggerVar6[MAX_STRING], TriggerVar7[MAX_STRING], TriggerVar8[MAX_STRING], TriggerVar9[MAX_STRING];
BOOL dataTriggerVar1(PCHAR szIndex, MQ2TYPEVAR &Ret);
BOOL dataTriggerVar2(PCHAR szIndex, MQ2TYPEVAR &Ret);
BOOL dataTriggerVar3(PCHAR szIndex, MQ2TYPEVAR &Ret);
BOOL dataTriggerVar4(PCHAR szIndex, MQ2TYPEVAR &Ret);
BOOL dataTriggerVar5(PCHAR szIndex, MQ2TYPEVAR &Ret);
BOOL dataTriggerVar6(PCHAR szIndex, MQ2TYPEVAR &Ret);
BOOL dataTriggerVar7(PCHAR szIndex, MQ2TYPEVAR &Ret);
BOOL dataTriggerVar8(PCHAR szIndex, MQ2TYPEVAR &Ret);
BOOL dataTriggerVar9(PCHAR szIndex, MQ2TYPEVAR &Ret);
void ClearTriggers();
void AddTrigger();
void OutputTriggers();

Blech *pEventsEvent = 0;
bool InitOnce = TRUE;
bool DEBUGGING = false;
bool EventCommandReady = FALSE;
bool EventsActive = true;
char EventCommandBuffer[MAX_STRING];
unsigned int eventID;

struct stTriggers
{
	unsigned int ID;
	char Name[MY_MAX_STRING];
	stTriggers *pNext;
};
stTriggers* pMyTriggers = NULL;

PLUGIN_API VOID EventsDebug(PSPAWNINFO pChar, PCHAR Cmd)
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

unsigned int __stdcall MQ2DataVariableLookup(char * VarName, char * Value, size_t ValueLen)
{
	strcpy_s(Value, ValueLen, VarName);
	if (!GetCharInfo())
		return strlen(Value);
	return strlen(ParseMacroParameter(GetCharInfo()->pSpawn, Value, ValueLen));
}

PLUGIN_API VOID InitializePlugin(VOID)
{
	DebugSpewAlways("Initializing MQ2Events");
	pEventsEvent = new Blech('#', '|', MQ2DataVariableLookup);
   // Add commands, MQ2Data items, hooks, etc.
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
}

// Called once, when the plugin is to shutdown
PLUGIN_API VOID ShutdownPlugin(VOID)
{
   DebugSpewAlways("Shutting down MQ2Events");

	if (pEventsEvent) {
		pEventsEvent->Reset();
		delete pEventsEvent;
		pEventsEvent = 0;
	}
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
PLUGIN_API VOID OnPulse(VOID)
{
	if (EventCommandReady && gGameState == GAMESTATE_INGAME && EventsActive)
	{
		EventCommandReady = FALSE;
		DoCommand(GetCharInfo()->pSpawn, EventCommandBuffer);
	}
}

PLUGIN_API VOID SetGameState(DWORD GameState)
{
	if (GameState == GAMESTATE_INGAME && InitOnce)
	{
		LoadMyEvents();
		InitOnce = false;
	}
}

PLUGIN_API DWORD OnIncomingChat(PCHAR Line, DWORD Color)
{
	if (gbInZone && pEventsEvent)
	{
		CHAR szLine[MAX_STRING] = { 0 };
		strcpy_s(szLine, Line);
		if (DEBUGGING)
			WriteChatf("MQ2Events::OnIncomingChat: Feed '%s'", Line);
		pEventsEvent->Feed(szLine);
	}
	return 0;
}

VOID Update_INIFileName()
{
	if (GetCharInfo()) {
		sprintf_s(INIFileName, "%s\\MQ2Events_%s.ini", gszINIPath, GetCharInfo()->Name);
		if (!_FileExists(INIFileName))
			sprintf_s(INIFileName, "%s\\MQ2Events.ini", gszINIPath);
	}
	else
		sprintf_s(INIFileName, "%s\\MQ2Events.ini", gszINIPath);
}

VOID EventCommand(PSPAWNINFO pChar, PCHAR szLine)
{
	char Arg1[MAX_STRING];
	char Arg2[MAX_STRING];
	PCHAR Arg3;
	char szTemp[MAX_STRING];

	GetArg(Arg1, szLine, 1, false);
	GetArg(Arg2, szLine, 2, false);
	Arg3 = GetNextArg(szLine, 2);

	if (!_strnicmp(Arg1, "on", 2))
	{
		WriteChatColor("MQ2Events::Events enabled", USERCOLOR_TELL);
		EventsActive = true;
		return;
	}
	else if (!_strnicmp(Arg1, "off", 3))
	{
		WriteChatColor("MQ2Events::Events disabled", USERCOLOR_TELL);
		EventsActive = false;
		return;
	}
	else if (!_strnicmp(Arg1, "load", 4))
	{
		WriteChatColor("MQ2Events::Reloading INI", USERCOLOR_TELL);
		LoadMyEvents();
	}
	else if (!_strnicmp(Arg1, "add", 3))
	{
		if (strlen(Arg2) >= MY_MAX_STRING)
		{
			WriteChatf("Event name too long.  Must be < %d characters.", MY_MAX_STRING);
			return;
		}
		sprintf_s(szTemp, "MQ2Events::Adding [%s]", Arg2);
		WriteChatColor(szTemp, USERCOLOR_TELL);
		Update_INIFileName();
		WritePrivateProfileString(Arg2, "trigger", "", INIFileName);
		WritePrivateProfileString(Arg2, "command", "", INIFileName);
		WriteChatColor(szTemp);
	}
	else if (!_strnicmp(Arg1, "delete", 6))
	{
		if (strlen(Arg2) >= MY_MAX_STRING)
		{
			WriteChatf("Event name too long.  Must be < %d characters.", MY_MAX_STRING);
			return;
		}
		sprintf_s(szTemp, "MQ2Events::Deleting [%s]", Arg2);
		Update_INIFileName();
		WritePrivateProfileString(Arg2, NULL, NULL, INIFileName);
		WriteChatColor(szTemp);
		LoadMyEvents();
	}
	else if (!_strnicmp(Arg1, "settrigger", 10))
	{
		if (strlen(Arg2) >= MY_MAX_STRING)
		{
			WriteChatf("Event name too long.  Must be < %d characters.", MY_MAX_STRING);
			return;
		}
		sprintf_s(szTemp, "MQ2Events::Setting [%s] Trigger == %s", Arg2, Arg3);
		WriteChatColor(szTemp);
		Update_INIFileName();
		WritePrivateProfileString(Arg2, "trigger", Arg3, INIFileName);
	}
	else if (!_strnicmp(Arg1, "setcommand", 10))
	{
		if (strlen(Arg2) >= MY_MAX_STRING)
		{
			WriteChatf("Event name too long.  Must be < %d characters.", MY_MAX_STRING);
			return;
		}
		sprintf_s(szTemp, "MQ2Events::Setting [%s] Command == %s", Arg2, Arg3);
		WriteChatColor(szTemp);
		Update_INIFileName();
		WritePrivateProfileString(Arg2, "command", Arg3, INIFileName);
	}
	else if (!_strnicmp(Arg1, "list", 4))
	{
		int count = 1;
		stTriggers *CurrentTrigger = pMyTriggers;
		while (CurrentTrigger)
		{
			sprintf_s(szTemp, "MQ2Events::Trigger %u--%s", count++, CurrentTrigger->Name);
			WriteChatColor(szTemp);
			CurrentTrigger = CurrentTrigger->pNext;
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

VOID __stdcall MyEvent(UINT ID, VOID *pData, PBLECHVALUE pValues)
{
	char szBuffer[MAX_STRING] = { 0 };

	if (DEBUGGING)
		WriteChatf("MQ2Events::MyEvent(): ID=%u", ID);
	while (pValues)
	{
		if (DEBUGGING)
			WriteChatf("MQ2Events::MyEvent(): Processing pValues, Name='%s', Value='%s'", pValues->Name, pValues->Value);
		if (!strcmp("1", pValues->Name))
			strcpy_s(TriggerVar1, pValues->Value);
		else
			if (!strcmp("2", pValues->Name))
				strcpy_s(TriggerVar2, pValues->Value);
			else
				if (!strcmp("3", pValues->Name))
					strcpy_s(TriggerVar3, pValues->Value);
				else
					if (!strcmp("4", pValues->Name))
						strcpy_s(TriggerVar4, pValues->Value);
					else
						if (!strcmp("5", pValues->Name))
							strcpy_s(TriggerVar5, pValues->Value);
						else
							if (!strcmp("6", pValues->Name))
								strcpy_s(TriggerVar6, pValues->Value);
							else
								if (!strcmp("7", pValues->Name))
									strcpy_s(TriggerVar7, pValues->Value);
								else
									if (!strcmp("8", pValues->Name))
										strcpy_s(TriggerVar8, pValues->Value);
									else
										if (!strcmp("9", pValues->Name))
											strcpy_s(TriggerVar9, pValues->Value);
		pValues = pValues->pNext;
	}
	stTriggers *pCurrentTrigger = pMyTriggers;
	while (pCurrentTrigger)
	{
		if (pCurrentTrigger->ID == ID)
		{
			if (DEBUGGING)
				WriteChatf("MQ2Events::MyEvent(): Found ID %u, setting event ready!", ID);
			Update_INIFileName();
			GetPrivateProfileString(pCurrentTrigger->Name, "command", "", EventCommandBuffer, MAX_STRING, INIFileName);
			EventCommandReady = TRUE;
		}
		pCurrentTrigger = pCurrentTrigger->pNext;
	}
}

VOID ClearTriggers()
{
	stTriggers *pNextTrigger = NULL;
	int count = 0;
	while (pMyTriggers && pEventsEvent)
	{
		DebugSpewAlways("unloading trigger [%s] #%u", pMyTriggers->Name, pMyTriggers->ID);
		pEventsEvent->RemoveEvent(pMyTriggers->ID);
		pNextTrigger = pMyTriggers->pNext;
		free(pMyTriggers);
		pMyTriggers = pNextTrigger;
	}
}

VOID AddTrigger(PCHAR szName, UINT uID, PCHAR szTrigger, PCHAR szCommand)
{
	if (strlen(szName) >= MY_MAX_STRING)
	{
		return;
	}
	stTriggers *pCurrentTrigger = pMyTriggers;
	stTriggers *pNextTrigger = NULL;
	if (pCurrentTrigger)
	{
		while (pCurrentTrigger->pNext)
		{
			pCurrentTrigger = pCurrentTrigger->pNext;
		}
		pCurrentTrigger->pNext = (stTriggers *)malloc(sizeof(stTriggers));
		pCurrentTrigger = pCurrentTrigger->pNext;
		pCurrentTrigger->pNext = NULL;

	}
	else
	{
		pMyTriggers = (stTriggers *)malloc(sizeof(stTriggers));
		pMyTriggers->pNext = NULL;
		pCurrentTrigger = pMyTriggers;
	}
	strcpy_s(pCurrentTrigger->Name, szName);
	pCurrentTrigger->ID = uID;
}

VOID LoadMyEvents()
{
	char szBuffer[MAX_STRING * 10];
	char szName[MY_MAX_STRING];
	unsigned int ID = 100;
	char szTrigger[MAX_STRING];
	char szCommand[MAX_STRING];

	ClearTriggers();
	Update_INIFileName();
	GetPrivateProfileString(NULL, NULL, NULL, szBuffer, MAX_STRING * 10, INIFileName); //szBuffer = Sections
	CHAR *szTriggers;
	szTriggers = szBuffer; //szTriggers = Section
	for (int i = 0; i == 0 || (szBuffer[i - 1] != 0 || szBuffer[i] != 0); i++)
	{
		if (szBuffer[i] == 0)
		{
			if (strncmp(szTriggers, "Settings", 8) && strlen(szTriggers) > 0)
			{
				if (strlen(szTriggers) >= MY_MAX_STRING)
				{
					WriteChatf("Event name \ar%s \axtoo long.  Must be < %d characters.", szTriggers, MY_MAX_STRING);
				}
				else
				{
					strcpy_s(szName, szTriggers);
					GetPrivateProfileString(szTriggers, "trigger", "trigger", szTrigger, MAX_STRING, INIFileName);
					if (pEventsEvent && szTrigger[0])
					{
						ID = pEventsEvent->AddEvent(szTrigger, MyEvent, (void *)0);
						AddTrigger(szName, ID, szTrigger, szCommand);
					}
				}
			}
			szTriggers = &szBuffer[i + 1];
		}
	}
}

BOOL dataTriggerVar1(PCHAR szIndex, MQ2TYPEVAR &Ret)
{
	Ret.Ptr = &TriggerVar1;
	Ret.Type = pStringType;
	return true;
}

BOOL dataTriggerVar2(PCHAR szIndex, MQ2TYPEVAR &Ret)
{
	Ret.Ptr = &TriggerVar2;
	Ret.Type = pStringType;
	return true;
}

BOOL dataTriggerVar3(PCHAR szIndex, MQ2TYPEVAR &Ret)
{
	Ret.Ptr = &TriggerVar3;
	Ret.Type = pStringType;
	return true;
}

BOOL dataTriggerVar4(PCHAR szIndex, MQ2TYPEVAR &Ret)
{
	Ret.Ptr = &TriggerVar4;
	Ret.Type = pStringType;
	return true;
}

BOOL dataTriggerVar5(PCHAR szIndex, MQ2TYPEVAR &Ret)
{
	Ret.Ptr = &TriggerVar5;
	Ret.Type = pStringType;
	return true;
}

BOOL dataTriggerVar6(PCHAR szIndex, MQ2TYPEVAR &Ret)
{
	Ret.Ptr = &TriggerVar6;
	Ret.Type = pStringType;
	return true;
}

BOOL dataTriggerVar7(PCHAR szIndex, MQ2TYPEVAR &Ret)
{
	Ret.Ptr = &TriggerVar7;
	Ret.Type = pStringType;
	return true;
}

BOOL dataTriggerVar8(PCHAR szIndex, MQ2TYPEVAR &Ret)
{
	Ret.Ptr = &TriggerVar8;
	Ret.Type = pStringType;
	return true;
}

BOOL dataTriggerVar9(PCHAR szIndex, MQ2TYPEVAR &Ret)
{
	Ret.Ptr = &TriggerVar9;
	Ret.Type = pStringType;
	return true;
}
