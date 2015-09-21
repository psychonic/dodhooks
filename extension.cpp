#include "extension.h"
#include "CDetour/detours.h"
#include "vglobals.h"
#include "natives.h"

CDODHooks g_Interface;
SMEXT_LINK(&g_Interface)

CDetour *g_pDetVoiceCommand			= NULL;
CDetour *g_pDetJoinClass			= NULL;
CDetour *g_pDetPopHelmet			= NULL;
CDetour *g_pDetRespawn				= NULL;
CDetour *g_pDetAddWaveTime			= NULL;
CDetour *g_pDetSetWinningTeam		= NULL;
CDetour *g_pDetRoundState			= NULL;
CDetour *g_pDetPlayerState			= NULL;
CDetour *g_pDetBombTargetState		= NULL;

IForward *g_pFwdVoiceCommand		= NULL;
IForward *g_pFwdJoinClass			= NULL;
IForward *g_pFwdPopHelmet			= NULL;
IForward *g_pFwdRespawn				= NULL;
IForward *g_pFwdAddWaveTime			= NULL;
IForward *g_pFwdSetWinningTeam		= NULL;
IForward *g_pFwdRoundState			= NULL;
IForward *g_pFwdPlayerState			= NULL;
IForward *g_pFwdBombTargetState		= NULL;

IGameConfig *g_pGameConf = NULL;

ICvar *g_pCvar = NULL;
ConCommand *g_pKillCmd = NULL;
INetworkStringTableContainer *netstringtables = NULL;

CGlobalVars *g_pGlobals = NULL;

IBinTools *g_pBinTools = NULL;
ISDKTools *g_pSDKTools = NULL;

IServerGameEnts *g_pGameEnts = NULL;
IServerGameClients *g_pGameClients = NULL;

CSharedEdictChangeInfo *g_pSharedChangeInfo = NULL;

uint32 g_iOffset_PlayerClass;
uint32 g_iOffset_DesiredPlayerClass;

uint32 g_iOffset_NumControlPoints;
uint32 g_iOffset_AlliesIcons;
uint32 g_iOffset_AxisIcons;
uint32 g_iOffset_NeutralIcons;
uint32 g_iOffset_TimerCapIcons;
uint32 g_iOffset_BombedIcons;
uint32 g_iOffset_CPIsVisible;

uint32 g_iOffset_TimerPaused;
uint32 g_iOffset_TimeRemaining;
uint32 g_iOffset_TimerEndTime;

float g_fKillCmdBlockTime[DOD_MAXPLAYERS + 1];
uint32 g_iCmdClient;

SH_DECL_HOOK1_void(ConCommand, Dispatch, SH_NOATTRIB, false, const CCommand &);
SH_DECL_HOOK1_void(IServerGameClients, SetCommandClient, SH_NOATTRIB, false, int);

DETOUR_DECL_MEMBER1(VoiceCommand, void, int, iVoiceCommand)
{
	cell_t Result = Pl_Continue;

	g_pFwdVoiceCommand->PushCell(g_pEngine->IndexOfEdict(g_pGameEnts->BaseEntityToEdict((CBaseEntity *)this)));
	g_pFwdVoiceCommand->PushCellByRef(&iVoiceCommand);
	g_pFwdVoiceCommand->Execute(&Result);

	if (Result >= Pl_Handled)
	{
		return;
	}

	DETOUR_MEMBER_CALL(VoiceCommand)(iVoiceCommand);
}

DETOUR_DECL_MEMBER1(JoinClass, void, int, iPlayerClass)
{
	uint32 client = g_pEngine->IndexOfEdict(g_pGameEnts->BaseEntityToEdict((CBaseEntity *)this));

	cell_t Result = Pl_Continue;

	g_pFwdJoinClass->PushCell(client);
	g_pFwdJoinClass->PushCellByRef(&iPlayerClass);
	g_pFwdJoinClass->Execute(&Result);
	
	if (Result >= Pl_Handled)
	{
		g_fKillCmdBlockTime[client] = g_pGlobals->curtime + 0.1;

		return;
	}

	DETOUR_MEMBER_CALL(JoinClass)(iPlayerClass);
}

