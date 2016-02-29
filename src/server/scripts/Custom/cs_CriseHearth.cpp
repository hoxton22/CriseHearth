/*
* Copyright © 2013-2014 Le Mouchard
*/
#ifndef SC_PRECOMPILED_H
#define SC_PRECOMPILED_H

#include "ScriptMgr.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GameEventMgr.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Unit.h"
#include "GameObject.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "InstanceScript.h"
#include "CombatAI.h"
#include "PassiveAI.h"
#include "Chat.h"
#include "DBCStructure.h"
#include "DBCStores.h"
#include "ObjectMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"

#ifdef _WIN32
#include <windows.h>
#endif

#endif


class AlertMJ_connexion : public PlayerScript
{
public:
	AlertMJ_connexion() : PlayerScript("AlertMJ_connexion"){}

	void OnLogin(Player* player, bool firstLogin = false)
	{
		player->Talk("Test test", CHAT_MSG_SYSTEM, LANG_UNIVERSAL, sWorld->getFloatConfig(CONFIG_LISTEN_RANGE_TEXTEMOTE), nullptr);
	}
};

void AddSC_cs_CriseHearth()
{
	new AlertMJ_connexion();
}