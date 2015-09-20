#include "extension.h"
#include "natives.h"

IChangeInfoAccessor *CBaseEdict::GetChangeAccessor()
{
	return g_pEngine->GetChangeAccessor( (const edict_t *)this );
}

cell_t Native_GetPlayerClass(IPluginContext *pContext, const cell_t *Params)
{
	edict_t *pEdict = g_pEngine->PEntityOfEntIndex(Params[1]);
	CBaseEntity *pEntity = g_pGameEnts->EdictToBaseEntity(pEdict);

	if (!pEntity || strcmp(pEdict->GetClassName(), "player") != 0)
	{
		return pContext->ThrowNativeError("Client index %d is not valid", Params[1]);
	}

	return OFFSET(int, pEntity, g_iOffset_PlayerClass);
}

cell_t Native_SetPlayerClass(IPluginContext *pContext, const cell_t *Params)
{
	edict_t *pEdict = g_pEngine->PEntityOfEntIndex(Params[1]);
	CBaseEntity *pEntity = g_pGameEnts->EdictToBaseEntity(pEdict);

	if (!pEntity || strcmp(pEdict->GetClassName(), "player") != 0)
	{
		return pContext->ThrowNativeError("Client index %d is not valid", Params[1]);
	}
	
	if (Params[2] <= PlayerClass_None || Params[2] > PlayerClass_Size)
	{
		return pContext->ThrowNativeError("Player class %d is not valid", Params[2]);
	}

	OFFSET(int, pEntity, g_iOffset_PlayerClass) = Params[2];

	return true;
}

cell_t Native_GetDesiredPlayerClass(IPluginContext *pContext, const cell_t *Params)
{
	edict_t *pEdict = g_pEngine->PEntityOfEntIndex(Params[1]);
	CBaseEntity *pEntity = g_pGameEnts->EdictToBaseEntity(pEdict);

	if (!pEntity || strcmp(pEdict->GetClassName(), "player") != 0)
	{
		return pContext->ThrowNativeError("Client index %d is not valid", Params[1]);
	}

	return OFFSET(int, pEntity, g_iOffset_DesiredPlayerClass);
}

cell_t Native_SetDesiredPlayerClass(IPluginContext *pContext, const cell_t *Params)
{
	edict_t *pEdict = g_pEngine->PEntityOfEntIndex(Params[1]);
	CBaseEntity *pEntity = g_pGameEnts->EdictToBaseEntity(pEdict);

	if (!pEntity || strcmp(pEdict->GetClassName(), "player") != 0)
	{
		return pContext->ThrowNativeError("Client index %d is not valid", Params[1]);
	}
	
	if (Params[2] < PlayerClass_Random || Params[2] > PlayerClass_Size)
	{
		return pContext->ThrowNativeError("Player class %d is not valid", Params[2]);
	}

	OFFSET(int, pEntity, g_iOffset_DesiredPlayerClass) = Params[2];

	return true;
}


cell_t Native_PopHelmet(IPluginContext *pContext, const cell_t *Params)
{
	edict_t *pEdict = g_pEngine->PEntityOfEntIndex(Params[1]);
	CBaseEntity *pEntity = g_pGameEnts->EdictToBaseEntity(pEdict);

	if (!pEntity || strcmp(pEdict->GetClassName(), "player") != 0)
	{
		return pContext->ThrowNativeError("Client index %d is not valid", Params[1]);
	}

	static ICallWrapper *pWrapper = NULL;
	
	if (!pWrapper)
	{
		REGISTER_NATIVE_ADDR("PopHelmet", 
			PassInfo Pass[2]; \

			Pass[0].flags = PASSFLAG_BYVAL; \
			Pass[0].size = sizeof(Vector); \
			Pass[0].type = PassType_Basic; \

			Pass[1].flags = PASSFLAG_BYVAL; \
			Pass[1].size = sizeof(Vector); \
			Pass[1].type = PassType_Basic; \

			pWrapper = g_pBinTools->CreateCall(pAddress, CallConv_ThisCall, NULL, Pass, 2));
	}

	cell_t *pAddr1, *pAddr2;

	pContext->LocalToPhysAddr(Params[2], &pAddr1);
	pContext->LocalToPhysAddr(Params[3], &pAddr2);

	unsigned char vstk[sizeof(CBaseEntity *) + (sizeof(Vector) * 2)];
	unsigned char *vptr = vstk;

	*(CBaseEntity **)vptr = pEntity;
	vptr += sizeof(CBaseEntity *);

	*(Vector *)vptr = Vector(sp_ctof(pAddr1[0]), sp_ctof(pAddr1[1]), sp_ctof(pAddr1[2]));
	vptr += sizeof(Vector);

	*(Vector *)vptr = Vector(sp_ctof(pAddr2[0]), sp_ctof(pAddr2[1]), sp_ctof(pAddr2[2]));

	pWrapper->Execute(vstk, NULL);

	return true;
}

