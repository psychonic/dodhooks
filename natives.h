#ifndef _INCLUDE_SOURCEMOD_NATIVES_PROPER_H_
#define _INCLUDE_SOURCEMOD_NATIVES_PROPER_H_

cell_t Native_GetPlayerClass			(IPluginContext *pContext, const cell_t *Params);
cell_t Native_SetPlayerClass			(IPluginContext *pContext, const cell_t *Params);
cell_t Native_GetDesiredPlayerClass		(IPluginContext *pContext, const cell_t *Params);
cell_t Native_SetDesiredPlayerClass		(IPluginContext *pContext, const cell_t *Params);
cell_t Native_PopHelmet					(IPluginContext *pContext, const cell_t *Params);
cell_t Native_SetNumControlPoints		(IPluginContext *pContext, const cell_t *Params);
cell_t Native_PrecacheCPIcon			(IPluginContext *pContext, const cell_t *Params);
cell_t Native_SetCPIcons				(IPluginContext *pContext, const cell_t *Params);
cell_t Native_SetCPVisible				(IPluginContext *pContext, const cell_t *Params);
cell_t Native_PauseTimer				(IPluginContext *pContext, const cell_t *Params);
cell_t Native_ResumeTimer				(IPluginContext *pContext, const cell_t *Params);
cell_t Native_SetTimeRemaining			(IPluginContext *pContext, const cell_t *Params);
cell_t Native_GetTimeRemaining			(IPluginContext *pContext, const cell_t *Params);
cell_t Native_RespawnPlayer				(IPluginContext *pContext, const cell_t *Params);
cell_t Native_AddWaveTime				(IPluginContext *pContext, const cell_t *Params);
cell_t Native_SetWinningTeam			(IPluginContext *pContext, const cell_t *Params);
cell_t Native_SetRoundState				(IPluginContext *pContext, const cell_t *Params);
cell_t Native_SetPlayerState			(IPluginContext *pContext, const cell_t *Params);
cell_t Native_SetBombTargetState		(IPluginContext *pContext, const cell_t *Params);

enum
{
	Team_Spectators = 1,
	Team_Allies,
	Team_Axis
};

#define MAX_CONTROL_POINTS 8

#define REGISTER_NATIVE_ADDR(name, code) \
	void *pAddress; \
	if (!g_pGameConf->GetMemSig(name, &pAddress) || !pAddress) \
	{ \
		return pContext->ThrowNativeError("Failed to locate function " #name); \
	} \
	code;

#define OFFSET(type, addr, offset) \
	*(type*)((uint32)addr + offset)

const sp_nativeinfo_t g_Natives[] = 
{
	{"GetPlayerClass",				Native_GetPlayerClass},
	{"SetPlayerClass",				Native_SetPlayerClass},
	{"GetDesiredPlayerClass",		Native_GetDesiredPlayerClass},
	{"SetDesiredPlayerClass",		Native_SetDesiredPlayerClass},
	{"PopHelmet",					Native_PopHelmet},
	{"SetNumControlPoints",			Native_SetNumControlPoints},
	{"PrecacheCPIcon",				Native_PrecacheCPIcon},
	{"SetCPIcons",					Native_SetCPIcons},
	{"SetCPVisible",				Native_SetCPVisible},
    {"PauseTimer",					Native_PauseTimer},
    {"ResumeTimer",					Native_ResumeTimer},
    {"SetTimeRemaining",			Native_SetTimeRemaining},
    {"GetTimeRemaining",			Native_GetTimeRemaining},
	{"RespawnPlayer",				Native_RespawnPlayer},
	{"AddWaveTime",					Native_AddWaveTime},
	{"SetWinningTeam",				Native_SetWinningTeam},
	{"SetRoundState",				Native_SetRoundState},
	{"SetPlayerState",				Native_SetPlayerState},
	{"SetBombTargetState",			Native_SetRoundState},
	{NULL,							NULL},
};

#endif // _INCLUDE_SOURCEMOD_NATIVES_PROPER_H_
