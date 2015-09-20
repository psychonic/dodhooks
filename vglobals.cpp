#include "vglobals.h"
#include "extension.h"

void *g_pEntList = NULL;

void **g_pObjectiveResource = NULL;

void InitializeValveGlobals()
{
	g_pEntList = g_pGameHelpers->GetGlobalEntityList();

	uint8 *pAddress = NULL;

#ifdef PLATFORM_WINDOWS
	int iOffset;

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
	if (!g_pGameConf->GetMemSig("g_pObjectiveResource", (void **)&pAddress) || !pAddress)
	{
		return;
	}

	g_pObjectiveResource = reinterpret_cast<void **>(pAddress);
#endif
}

