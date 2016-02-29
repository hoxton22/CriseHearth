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
#include "AccountMgr.h"
#include "ArenaTeamMgr.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "Group.h"
#include "InstanceSaveMgr.h"
#include "Language.h"
#include "MovementGenerator.h"
#include "ObjectAccessor.h"
#include "Opcodes.h"
#include "SpellAuras.h"
#include "TargetedMovementGenerator.h"
#include "WeatherMgr.h"
#include "Player.h"
#include "Pet.h"
#include "LFG.h"
#include "GroupMgr.h"
#include "MMapFactory.h"
#include "DisableMgr.h"
#include "SpellHistory.h"
#include "MiscPackets.h"
#include "Transport.h"

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
		if (player->GetSession()->GetSecurity() < 2 )
		{
			return;
		}
		PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_REQUEST_LIST);
		PreparedQueryResult reqResult = WorldDatabase.Query(stmt);
		if (!reqResult)
		{
			ChatHandler(player->GetSession()).SendSysMessage("Toutes les requetes sont fermees, hela bonne journee");
			return;
		}
		uint32 i = 0;
		do
		{
			i++;
		} while (reqResult->NextRow());
		//
		ChatHandler(player->GetSession()).PSendSysMessage("Il y a %u requetes, au boulot feignasse", i);
	}
};

void AddSC_cs_CriseHearth()
{
	new AlertMJ_connexion();
}