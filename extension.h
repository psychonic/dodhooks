#ifndef _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
#define _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_

#include <IBinTools.h>
#include <ISDKTools.h>

#include <smsdk_ext.h>
#include <CDetour/detours.h>

#define g_pGameHelpers gamehelpers
#define g_pEngine engine
#define g_pGameConfs gameconfs

extern IGameConfig *g_pGameConf;

extern CGlobalVars *g_pGlobals;

extern IBinTools *g_pBinTools;
extern ISDKTools *g_pSDKTools;

extern IServerGameEnts *g_pGameEnts;

extern CSharedEdictChangeInfo *g_pSharedChangeInfo;

extern uint32 g_iOffset_PlayerClass;
extern uint32 g_iOffset_DesiredPlayerClass;

extern uint32 g_iOffset_NumControlPoints;
extern uint32 g_iOffset_AlliesIcons;
extern uint32 g_iOffset_AxisIcons;
extern uint32 g_iOffset_NeutralIcons;
extern uint32 g_iOffset_TimerCapIcons;
extern uint32 g_iOffset_BombedIcons;
extern uint32 g_iOffset_CPIsVisible;

extern uint32 g_iOffset_TimerPaused;
extern uint32 g_iOffset_TimeRemaining;
extern uint32 g_iOffset_TimerEndTime;

extern void *g_pEntList;

extern void **g_pObjectiveResource;

#define DOD_MAXPLAYERS 33

#define CREATE_DETOUR(detour, name, gamedata) \
	detour = DETOUR_CREATE_MEMBER(name, gamedata); \
	if (detour != NULL) \
	{ \
		detour->EnableDetour(); \
	} \
	else \
	{ \
		if (szConfigError[0]) \
		{ \
			snprintf(error, maxlength, "Fatal Error: Unable to load detour - %s", gamedata); \
		} \
		return false; \
	}

#define REMOVE_DETOUR(detour) \
	if (detour != NULL) \
	{ \
		detour->Destroy(); \
	}

enum
{
	PlayerClass_Random = -2,
	PlayerClass_None,
	PlayerClass_Rifleman,
	PlayerClass_Assault,
	PlayerClass_Support,
	PlayerClass_Sniper,
	PlayerClass_Machinegunner,
	PlayerClass_Rocket,

	PlayerClass_Size
};

inline uint32 GetSendPropOffset(const char *szNetClass, const char *szPropName)
{
	sm_sendprop_info_t SendPropInfo;

	if (!g_pGameHelpers->FindSendPropInfo(szNetClass, szPropName, &SendPropInfo))
	{
		META_CONPRINTF("Fatal Error: Unable to get offset: %s::%s!\n", szNetClass, szPropName);

		return -1;
	}

	return SendPropInfo.actual_offset;
}

/**
 * @brief Sample implementation of the SDK Extension.
 * Note: Uncomment one of the pre-defined virtual functions in order to use it.
 */
class CDODHooks : public SDKExtension
{
public:
	void OnSetCommandClient(int client);
public:

	/**
	 * @brief This is called after the initial loading sequence has been processed.
	 *
	 * @param error		Error message buffer.
	 * @param maxlength	Size of error message buffer.
	 * @param late		Whether or not the module was loaded after map load.
	 * @return			True to succeed loading, false to fail.
	 */
	virtual bool SDK_OnLoad(char *error, size_t maxlength, bool late);
	
	/**
	 * @brief This is called right before the extension is unloaded.
	 */
	virtual void SDK_OnUnload();

	/**
	 * @brief This is called once all known extensions have been loaded.
	 * Note: It is is a good idea to add natives here, if any are provided.
	 */
	virtual void SDK_OnAllLoaded();

	/**
	 * @brief Called when the pause state is changed.
	 */
	//virtual void SDK_OnPauseChange(bool paused);

	/**
	 * @brief this is called when Core wants to know if your extension is working.
	 *
	 * @param error		Error message buffer.
	 * @param maxlength	Size of error message buffer.
	 * @return			True if working, false otherwise.
	 */
	//virtual bool QueryRunning(char *error, size_t maxlength);

	//void NotifyInterfaceDrop(SMInterface *pInterface);
	//bool QueryInterfaceDrop(SMInterface *pInterface);
public:
#if defined SMEXT_CONF_METAMOD
	/**
	 * @brief Called when Metamod is attached, before the extension version is called.
	 *
	 * @param error			Error buffer.
	 * @param maxlength		Maximum size of error buffer.
	 * @param late			Whether or not Metamod considers this a late load.
	 * @return				True to succeed, false to fail.
	 */
	virtual bool SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlength, bool late);

	/**
	 * @brief Called when Metamod is detaching, after the extension version is called.
	 * NOTE: By default this is blocked unless sent from SourceMod.
	 *
	 * @param error			Error buffer.
	 * @param maxlength		Maximum size of error buffer.
	 * @return				True to succeed, false to fail.
	 */
	//virtual bool SDK_OnMetamodUnload(char *error, size_t maxlength);

	/**
	 * @brief Called when Metamod's pause state is changing.
	 * NOTE: By default this is blocked unless sent from SourceMod.
	 *
	 * @param paused		Pause state being set.
	 * @param error			Error buffer.
	 * @param maxlength		Maximum size of error buffer.
	 * @return				True to succeed, false to fail.
	 */
	//virtual bool SDK_OnMetamodPauseChange(bool paused, char *error, size_t maxlength);
#endif
};

#endif // _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