cell_t Native_SetNumControlPoints(IPluginContext *pContext, const cell_t *Params)
{
	if (!g_pObjectiveResource)
	{
		return pContext->ThrowNativeError("g_pObjectiveResource is NULL");
	}

	CBaseEntity *pObjectiveResource = (CBaseEntity *)*g_pObjectiveResource;

	if (!pObjectiveResource)
	{
		return pContext->ThrowNativeError("ObjectiveResource not available before map is loaded");
	}
	
	OFFSET(int, pObjectiveResource, g_iOffset_NumControlPoints) = Params[1];

	g_pGameEnts->BaseEntityToEdict(pObjectiveResource)->StateChanged(g_iOffset_NumControlPoints);

	return true;
}

void PrecacheMaterial(const char *szMaterialName)
{
	static ICallWrapper *pWrapper = NULL;

	if (!pWrapper)
	{
		void *pAddress;

		if (!g_pGameConf->GetMemSig("PrecacheMaterial", &pAddress) || !pAddress)
		{
			return;
		}

		PassInfo Pass[1];

		Pass[0].flags = PASSFLAG_BYVAL;
		Pass[0].size = sizeof(const char *);
		Pass[0].type = PassType_Basic;

		pWrapper = g_pBinTools->CreateCall(pAddress, CallConv_Cdecl, NULL, Pass, 1);
	}
	
	pWrapper->Execute(&szMaterialName, NULL);
}

int GetMaterialIndex(const char *pMaterialName)
{
	static ICallWrapper *pWrapper = NULL;

	if (!pWrapper)
	{
		void *pAddress;

		if (!g_pGameConf->GetMemSig("GetMaterialIndex", &pAddress) || !pAddress)
		{
			return 0;
		}

		PassInfo Pass[1];

		Pass[0].flags = PASSFLAG_BYVAL;
		Pass[0].size = sizeof(const char *);
		Pass[0].type = PassType_Basic;

		PassInfo Ret[1];

		Ret[0].flags = PASSFLAG_BYVAL;
		Ret[0].size = sizeof(int);
		Ret[0].type = PassType_Basic;

		pWrapper = g_pBinTools->CreateCall(pAddress, CallConv_Cdecl, Ret, Pass, 1);
	}
	
	int iIndex = -1;

	pWrapper->Execute(&pMaterialName, &iIndex);

	return iIndex;
}

cell_t Native_PrecacheCPIcon(IPluginContext *pContext, const cell_t *Params)
{
	char *szMaterialName;
	pContext->LocalToString(Params[1], &szMaterialName);

	PrecacheMaterial(szMaterialName);

	return GetMaterialIndex(szMaterialName);
}

cell_t Native_SetCPIcons(IPluginContext *pContext, const cell_t *Params)
{
	if (!g_pObjectiveResource)
	{
		return pContext->ThrowNativeError("g_pObjectiveResource is NULL");
	}
	
	CBaseEntity *pObjectiveResource = (CBaseEntity *)*g_pObjectiveResource;

	if (!pObjectiveResource)
	{
		return pContext->ThrowNativeError("ObjectiveResource not available before map is loaded");
	}

	if (Params[1] < 0 || Params[1] >= MAX_CONTROL_POINTS)
	{
		return pContext->ThrowNativeError("Control point index %d is not valid", Params[1]);
	}

	edict_t *pEdict = g_pGameEnts->BaseEntityToEdict(pObjectiveResource);

	uint8 arrayElement = Params[1] * sizeof(int);

	if (Params[2])
	{
		OFFSET(int, pObjectiveResource, g_iOffset_AlliesIcons + arrayElement) = Params[2];

		pEdict->StateChanged(g_iOffset_AlliesIcons + arrayElement);
	}

	if (Params[3])
	{
		OFFSET(int, pObjectiveResource, g_iOffset_AxisIcons + arrayElement) = Params[3];

		pEdict->StateChanged(g_iOffset_AxisIcons + arrayElement);
	}

	if (Params[4])
	{
		OFFSET(int, pObjectiveResource, g_iOffset_NeutralIcons + arrayElement) = Params[4]; 

		pEdict->StateChanged(g_iOffset_NeutralIcons + arrayElement);
	}

	if (Params[5])
	{
		OFFSET(int, pObjectiveResource, g_iOffset_TimerCapIcons + arrayElement) = Params[5]; 

		pEdict->StateChanged(g_iOffset_TimerCapIcons + arrayElement);
	}

	if (Params[6])
	{
		OFFSET(int, pObjectiveResource, g_iOffset_BombedIcons + arrayElement) = Params[6]; 

		pEdict->StateChanged(g_iOffset_BombedIcons + arrayElement);
	}

	return true;
}