DETOUR_DECL_MEMBER2(PopHelmet, void, Vector, vecVelocity, Vector, vecOrigin)
{
	cell_t Result = Pl_Continue;

	g_pFwdPopHelmet->PushCell(g_pEngine->IndexOfEdict(g_pGameEnts->BaseEntityToEdict((CBaseEntity *)this)));
	g_pFwdPopHelmet->PushArray((cell_t *)&vecVelocity, 3, SM_PARAM_COPYBACK);
	g_pFwdPopHelmet->PushArray((cell_t *)&vecOrigin, 3, SM_PARAM_COPYBACK);
	g_pFwdPopHelmet->Execute(&Result);
	
	if (Result >= Pl_Handled)
	{
		return;
	}

	DETOUR_MEMBER_CALL(PopHelmet)(vecVelocity, vecOrigin);
}

DETOUR_DECL_MEMBER0(Respawn, void)
{
	cell_t Result = Pl_Continue;
	g_pFwdRespawn->PushCell(g_pEngine->IndexOfEdict(g_pGameEnts->BaseEntityToEdict((CBaseEntity *)this)));
	g_pFwdRespawn->Execute(&Result);

	if (Result >= Pl_Handled)
	{
		return;
	}

	DETOUR_MEMBER_CALL(Respawn)();
}

DETOUR_DECL_MEMBER2(AddWaveTime, void, int, iTeamIndex, float, fDelay)
{
	cell_t Result = Pl_Continue;
	g_pFwdAddWaveTime->PushCell(iTeamIndex);
	g_pFwdAddWaveTime->PushFloatByRef(&fDelay);
	g_pFwdAddWaveTime->Execute(&Result);

	if (Result >= Pl_Handled)
	{
		return;
	}

	DETOUR_MEMBER_CALL(AddWaveTime)(iTeamIndex, fDelay);
}

DETOUR_DECL_MEMBER1(SetWinningTeam, void, int, iTeamIndex)
{
	cell_t Result = Pl_Continue;
	g_pFwdSetWinningTeam->PushCell(iTeamIndex);
	g_pFwdSetWinningTeam->Execute(&Result);

	if (Result >= Pl_Handled)
	{
		return;
	}

	DETOUR_MEMBER_CALL(SetWinningTeam)(iTeamIndex);
}

DETOUR_DECL_MEMBER1(RoundState, void, int, iRoundState)
{
	static uint8 iPreviousRoundState = -1;

	if (iRoundState == iPreviousRoundState)
	{
		return;
	}

	cell_t Result = Pl_Continue;
	g_pFwdRoundState->PushCellByRef(&iRoundState);
	g_pFwdRoundState->Execute(&Result);

	if (Result >= Pl_Handled)
	{
		iPreviousRoundState = iRoundState;

		return;
	}

	DETOUR_MEMBER_CALL(RoundState)(iRoundState);
}

DETOUR_DECL_MEMBER1(PlayerState, void, int, iPlayerState)
{
	cell_t Result = Pl_Continue;
	g_pFwdPlayerState->PushCell(g_pEngine->IndexOfEdict(g_pGameEnts->BaseEntityToEdict((CBaseEntity *)this)));
	g_pFwdPlayerState->PushCellByRef(&iPlayerState);
	g_pFwdPlayerState->Execute(&Result);

	if (Result >= Pl_Handled)
	{
		return;
	}

	DETOUR_MEMBER_CALL(PlayerState)(iPlayerState);
}

DETOUR_DECL_MEMBER1(BombTargetState, void, int, iBombTargetState)
{
	cell_t Result = Pl_Continue;
	g_pFwdBombTargetState->PushCell(g_pEngine->IndexOfEdict(g_pGameEnts->BaseEntityToEdict((CBaseEntity *)this)));
	g_pFwdBombTargetState->PushCellByRef(&iBombTargetState);
	g_pFwdBombTargetState->Execute(&Result);

	if (Result >= Pl_Handled)
	{
		return;
	}

	DETOUR_MEMBER_CALL(BombTargetState)(iBombTargetState);
}

