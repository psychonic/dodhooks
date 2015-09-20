#include "vglobals.h"
#include "extension.h"

void *g_pEntList = NULL;

void **g_pGameRules = NULL;
void **g_pObjectiveResource = NULL;

void InitializeValveGlobals()
{
	g_pEntList = g_pGameHelpers->GetGlobalEntityList();

	uint8 *pAddress = NULL;

#ifdef PLATFORM_WINDOWS
	int iOffset;

	if (!g_pGameConfSDKTools->GetMemSig("CreateGameRulesObject", (void **)&pAddress) || !pAddress)
	{
		return;
	}

	if (!g_pGameConfSDKTools->GetOffset("g_pGameRules", &iOffset) || !iOffset)
	{
		return;
	}

	g_pGameRules = *reinterpret_cast<void ***>(pAddress + iOffset);

	if (!g_pGameConf->GetMemSig("CreateStandardEntities", (void **)&pAddress) || !pAddress)
	{
		return;
	}

	if (!g_pGameConf->GetOffset("g_pObjectiveResource", &iOffset) || !iOffset)
	{
		return;
	}

	g_pObjectiveResource = *reinterpret_cast<void ***>(pAddress + iOffset);

#elif defined PLATFORM_LINUX
	if (!g_pGameConfSDKTools->GetMemSig("g_pGameRules", (void **)&pAddress) || !pAddress)
	{
		return;
	}

	g_pGameRules = reinterpret_cast<void **>(pAddress);

	if (!g_pGameConf->GetMemSig("g_pObjectiveResource", (void **)&pAddress) || !pAddress)
	{
		return;
	}

	g_pObjectiveResource = reinterpret_cast<void **>(pAddress);
#endif
}