cell_t Native_SetCPVisible(IPluginContext *pContext, const cell_t *Params)
{
	if (!g_pObjectiveResource)
	{
		return pContext->ThrowNativeError("g_pObjectiveResource is NULL");
	}

	CBaseEntity *pObjectiveResource = (CBaseEntity *)*g_pObjectiveResource;

	if (!pObjectiveResource)
	{
		return pContext->ThrowNativeError("ObjectiveResource not available before map is loaded");
	}

	if (Params[1] < 0 || Params[1] >= MAX_CONTROL_POINTS)
	{
		return pContext->ThrowNativeError("Control point index %d is not valid", Params[1]);
	}

	uint8 arrayElement = Params[1] * sizeof(int);

	OFFSET(int, pObjectiveResource, g_iOffset_CPIsVisible + arrayElement) = Params[2];

	g_pGameEnts->BaseEntityToEdict(pObjectiveResource)->StateChanged(g_iOffset_CPIsVisible + arrayElement);

	return true;
}

cell_t Native_PauseTimer(IPluginContext *pContext, const cell_t *Params)
{
	edict_t *pEdict = g_pEngine->PEntityOfEntIndex(Params[1]);
	CBaseEntity *pEntity = g_pGameEnts->EdictToBaseEntity(pEdict);

	if (!pEntity || strcmp(pEdict->GetClassName(), "dod_round_timer") != 0)
	{
		return pContext->ThrowNativeError("Entity index %d is not valid", Params[1]);
	}

	if (!OFFSET(bool, pEntity, g_iOffset_TimerPaused))
	{
		OFFSET(float, pEntity, g_iOffset_TimeRemaining) = OFFSET(float, pEntity, g_iOffset_TimerEndTime) - g_pGlobals->curtime;
		OFFSET(bool, pEntity, g_iOffset_TimerPaused) = true;

		pEdict->StateChanged(g_iOffset_TimeRemaining);
		pEdict->StateChanged(g_iOffset_TimerPaused);
	}

	return true;
}

cell_t Native_ResumeTimer(IPluginContext *pContext, const cell_t *Params)
{
	edict_t *pEdict = g_pEngine->PEntityOfEntIndex(Params[1]);
	CBaseEntity *pEntity = g_pGameEnts->EdictToBaseEntity(pEdict);

	if (!pEntity || strcmp(pEdict->GetClassName(), "dod_round_timer") != 0)
	{
		return pContext->ThrowNativeError("Entity index %d is not valid", Params[1]);
	}

	if (OFFSET(bool, pEntity, g_iOffset_TimerPaused))
	{
		OFFSET(float, pEntity, g_iOffset_TimerEndTime) = OFFSET(float, pEntity, g_iOffset_TimeRemaining) + g_pGlobals->curtime;
		OFFSET(bool, pEntity, g_iOffset_TimerPaused) = false;

		pEdict->StateChanged(g_iOffset_TimerEndTime);
		pEdict->StateChanged(g_iOffset_TimerPaused);
	}

	return true;
}

cell_t Native_SetTimeRemaining(IPluginContext *pContext, const cell_t *Params)
{
	edict_t *pEdict = g_pEngine->PEntityOfEntIndex(Params[1]);
	CBaseEntity *pEntity = g_pGameEnts->EdictToBaseEntity(pEdict);

	if (!pEntity || strcmp(pEdict->GetClassName(), "dod_round_timer") != 0)
	{
		return pContext->ThrowNativeError("Entity index %d is not valid", Params[1]);
	}

	OFFSET(float, pEntity, g_iOffset_TimeRemaining) = Params[2];
	OFFSET(float, pEntity, g_iOffset_TimerEndTime) = g_pGlobals->curtime + Params[2];

	pEdict->StateChanged(g_iOffset_TimeRemaining);
	pEdict->StateChanged(g_iOffset_TimerEndTime);

	return true;
}