void OnKillCommand(const CCommand &command)
{
	RETURN_META(g_fKillCmdBlockTime[g_iCmdClient] > g_pGlobals->curtime ? MRES_SUPERCEDE : MRES_IGNORED);
}

void CDODHooks::OnSetCommandClient(int client)
{
	g_iCmdClient = client + 1;
}

bool CDODHooks::SDK_OnLoad(char *error, size_t maxlength, bool late)
{
	sharesys->AddDependency(myself, "bintools.ext", true, true);
	sharesys->AddDependency(myself, "sdktools.ext", true, true);
	sharesys->AddNatives(myself, g_Natives);

	char szConfigError[255] = "";

	if (!g_pGameConfs->LoadGameConfigFile("dodhooks", &g_pGameConf, szConfigError, sizeof(szConfigError)))
	{
		if (szConfigError[0])
		{
			snprintf(error, maxlength, "Fatal Error: Unable to open file: \"dodhooks.txt\" - %s", szConfigError);
		}

		return false;
	}

	CDetourManager::Init(g_pSM->GetScriptingEngine(), g_pGameConf);

	CREATE_DETOUR(g_pDetVoiceCommand,		VoiceCommand,		"VoiceCommand");
	CREATE_DETOUR(g_pDetJoinClass,			JoinClass,			"JoinClass");
	CREATE_DETOUR(g_pDetPopHelmet,			PopHelmet,			"PopHelmet");
	CREATE_DETOUR(g_pDetRespawn,			Respawn,			"DODRespawn");
	CREATE_DETOUR(g_pDetAddWaveTime,		AddWaveTime,		"AddWaveTime");
	CREATE_DETOUR(g_pDetSetWinningTeam,		SetWinningTeam,		"SetWinningTeam");
	CREATE_DETOUR(g_pDetRoundState,			RoundState,			"RoundState");
	CREATE_DETOUR(g_pDetPlayerState,		PlayerState,		"PlayerState");
	CREATE_DETOUR(g_pDetBombTargetState,	BombTargetState,	"BombTargetState");

	return true;
}

void CDODHooks::SDK_OnUnload()
{
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, SetCommandClient, g_pGameClients, this, &CDODHooks::OnSetCommandClient, false);

	SH_REMOVE_HOOK_STATICFUNC(ConCommand, Dispatch, g_pKillCmd, OnKillCommand, false);

	g_pGameConfs->CloseGameConfigFile(g_pGameConf);
	
	g_pForwards->ReleaseForward(g_pFwdVoiceCommand);
	g_pForwards->ReleaseForward(g_pFwdJoinClass);
	g_pForwards->ReleaseForward(g_pFwdPopHelmet);
	g_pForwards->ReleaseForward(g_pFwdRespawn);
	g_pForwards->ReleaseForward(g_pFwdAddWaveTime);
	g_pForwards->ReleaseForward(g_pFwdSetWinningTeam);
	g_pForwards->ReleaseForward(g_pFwdRoundState);
	g_pForwards->ReleaseForward(g_pFwdPlayerState);
	g_pForwards->ReleaseForward(g_pFwdBombTargetState);

	REMOVE_DETOUR(g_pDetVoiceCommand);
	REMOVE_DETOUR(g_pDetJoinClass);
	REMOVE_DETOUR(g_pDetPopHelmet);
	REMOVE_DETOUR(g_pDetRespawn);
	REMOVE_DETOUR(g_pDetAddWaveTime);
	REMOVE_DETOUR(g_pDetSetWinningTeam);
	REMOVE_DETOUR(g_pDetRoundState);
	REMOVE_DETOUR(g_pDetPlayerState);
	REMOVE_DETOUR(g_pDetBombTargetState);
}

