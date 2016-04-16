/*
 * Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/* ScriptData
Name: gobject_commandscript
%Complete: 100
Comment: All gobject related commands
Category: commandscripts
EndScriptData */

#include "ScriptMgr.h"
#include "GameEventMgr.h"
#include "ObjectMgr.h"
#include "PoolMgr.h"
#include "MapManager.h"
#include "Chat.h"
#include "Language.h"
#include "Player.h"
#include "Opcodes.h"

class gobject_commandscript : public CommandScript
{
public:
    gobject_commandscript() : CommandScript("gobject_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> gobjectAddCommandTable =
        {
            { "temp", rbac::RBAC_PERM_COMMAND_GOBJECT_ADD_TEMP, false, &HandleGameObjectAddTempCommand,   "" },
            { "",     rbac::RBAC_PERM_COMMAND_GOBJECT_ADD,      false, &HandleGameObjectAddCommand,       "" },
        };
        static std::vector<ChatCommand> gobjectSetCommandTable =
        {
            { "phase", rbac::RBAC_PERM_COMMAND_GOBJECT_SET_PHASE, false, &HandleGameObjectSetPhaseCommand,  "" },
            { "state", rbac::RBAC_PERM_COMMAND_GOBJECT_SET_STATE, false, &HandleGameObjectSetStateCommand,  "" },
        };
        static std::vector<ChatCommand> gobjectCommandTable =
        {
            { "activate", rbac::RBAC_PERM_COMMAND_GOBJECT_ACTIVATE, false, &HandleGameObjectActivateCommand,  ""       },
            { "delete",   rbac::RBAC_PERM_COMMAND_GOBJECT_DELETE,   false, &HandleGameObjectDeleteCommand,    ""       },
            { "info",     rbac::RBAC_PERM_COMMAND_GOBJECT_INFO,     false, &HandleGameObjectInfoCommand,      ""       },
            { "move",     rbac::RBAC_PERM_COMMAND_GOBJECT_MOVE,     false, &HandleGameObjectMoveCommand,      ""       },
            { "near",     rbac::RBAC_PERM_COMMAND_GOBJECT_NEAR,     false, &HandleGameObjectNearCommand,      ""       },
			{ "scale",    rbac::RBAC_PERM_COMMAND_GOBJECT_MOVE,     false, &HandleGameObjectScaleCommand,     ""       },
            { "target",   rbac::RBAC_PERM_COMMAND_GOBJECT_TARGET,   false, &HandleGameObjectTargetCommand,    ""       },
            { "turn",     rbac::RBAC_PERM_COMMAND_GOBJECT_TURN,     false, &HandleGameObjectTurnCommand,      ""       },
            { "add",      rbac::RBAC_PERM_COMMAND_GOBJECT_ADD,      false, NULL,            "", gobjectAddCommandTable },
            { "set",      rbac::RBAC_PERM_COMMAND_GOBJECT_SET,      false, NULL,            "", gobjectSetCommandTable },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "gobject", rbac::RBAC_PERM_COMMAND_GOBJECT, false, NULL, "", gobjectCommandTable },
        };
        return commandTable;
    }
	
	static bool HandleGameObjectScaleCommand(ChatHandler* handler, char const* args)
	{
		if (!*args)
			return false;

		char* id = handler->extractKeyFromLink((char*)args, "Hgameobject");
		if (!id)
			return false;

		uint32 guidLow = atoi(id);
		if (!guidLow)
			return false;

		GameObject* object = NULL;

		// via db
		if (GameObjectData const* goData = sObjectMgr->GetGOData(guidLow))
			object = handler->GetObjectGlobalyWithGuidOrNearWithDbGuid(guidLow, goData->id);

		if (!object)
		{
			handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, guidLow);
			handler->SetSentErrorMessage(true);
			return false;
		}

		char* scale_temp = strtok(NULL, " ");
		float scale = scale_temp ? atof(scale_temp) : 1.0f;
		if (scale < 0.0f || scale > 100.0f)
		{
			handler->SendSysMessage(LANG_BAD_VALUE);
			handler->SetSentErrorMessage(true);
			return false;
		}

		// exécution du scale
		object->SetObjectScale(scale);
		object->DestroyForNearbyPlayers();
		object->UpdateObjectVisibility();
		std::string playerName = handler->GetSession()->GetPlayer()->GetName();
		PreparedStatement* stmtSel = WorldDatabase.GetPreparedStatement(WORLD_SEL_CHARACTERCUSTOM_ALL);
		stmtSel->setString(0, playerName.c_str());
		PreparedQueryResult reqResult = WorldDatabase.Query(stmtSel);
		if (!reqResult)
		{
			object->SaveToDB();
		}
		else
		{
			Field* field = reqResult->Fetch();
			uint32 phase = field[3].GetUInt32();
			object->SaveToDBWithPhase(phase);
		}


		Player* _caller = handler->GetSession()->GetPlayer();
		Map::PlayerList const& PlayerList = _caller->GetMap()->GetPlayers();
		for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
			if (Player* _player = itr->GetSource())
				_player->TeleportTo(_player->GetMapId(), _player->GetPositionX(), _player->GetPositionY(), _player->GetPositionZ(), _player->GetOrientation());

		return true;

	}

    static bool HandleGameObjectActivateCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* id = handler->extractKeyFromLink((char*)args, "Hgameobject");
        if (!id)
            return false;

        ObjectGuid::LowType guidLow = strtoull(id, nullptr, 10);
        if (!guidLow)
            return false;

        GameObject* object = NULL;

        // by DB guid
        if (GameObjectData const* goData = sObjectMgr->GetGOData(guidLow))
            object = handler->GetObjectGlobalyWithGuidOrNearWithDbGuid(guidLow, goData->id);

        if (!object)
        {
            handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, guidLow);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // Activate
        object->SetLootState(GO_READY);
		//CUSTOM GOB_DOOR ( if not go delete all except usedoor et PSendSys )
		GameObjectTemplate const* goInfo = object->GetGOInfo();
		if (goInfo->entry > 2000000)
		{
			Player* player = handler->GetSession()->GetPlayer();
			if (player->HasItemCount(500102, 1, false))
			{
				object->UseDoorOrButton(10000, false, handler->GetSession()->GetPlayer());
				return true;
			}
			PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_GAMEOBJECT_DOOR);
			stmt->setUInt64(0, goInfo->entry);
			PreparedQueryResult result = WorldDatabase.Query(stmt);
			if (result)
			{
				Field* field = result->Fetch();
				uint64 id_item = field[0].GetUInt64();
				if (player->HasItemCount(id_item, 1, false))
				{
					object->UseDoorOrButton(10000, false, handler->GetSession()->GetPlayer());
				}
				else
				{
					player->GetSession()->SendNotification("Vous n avez pas l objet pour activer le mecanisme.");
				}
				return true;
			}
			else
			{
				object->UseDoorOrButton(10000, false, handler->GetSession()->GetPlayer());
				return true;
			}
		}
		else
		{
			object->UseDoorOrButton(10000, false, handler->GetSession()->GetPlayer());
			return true;
		}
		object->UseDoorOrButton(10000, false, handler->GetSession()->GetPlayer());
		//
        //object->UseDoorOrButton(10000, false, handler->GetSession()->GetPlayer());

        handler->PSendSysMessage("Object activated!");

        return true;
    }

    //spawn go
    static bool HandleGameObjectAddCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        // number or [name] Shift-click form |color|Hgameobject_entry:go_id|h[name]|h|r
        char* id = handler->extractKeyFromLink((char*)args, "Hgameobject_entry");
        if (!id)
            return false;

        uint32 objectId = atoul(id);
        if (!objectId)
            return false;
		
		TC_LOG_DEBUG("chat.log.whisper", "%s a .gob add %d", handler->GetSession()->GetPlayer()->GetName().c_str(), objectId);

        char* spawntimeSecs = strtok(NULL, " ");

        const GameObjectTemplate* objectInfo = sObjectMgr->GetGameObjectTemplate(objectId);

        if (!objectInfo)
        {
            handler->PSendSysMessage(LANG_GAMEOBJECT_NOT_EXIST, objectId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (objectInfo->displayId && !sGameObjectDisplayInfoStore.LookupEntry(objectInfo->displayId))
        {
            // report to DB errors log as in loading case
            TC_LOG_ERROR("sql.sql", "Gameobject (Entry %u GoType: %u) have invalid displayId (%u), not spawned.", objectId, objectInfo->type, objectInfo->displayId);
            handler->PSendSysMessage(LANG_GAMEOBJECT_HAVE_INVALID_DATA, objectId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();
        float x = float(player->GetPositionX());
        float y = float(player->GetPositionY());
        float z = float(player->GetPositionZ());
        float o = float(player->GetOrientation());
        Map* map = player->GetMap();

        GameObject* object = new GameObject;
        ObjectGuid::LowType guidLow = map->GenerateLowGuid<HighGuid::GameObject>();

        if (!object->Create(guidLow, objectInfo->entry, map, 0, x, y, z, o, 0.0f, 0.0f, 0.0f, 0.0f, 0, GO_STATE_READY))
        {
            delete object;
            return false;
        }

        object->CopyPhaseFrom(player, true);

        if (spawntimeSecs)
        {
            uint32 value = atoi((char*)spawntimeSecs);
            object->SetRespawnTime(value);
        }

        // fill the gameobject data and save to the db
        object->SaveToDB(map->GetId(), (1 << map->GetSpawnMode()), player->GetPhaseMask());
        guidLow = object->GetSpawnId();


        // delete the old object and do a clean load from DB with a fresh new GameObject instance.
        // this is required to avoid weird behavior and memory leaks
        delete object;

        object = new GameObject();
        // this will generate a new guid if the object is in an instance
        if (!object->LoadGameObjectFromDB(guidLow, map))
        {
            delete object;
            return false;
        }

        /// @todo is it really necessary to add both the real and DB table guid here ?
        sObjectMgr->AddGameobjectToGrid(guidLow, ASSERT_NOTNULL(sObjectMgr->GetGOData(guidLow)));

        handler->PSendSysMessage(LANG_GAMEOBJECT_ADD, objectId, objectInfo->name.c_str(), guidLow, x, y, z);
		// Save phasing
		uint32 phaseId = 0;
		std::string playerName = player->GetName();
		PreparedStatement* stmtSel = WorldDatabase.GetPreparedStatement(WORLD_SEL_CHARACTERCUSTOM_ALL);
		stmtSel->setString(0, playerName.c_str());
		PreparedQueryResult reqResult = WorldDatabase.Query(stmtSel);
		if (reqResult)
		{
			Field* field = reqResult->Fetch();
			phaseId = field[3].GetUInt32();
		}
		object->SaveToDBWithPhase(phaseId);
		object->SetInPhase(phaseId, true, true);
        return true;
    }

    // add go, temp only
    static bool HandleGameObjectAddTempCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        char* id = strtok((char*)args, " ");
        if (!id)
            return false;

        Player* player = handler->GetSession()->GetPlayer();

        char* spawntime = strtok(NULL, " ");
        uint32 spawntm = 300;

        if (spawntime)
            spawntm = atoi((char*)spawntime);

        float x = player->GetPositionX();
        float y = player->GetPositionY();
        float z = player->GetPositionZ();
        float ang = player->GetOrientation();

        float rot2 = std::sin(ang/2);
        float rot3 = std::cos(ang/2);

        uint32 objectId = atoi(id);

        if (!sObjectMgr->GetGameObjectTemplate(objectId))
        {
            handler->PSendSysMessage(LANG_GAMEOBJECT_NOT_EXIST, objectId);
            handler->SetSentErrorMessage(true);
            return false;
        }

        player->SummonGameObject(objectId, x, y, z, ang, 0, 0, rot2, rot3, spawntm);

        return true;
    }

    static bool HandleGameObjectTargetCommand(ChatHandler* handler, char const* args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        QueryResult result;
        GameEventMgr::ActiveEvents const& activeEventsList = sGameEventMgr->GetActiveEventList();

        if (*args)
        {
            // number or [name] Shift-click form |color|Hgameobject_entry:go_id|h[name]|h|r
            char* id = handler->extractKeyFromLink((char*)args, "Hgameobject_entry");
            if (!id)
                return false;

            uint32 objectId = atoul(id);

            if (objectId)
                result = WorldDatabase.PQuery("SELECT guid, id, position_x, position_y, position_z, orientation, map, PhaseId, PhaseGroup, (POW(position_x - '%f', 2) + POW(position_y - '%f', 2) + POW(position_z - '%f', 2)) AS order_ FROM gameobject WHERE map = '%i' AND id = '%u' ORDER BY order_ ASC LIMIT 1",
                player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetMapId(), objectId);
            else
            {
                std::string name = id;
                WorldDatabase.EscapeString(name);
                result = WorldDatabase.PQuery(
                    "SELECT guid, id, position_x, position_y, position_z, orientation, map, PhaseId, PhaseGroup, (POW(position_x - %f, 2) + POW(position_y - %f, 2) + POW(position_z - %f, 2)) AS order_ "
                    "FROM gameobject, gameobject_template WHERE gameobject_template.entry = gameobject.id AND map = %i AND name " _LIKE_" " _CONCAT3_("'%%'", "'%s'", "'%%'")" ORDER BY order_ ASC LIMIT 1",
                    player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetMapId(), name.c_str());
            }
        }
        else
        {
            std::ostringstream eventFilter;
            eventFilter << " AND (eventEntry IS NULL ";
            bool initString = true;

            for (GameEventMgr::ActiveEvents::const_iterator itr = activeEventsList.begin(); itr != activeEventsList.end(); ++itr)
            {
                if (initString)
                {
                    eventFilter  <<  "OR eventEntry IN (" << *itr;
                    initString = false;
                }
                else
                    eventFilter << ',' << *itr;
            }

            if (!initString)
                eventFilter << "))";
            else
                eventFilter << ')';

            result = WorldDatabase.PQuery("SELECT gameobject.guid, id, position_x, position_y, position_z, orientation, map, PhaseId, PhaseGroup, "
                "(POW(position_x - %f, 2) + POW(position_y - %f, 2) + POW(position_z - %f, 2)) AS order_ FROM gameobject "
                "LEFT OUTER JOIN game_event_gameobject on gameobject.guid = game_event_gameobject.guid WHERE map = '%i' %s ORDER BY order_ ASC LIMIT 10",
                handler->GetSession()->GetPlayer()->GetPositionX(), handler->GetSession()->GetPlayer()->GetPositionY(), handler->GetSession()->GetPlayer()->GetPositionZ(),
                handler->GetSession()->GetPlayer()->GetMapId(), eventFilter.str().c_str());
        }

        if (!result)
        {
            handler->SendSysMessage(LANG_COMMAND_TARGETOBJNOTFOUND);
            return true;
        }

        bool found = false;
        float x, y, z, o;
        ObjectGuid::LowType guidLow;
        uint32 id, phaseId, phaseGroup;
        uint16 mapId;
        uint32 poolId;

        do
        {
            Field* fields = result->Fetch();
            guidLow =       fields[0].GetUInt64();
            id =            fields[1].GetUInt32();
            x =             fields[2].GetFloat();
            y =             fields[3].GetFloat();
            z =             fields[4].GetFloat();
            o =             fields[5].GetFloat();
            mapId =         fields[6].GetUInt16();
            phaseId =       fields[7].GetUInt32();
            phaseGroup =    fields[8].GetUInt32();
            poolId =  sPoolMgr->IsPartOfAPool<GameObject>(guidLow);
            if (!poolId || sPoolMgr->IsSpawnedObject<GameObject>(guidLow))
                found = true;
        } while (result->NextRow() && !found);

        if (!found)
        {
            handler->PSendSysMessage(LANG_GAMEOBJECT_NOT_EXIST, id);
            return false;
        }

        GameObjectTemplate const* objectInfo = sObjectMgr->GetGameObjectTemplate(id);

        if (!objectInfo)
        {
            handler->PSendSysMessage(LANG_GAMEOBJECT_NOT_EXIST, id);
            return false;
        }

        GameObject* target = handler->GetSession()->GetPlayer()->GetMap()->GetGameObject(ObjectGuid::Create<HighGuid::GameObject>(mapId, id, guidLow));

        handler->PSendSysMessage(LANG_GAMEOBJECT_DETAIL, guidLow, objectInfo->name.c_str(), guidLow, id, x, y, z, mapId, o, phaseId, phaseGroup);

        if (target)
        {
            int32 curRespawnDelay = int32(target->GetRespawnTimeEx() - time(NULL));
            if (curRespawnDelay < 0)
                curRespawnDelay = 0;

            std::string curRespawnDelayStr = secsToTimeString(curRespawnDelay, true);
            std::string defRespawnDelayStr = secsToTimeString(target->GetRespawnDelay(), true);

            handler->PSendSysMessage(LANG_COMMAND_RAWPAWNTIMES, defRespawnDelayStr.c_str(), curRespawnDelayStr.c_str());
        }
        return true;
    }

	static ObjectGuid::LowType HandleQuickGobDelCommand(ChatHandler* handler, char const* args){
		Player* player = handler->GetSession()->GetPlayer();
		if (player->GetSession()->GetSecurity() < 2)
		{
			return NULL;
		}
		QueryResult result;
		GameEventMgr::ActiveEvents const& activeEventsList = sGameEventMgr->GetActiveEventList();

		if (*args)
		{
			// number or [name] Shift-click form |color|Hgameobject_entry:go_id|h[name]|h|r
			char* id = handler->extractKeyFromLink((char*)args, "Hgameobject_entry");
			if (!id)
				return NULL;

			uint32 objectId = atoul(id);

			if (objectId)
				result = WorldDatabase.PQuery("SELECT guid, id, position_x, position_y, position_z, orientation, map, PhaseId, PhaseGroup, (POW(position_x - '%f', 2) + POW(position_y - '%f', 2) + POW(position_z - '%f', 2)) AS order_ FROM gameobject WHERE map = '%i' AND id = '%u' ORDER BY order_ ASC LIMIT 1",
				player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetMapId(), objectId);
			else
			{
				std::string name = id;
				WorldDatabase.EscapeString(name);
				result = WorldDatabase.PQuery(
					"SELECT guid, id, position_x, position_y, position_z, orientation, map, PhaseId, PhaseGroup, (POW(position_x - %f, 2) + POW(position_y - %f, 2) + POW(position_z - %f, 2)) AS order_ "
					"FROM gameobject, gameobject_template WHERE gameobject_template.entry = gameobject.id AND map = %i AND name " _LIKE_" " _CONCAT3_("'%%'", "'%s'", "'%%'")" ORDER BY order_ ASC LIMIT 1",
					player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetMapId(), name.c_str());
			}
		}
		else
		{
			std::ostringstream eventFilter;
			eventFilter << " AND (eventEntry IS NULL ";
			bool initString = true;

			for (GameEventMgr::ActiveEvents::const_iterator itr = activeEventsList.begin(); itr != activeEventsList.end(); ++itr)
			{
				if (initString)
				{
					eventFilter << "OR eventEntry IN (" << *itr;
					initString = false;
				}
				else
					eventFilter << ',' << *itr;
			}

			if (!initString)
				eventFilter << "))";
			else
				eventFilter << ')';

			result = WorldDatabase.PQuery("SELECT gameobject.guid, id, position_x, position_y, position_z, orientation, map, PhaseId, PhaseGroup, "
				"(POW(position_x - %f, 2) + POW(position_y - %f, 2) + POW(position_z - %f, 2)) AS order_ FROM gameobject "
				"LEFT OUTER JOIN game_event_gameobject on gameobject.guid = game_event_gameobject.guid WHERE map = '%i' %s ORDER BY order_ ASC LIMIT 10",
				handler->GetSession()->GetPlayer()->GetPositionX(), handler->GetSession()->GetPlayer()->GetPositionY(), handler->GetSession()->GetPlayer()->GetPositionZ(),
				handler->GetSession()->GetPlayer()->GetMapId(), eventFilter.str().c_str());
		}

		if (!result)
		{
			handler->SendSysMessage(LANG_COMMAND_TARGETOBJNOTFOUND);
			return NULL;
		}

		bool found = false;
		float x, y, z, o;
		ObjectGuid::LowType guidLow;
		uint32 id, phaseId, phaseGroup;
		uint16 mapId;
		uint32 poolId;

		do
		{
			Field* fields = result->Fetch();
			guidLow = fields[0].GetUInt64();
			id = fields[1].GetUInt32();
			x = fields[2].GetFloat();
			y = fields[3].GetFloat();
			z = fields[4].GetFloat();
			o = fields[5].GetFloat();
			mapId = fields[6].GetUInt16();
			phaseId = fields[7].GetUInt32();
			phaseGroup = fields[8].GetUInt32();
			poolId = sPoolMgr->IsPartOfAPool<GameObject>(guidLow);
			if (!poolId || sPoolMgr->IsSpawnedObject<GameObject>(guidLow))
				found = true;
		} while (result->NextRow() && !found);

		if (!found)
		{
			handler->PSendSysMessage(LANG_GAMEOBJECT_NOT_EXIST, id);
			return NULL;
		}

		GameObjectTemplate const* objectInfo = sObjectMgr->GetGameObjectTemplate(id);

		if (!objectInfo)
		{
			handler->PSendSysMessage(LANG_GAMEOBJECT_NOT_EXIST, id);
			return NULL;
		}

		GameObject* target = handler->GetSession()->GetPlayer()->GetMap()->GetGameObject(ObjectGuid::Create<HighGuid::GameObject>(mapId, id, guidLow));

		handler->PSendSysMessage(LANG_GAMEOBJECT_DETAIL, guidLow, objectInfo->name.c_str(), guidLow, id, x, y, z, mapId, o, phaseId, phaseGroup);

		if (target)
		{
			int32 curRespawnDelay = int32(target->GetRespawnTimeEx() - time(NULL));
			if (curRespawnDelay < 0)
				curRespawnDelay = 0;

			std::string curRespawnDelayStr = secsToTimeString(curRespawnDelay, true);
			std::string defRespawnDelayStr = secsToTimeString(target->GetRespawnDelay(), true);

			handler->PSendSysMessage(LANG_COMMAND_RAWPAWNTIMES, defRespawnDelayStr.c_str(), curRespawnDelayStr.c_str());
		}
		return guidLow;
	}

    //delete object by selection or guid
    static bool HandleGameObjectDeleteCommand(ChatHandler* handler, char const* args)
    {
        // number or [name] Shift-click form |color|Hgameobject:go_guid|h[name]|h|r
        char* id = handler->extractKeyFromLink((char*)args, "Hgameobject");
		ObjectGuid::LowType guidLow;
		if (!id)
		{
			guidLow = HandleQuickGobDelCommand(handler, args);
			if (guidLow == NULL)
				return false;
		}
        else
			guidLow = strtoull(id, nullptr, 10);
        if (!guidLow)
            return false;

        GameObject* object = NULL;

        // by DB guid
        if (GameObjectData const* gameObjectData = sObjectMgr->GetGOData(guidLow))
            object = handler->GetObjectGlobalyWithGuidOrNearWithDbGuid(guidLow, gameObjectData->id);

        if (!object)
        {
            handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, guidLow);
            handler->SetSentErrorMessage(true);
            return false;
        }

        ObjectGuid ownerGuid = object->GetOwnerGUID();
        if (!ownerGuid.IsEmpty())
        {
            Unit* owner = ObjectAccessor::GetUnit(*handler->GetSession()->GetPlayer(), ownerGuid);
            if (!owner || !ownerGuid.IsPlayer())
            {
                handler->PSendSysMessage(LANG_COMMAND_DELOBJREFERCREATURE, ownerGuid.ToString().c_str(), object->GetGUID().ToString().c_str());
                handler->SetSentErrorMessage(true);
                return false;
            }

            owner->RemoveGameObject(object, false);
        }

        object->SetRespawnTime(0);                                 // not save respawn time
        object->Delete();
        object->DeleteFromDB();

        handler->PSendSysMessage(LANG_COMMAND_DELOBJMESSAGE, object->GetGUID().ToString().c_str());

        return true;
    }

    //turn selected object
    static bool HandleGameObjectTurnCommand(ChatHandler* handler, char const* args)
    {
        // number or [name] Shift-click form |color|Hgameobject:go_id|h[name]|h|r
        char* id = handler->extractKeyFromLink((char*)args, "Hgameobject");
        if (!id)
            return false;

        ObjectGuid::LowType guidLow = strtoull(id, nullptr, 10);
        if (!guidLow)
            return false;

        GameObject* object = NULL;

        // by DB guid
        if (GameObjectData const* gameObjectData = sObjectMgr->GetGOData(guidLow))
            object = handler->GetObjectGlobalyWithGuidOrNearWithDbGuid(guidLow, gameObjectData->id);

        if (!object)
        {
            handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, guidLow);
            handler->SetSentErrorMessage(true);
            return false;
        }

        char* orientation = strtok(NULL, " ");
        float o;

        if (orientation)
            o = (float)atof(orientation);
        else
        {
            Player* player = handler->GetSession()->GetPlayer();
            o = player->GetOrientation();
        }

        object->Relocate(object->GetPositionX(), object->GetPositionY(), object->GetPositionZ(), o);
        object->RelocateStationaryPosition(object->GetPositionX(), object->GetPositionY(), object->GetPositionZ(), o);
        object->UpdateRotationFields();
        object->DestroyForNearbyPlayers();
        object->UpdateObjectVisibility();

        object->SaveToDB();

        handler->PSendSysMessage(LANG_COMMAND_TURNOBJMESSAGE, object->GetSpawnId(), object->GetGOInfo()->name.c_str(), object->GetGUID().ToString().c_str(), o);

        return true;
    }

    //move selected object
    static bool HandleGameObjectMoveCommand(ChatHandler* handler, char const* args)
    {
        // number or [name] Shift-click form |color|Hgameobject:go_guid|h[name]|h|r
        char* id = handler->extractKeyFromLink((char*)args, "Hgameobject");
        if (!id)
            return false;

        ObjectGuid::LowType guidLow = strtoull(id, nullptr, 10);
        if (!guidLow)
            return false;

        GameObject* object = NULL;

        // by DB guid
        if (GameObjectData const* gameObjectData = sObjectMgr->GetGOData(guidLow))
            object = handler->GetObjectGlobalyWithGuidOrNearWithDbGuid(guidLow, gameObjectData->id);

        if (!object)
        {
            handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, guidLow);
            handler->SetSentErrorMessage(true);
            return false;
        }

        char* toX = strtok(NULL, " ");
        char* toY = strtok(NULL, " ");
        char* toZ = strtok(NULL, " ");

		float x, y, z;
		Player* player = handler->GetSession()->GetPlayer();
        if (!toX)
            player->GetPosition(x, y, z);
		
        else
        {
            if (!toY || !toZ)
                return false;

            x = (float)atof(toX);
            y = (float)atof(toY);
            z = (float)atof(toZ);

            if (!MapManager::IsValidMapCoord(object->GetMapId(), x, y, z))
            {
                handler->PSendSysMessage(LANG_INVALID_TARGET_COORD, x, y, object->GetMapId());
                handler->SetSentErrorMessage(true);
                return false;
            }
        }

        object->DestroyForNearbyPlayers();
        object->RelocateStationaryPosition(x, y, z, object->GetOrientation());
        object->GetMap()->GameObjectRelocation(object, x, y, z, object->GetOrientation());

        object->SaveToDB();

        handler->PSendSysMessage(LANG_COMMAND_MOVEOBJMESSAGE, object->GetSpawnId(), object->GetGOInfo()->name.c_str(), object->GetGUID().ToString().c_str());

        return true;
    }

    // Code Style = )
    static bool HandleGameObjectSetPhaseCommand(ChatHandler* handler, char const* args)
    {
			if (!*args)
				return false;

			char* id = handler->extractKeyFromLink((char*)args, "Hgameobject");
			if (!id)
				return false;

			uint32 guidLow = atoi(id);
			if (!guidLow)
				return false;

			GameObject* object = NULL;

			// via db
			if (GameObjectData const* goData = sObjectMgr->GetGOData(guidLow))
				object = handler->GetObjectGlobalyWithGuidOrNearWithDbGuid(guidLow, goData->id);

			if (!object)
			{
				handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, guidLow);
				handler->SetSentErrorMessage(true);
				return false;
			}
		char* phase = strtok(NULL, " ");
		if (phase == NULL)
		{
			handler->SendSysMessage("Pas de phase, pas de set phase !");
			return false;
		}
		uint32 phaseId = (uint32)atoi(phase);
        if (phaseId == 0)
        {
            handler->SendSysMessage(LANG_BAD_VALUE);
            handler->SetSentErrorMessage(true);
            return false;
        }
		object->SetInPhase(phaseId, true, true);
		object->SaveToDBWithPhase(phaseId);

        return true;
    }

    static bool HandleGameObjectNearCommand(ChatHandler* handler, char const* args)
    {
        float distance = (!*args) ? 10.0f : (float)(atof(args));
        uint32 count = 0;

        Player* player = handler->GetSession()->GetPlayer();

        PreparedStatement* stmt = WorldDatabase.GetPreparedStatement(WORLD_SEL_GAMEOBJECT_NEAREST);
        stmt->setFloat(0, player->GetPositionX());
        stmt->setFloat(1, player->GetPositionY());
        stmt->setFloat(2, player->GetPositionZ());
        stmt->setUInt32(3, player->GetMapId());
        stmt->setFloat(4, player->GetPositionX());
        stmt->setFloat(5, player->GetPositionY());
        stmt->setFloat(6, player->GetPositionZ());
        stmt->setFloat(7, distance * distance);
        PreparedQueryResult result = WorldDatabase.Query(stmt);

        if (result)
        {
            do
            {
                Field* fields = result->Fetch();
                ObjectGuid::LowType guid = fields[0].GetUInt64();
                uint32 entry = fields[1].GetUInt32();
                float x = fields[2].GetFloat();
                float y = fields[3].GetFloat();
                float z = fields[4].GetFloat();
                uint16 mapId = fields[5].GetUInt16();

                GameObjectTemplate const* gameObjectInfo = sObjectMgr->GetGameObjectTemplate(entry);

                if (!gameObjectInfo)
                    continue;

                handler->PSendSysMessage(LANG_GO_LIST_CHAT, guid, entry, guid, gameObjectInfo->name.c_str(), x, y, z, mapId);

                ++count;
            } while (result->NextRow());
        }

        handler->PSendSysMessage(LANG_COMMAND_NEAROBJMESSAGE, distance, count);
        return true;
    }

    //show info of gameobject
    static bool HandleGameObjectInfoCommand(ChatHandler* handler, char const* args)
    {
        uint32 entry = 0;
        uint32 type = 0;
        uint32 displayId = 0;
        std::string name;
        uint32 lootId = 0;

        if (!*args)
        {
            if (WorldObject* object = handler->getSelectedObject())
                entry = object->GetEntry();
            else
                entry = atoi((char*)args);
        } else
                entry = atoi((char*)args);

        GameObjectTemplate const* gameObjectInfo = sObjectMgr->GetGameObjectTemplate(entry);

        if (!gameObjectInfo)
            return false;

        type = gameObjectInfo->type;
        displayId = gameObjectInfo->displayId;
        name = gameObjectInfo->name;
        lootId = gameObjectInfo->GetLootId();

        handler->PSendSysMessage(LANG_GOINFO_ENTRY, entry);
        handler->PSendSysMessage(LANG_GOINFO_TYPE, type);
        handler->PSendSysMessage(LANG_GOINFO_LOOTID, lootId);
        handler->PSendSysMessage(LANG_GOINFO_DISPLAYID, displayId);
        handler->PSendSysMessage(LANG_GOINFO_NAME, name.c_str());

        return true;
    }

    static bool HandleGameObjectSetStateCommand(ChatHandler* handler, char const* args)
    {
        // number or [name] Shift-click form |color|Hgameobject:go_id|h[name]|h|r
        char* id = handler->extractKeyFromLink((char*)args, "Hgameobject");
        if (!id)
            return false;

        ObjectGuid::LowType guidLow = strtoull(id, nullptr, 10);
        if (!guidLow)
            return false;

        GameObject* object = NULL;

        if (GameObjectData const* gameObjectData = sObjectMgr->GetGOData(guidLow))
            object = handler->GetObjectGlobalyWithGuidOrNearWithDbGuid(guidLow, gameObjectData->id);

        if (!object)
        {
            handler->PSendSysMessage(LANG_COMMAND_OBJNOTFOUND, guidLow);
            handler->SetSentErrorMessage(true);
            return false;
        }

        char* type = strtok(NULL, " ");
        if (!type)
            return false;

        int32 objectType = atoi(type);
        if (objectType < 0)
        {
            if (objectType == -1)
                object->SendObjectDeSpawnAnim(object->GetGUID());
            else if (objectType == -2)
                return false;
            return true;
        }

        char* state = strtok(NULL, " ");
        if (!state)
            return false;

        int32 objectState = atoi(state);

        if (objectType < 4)
            object->SetByteValue(GAMEOBJECT_BYTES_1, objectType, objectState);
        else if (objectType == 4)
            object->SendCustomAnim(objectState);

        handler->PSendSysMessage("Set gobject type %d state %d", objectType, objectState);
        return true;
    }
};

void AddSC_gobject_commandscript()
{
    new gobject_commandscript();
}