cell_t Native_GetTimeRemaining(IPluginContext *pContext, const cell_t *Params)
{
	edict_t *pEdict = g_pEngine->PEntityOfEntIndex(Params[1]);
	CBaseEntity *pEntity = g_pGameEnts->EdictToBaseEntity(pEdict);

	if (!pEntity || strcmp(pEdict->GetClassName(), "dod_round_timer") != 0)
	{
		return pContext->ThrowNativeError("Entity index %d is not valid", Params[1]);
	}

	float fTimeRemaining;
	
	if (OFFSET(bool, pEntity, g_iOffset_TimerPaused))
	{
		fTimeRemaining = OFFSET(float, pEntity, g_iOffset_TimeRemaining);
	}
	else
	{
		fTimeRemaining = OFFSET(float, pEntity, g_iOffset_TimerEndTime) - g_pGlobals->curtime;
	}

	if (fTimeRemaining < 0.0)
	{
		fTimeRemaining = 0.0;
	}

	return sp_ftoc(fTimeRemaining);
}

cell_t Native_RespawnPlayer(IPluginContext *pContext, const cell_t *Params)
{
	edict_t *pEdict = g_pEngine->PEntityOfEntIndex(Params[1]);
	CBaseEntity *pEntity = g_pGameEnts->EdictToBaseEntity(pEdict);

	if (!pEntity || strcmp(pEdict->GetClassName(), "player") != 0)
	{
		return pContext->ThrowNativeError("Client index %d is not valid", Params[1]);
	}
	
	if (Params[2])
	{
		int iPlayerClass = OFFSET(int, pEntity, g_iOffset_DesiredPlayerClass);

		if (iPlayerClass == PlayerClass_None)
		{
			return pContext->ThrowNativeError("Player class %d is not valid", iPlayerClass);
		}
	}
	
	static ICallWrapper *pWrapper = NULL;

	if (!pWrapper)
	{
		REGISTER_NATIVE_ADDR("DODRespawn",
			pWrapper = g_pBinTools->CreateCall(pAddress, CallConv_ThisCall, NULL, NULL, 0));
	}

	pWrapper->Execute(&pEntity, NULL);

	return true;
}

cell_t Native_AddWaveTime(IPluginContext *pContext, const cell_t *Params)
{
	if (Params[2] < Team_Spectators || Params[2] > Team_Axis)
	{
		return pContext->ThrowNativeError("Team index %i is not valid", Params[2]);
	}
	
	static ICallWrapper *pWrapper = NULL;

	if (!pWrapper)
	{
		REGISTER_NATIVE_ADDR("AddWaveTime", 
			PassInfo Pass[2]; \

			Pass[0].flags = PASSFLAG_BYVAL; \
			Pass[0].size = sizeof(int); \
			Pass[0].type = PassType_Basic; \

			Pass[1].flags = PASSFLAG_BYVAL; \
			Pass[1].size = sizeof(float); \
			Pass[1].type = PassType_Basic; \

			pWrapper = g_pBinTools->CreateCall(pAddress, CallConv_ThisCall, NULL, Pass, 2));
	}
	
	if (!g_pGameRules)
	{
		return pContext->ThrowNativeError("g_pGameRules is NULL");
	}

	void *pGameRules = *g_pGameRules;

	if (!pGameRules)
	{
		return pContext->ThrowNativeError("GameRules not available before map is loaded");
	}

	unsigned char vstk[sizeof(void *) + sizeof(int) + sizeof(float)];
	unsigned char *vptr = vstk;

	*(void **)vptr = pGameRules;
	vptr += sizeof(void *);

	*(int *)vptr = Params[1];
	vptr += sizeof(int);

	*(float *)vptr = *(float *)&Params[2];

	pWrapper->Execute(vstk, NULL);

	return true;
}