void CDODHooks::SDK_OnAllLoaded()
{
	SM_GET_LATE_IFACE(BINTOOLS, g_pBinTools);
	SM_GET_LATE_IFACE(SDKTOOLS, g_pSDKTools);

	if (!g_pBinTools)
	{
		META_CONPRINTF("Fatal Error: Failed to load bintools.\n");

		return;
	}

	SH_ADD_HOOK_MEMFUNC(IServerGameClients, SetCommandClient, g_pGameClients, this, &CDODHooks::OnSetCommandClient, false);

	g_pKillCmd = g_pCvar->FindCommand("kill");
	
	SH_ADD_HOOK_STATICFUNC(ConCommand, Dispatch, g_pKillCmd, OnKillCommand, false);

	g_iOffset_PlayerClass			= GetSendPropOffset("CDODPlayer", "m_iDesiredPlayerClass");
	g_iOffset_DesiredPlayerClass	= GetSendPropOffset("CDODPlayer", "m_iDesiredPlayerClass");

	g_iOffset_NumControlPoints		= GetSendPropOffset("CDODObjectiveResource", "m_iNumControlPoints");
	g_iOffset_AlliesIcons			= GetSendPropOffset("CDODObjectiveResource", "m_iAlliesIcons");
	g_iOffset_AxisIcons				= GetSendPropOffset("CDODObjectiveResource", "m_iAxisIcons");
	g_iOffset_NeutralIcons			= GetSendPropOffset("CDODObjectiveResource", "m_iNeutralIcons");
	g_iOffset_TimerCapIcons			= GetSendPropOffset("CDODObjectiveResource", "m_iTimerCapIcons");
	g_iOffset_BombedIcons			= GetSendPropOffset("CDODObjectiveResource", "m_iBombedIcons");
	g_iOffset_CPIsVisible			= GetSendPropOffset("CDODObjectiveResource", "m_bCPIsVisible");

	g_iOffset_TimerPaused			= GetSendPropOffset("CDODRoundTimer", "m_bTimerPaused");
	g_iOffset_TimeRemaining			= GetSendPropOffset("CDODRoundTimer", "m_flTimeRemaining");
	g_iOffset_TimerEndTime			= GetSendPropOffset("CDODRoundTimer", "m_flTimerEndTime");

	g_pFwdVoiceCommand		= g_pForwards->CreateForward("OnVoiceCommand",			ET_Event, 2, NULL, Param_Cell, Param_CellByRef);
	g_pFwdJoinClass			= g_pForwards->CreateForward("OnJoinClass",				ET_Event, 2, NULL, Param_Cell, Param_CellByRef);
	g_pFwdPopHelmet			= g_pForwards->CreateForward("OnPopHelmet",				ET_Event, 3, NULL, Param_Cell, Param_Array, Param_Array);
	g_pFwdRespawn			= g_pForwards->CreateForward("OnPlayerRespawn",			ET_Event, 1, NULL, Param_Cell);
	g_pFwdAddWaveTime		= g_pForwards->CreateForward("OnAddWaveTime",			ET_Event, 2, NULL, Param_Cell, Param_FloatByRef);
	g_pFwdSetWinningTeam	= g_pForwards->CreateForward("OnSetWinningTeam",		ET_Event, 1, NULL, Param_Cell);
	g_pFwdRoundState		= g_pForwards->CreateForward("OnEnterRoundState",		ET_Event, 1, NULL, Param_CellByRef);
	g_pFwdPlayerState		= g_pForwards->CreateForward("OnEnterPlayerState",		ET_Event, 2, NULL, Param_Cell, Param_CellByRef);
	g_pFwdBombTargetState	= g_pForwards->CreateForward("OnEnterBombTargetState",	ET_Event, 2, NULL, Param_Cell, Param_CellByRef);

	InitializeValveGlobals();
}

bool CDODHooks::SDK_OnMetamodLoad(SourceMM::ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pEngine, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pCvar, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetServerFactory, g_pGameEnts, IServerGameEnts, INTERFACEVERSION_SERVERGAMEENTS);
	GET_V_IFACE_ANY(GetServerFactory, g_pGameClients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
	GET_V_IFACE_ANY(GetEngineFactory, netstringtables, INetworkStringTableContainer, INTERFACENAME_NETWORKSTRINGTABLESERVER);

	g_pGlobals = ismm->GetCGlobals();

	g_pSharedChangeInfo = g_pEngine->GetSharedEdictChangeInfo();

	return true;
}