cell_t Native_SetWinningTeam(IPluginContext *pContext, const cell_t *Params)
{
	if (Params[1] != Team_Allies && Params[1] != Team_Axis)
	{
		return pContext->ThrowNativeError("Team index %d is not valid", Params[1]);
	}

	static ICallWrapper *pWrapper = NULL;

	if (!pWrapper)
	{
		REGISTER_NATIVE_ADDR("SetWinningTeam",
			PassInfo Pass[1]; \

			Pass[0].flags = PASSFLAG_BYVAL; \
			Pass[0].size = sizeof(int); \
			Pass[0].type = PassType_Basic; \

			pWrapper = g_pBinTools->CreateCall(pAddress, CallConv_ThisCall, NULL, Pass, 1));
	}

	if (!g_pGameRules)
	{
		return pContext->ThrowNativeError("g_pGameRules is NULL");
	}

	void *pGameRules = *g_pGameRules;

	if (!pGameRules)
	{
		return pContext->ThrowNativeError("GameRules not available before map is loaded");
	}

	unsigned char vstk[sizeof(void *) + sizeof(int)];
	unsigned char *vptr = vstk;

	*(void **)vptr = pGameRules;
	vptr += sizeof(void *);

	*(int *)vptr = Params[1];

	pWrapper->Execute(vstk, NULL);

	return true;
}

cell_t Native_SetRoundState(IPluginContext *pContext, const cell_t *Params)
{
	static ICallWrapper *pWrapper = NULL;

	if (!pWrapper)
	{
		REGISTER_NATIVE_ADDR("RoundState", 
			PassInfo Pass[1]; \

			Pass[0].flags = PASSFLAG_BYVAL; \
			Pass[0].size = sizeof(int); \
			Pass[0].type = PassType_Basic; \

			pWrapper = g_pBinTools->CreateCall(pAddress, CallConv_ThisCall, NULL, Pass, 1));
	}

	if (!g_pGameRules)
	{
		return pContext->ThrowNativeError("g_pGameRules is NULL");
	}

	void *pGameRules = *g_pGameRules;

	if (!pGameRules)
	{
		return pContext->ThrowNativeError("GameRules not available before map is loaded");
	}

	unsigned char vstk[sizeof(void *) + sizeof(int)];
	unsigned char *vptr = vstk;

	*(void **)vptr = pGameRules;
	vptr += sizeof(void *);

	*(int *)vptr = Params[1];

	pWrapper->Execute(vstk, NULL);

	return true;
}

cell_t Native_SetPlayerState(IPluginContext *pContext, const cell_t *Params)
{
	edict_t *pEdict = g_pEngine->PEntityOfEntIndex(Params[1]);
	CBaseEntity *pEntity = g_pGameEnts->EdictToBaseEntity(pEdict);

	if (!pEntity || strcmp(pEdict->GetClassName(), "player") != 0)
	{
		return pContext->ThrowNativeError("Client index %d is not valid", Params[1]);
	}

	static ICallWrapper *pWrapper = NULL;

	if (!pWrapper)
	{
		REGISTER_NATIVE_ADDR("PlayerState", 
			PassInfo Pass[1]; \

			Pass[0].flags = PASSFLAG_BYVAL; \
			Pass[0].size = sizeof(int); \
			Pass[0].type = PassType_Basic; \

			pWrapper = g_pBinTools->CreateCall(pAddress, CallConv_ThisCall, NULL, Pass, 1));
	}

	unsigned char vstk[sizeof(CBaseEntity *) + sizeof(int)];
	unsigned char *vptr = vstk;

	*(CBaseEntity **)vptr = pEntity;
	vptr += sizeof(CBaseEntity *);

	*(int *)vptr = Params[2];

	pWrapper->Execute(vstk, NULL);

	return true;
}

cell_t Native_SetBombTargetState(IPluginContext *pContext, const cell_t *Params)
{
	edict_t *pEdict = g_pEngine->PEntityOfEntIndex(Params[1]);
	CBaseEntity *pEntity = g_pGameEnts->EdictToBaseEntity(pEdict);

	if (!pEntity || strcmp(pEdict->GetClassName(), "dod_bomb_target") != 0)
	{
		return pContext->ThrowNativeError("Entity index %d is not valid", Params[1]);
	}

	static ICallWrapper *pWrapper = NULL;

	if (!pWrapper)
	{
		REGISTER_NATIVE_ADDR("BombTargetState", 
			PassInfo Pass[1]; \

			Pass[0].flags = PASSFLAG_BYVAL; \
			Pass[0].size = sizeof(int); \
			Pass[0].type = PassType_Basic; \

			pWrapper = g_pBinTools->CreateCall(pAddress, CallConv_ThisCall, NULL, Pass, 1));
	}

	unsigned char vstk[sizeof(CBaseEntity *) + sizeof(int)];
	unsigned char *vptr = vstk;

	*(CBaseEntity **)vptr = pEntity;
	vptr += sizeof(CBaseEntity *);

	*(int *)vptr = Params[2];

	pWrapper->Execute(vstk, NULL);

	return true;
}
