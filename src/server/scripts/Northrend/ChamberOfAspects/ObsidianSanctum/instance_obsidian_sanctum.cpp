/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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

#include "ScriptPCH.h"
#include "obsidian_sanctum.h"

#define MAX_ENCOUNTER     4

/* Obsidian Sanctum encounters:
0 - Sartharion
*/

class instance_obsidian_sanctum : public InstanceMapScript
{
public:
    instance_obsidian_sanctum() : InstanceMapScript("instance_obsidian_sanctum", 615) { }

    InstanceScript* GetInstanceScript(InstanceMap* map) const
    {
        return new instance_obsidian_sanctum_InstanceMapScript(map);
    }

    struct instance_obsidian_sanctum_InstanceMapScript : public InstanceScript
    {
        instance_obsidian_sanctum_InstanceMapScript(Map* map) : InstanceScript(map) {}

        uint32 m_auiEncounter[MAX_ENCOUNTER];
        uint64 m_uiSartharionGUID;
        uint64 m_uiTenebronGUID;
        uint64 m_uiShadronGUID;
        uint64 m_uiVesperonGUID;

        std::string str_data;

        void Initialize()
        {
            memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));

            m_uiSartharionGUID = 0;
            m_uiTenebronGUID   = 0;
            m_uiShadronGUID    = 0;
            m_uiVesperonGUID   = 0;
        }

        std::string GetSaveData()
        {
            OUT_SAVE_INST_DATA;

            std::ostringstream saveStream;

            saveStream << "O B" << m_auiEncounter[0]
                << " " << m_auiEncounter[1]
                << " " << m_auiEncounter[2]
                << " " << m_auiEncounter[3];

            str_data = saveStream.str();

            OUT_SAVE_INST_DATA_COMPLETE;
            return str_data;
        }

        void Load(const char* in)
        {
            if (!in)
            {
                OUT_LOAD_INST_DATA_FAIL;
                return;
            }

            OUT_LOAD_INST_DATA(in);

            char dataHead1, dataHead2;
            uint16 data0, data1, data2, data3;

            std::istringstream loadStream(in);
            loadStream >> dataHead1 >> dataHead2 >> data0 >> data1 >> data2 >> data3;

            if (dataHead1 == 'O' && dataHead2 == 'B')
            {
                m_auiEncounter[0] = data0;
                m_auiEncounter[1] = data1;
                m_auiEncounter[2] = data2;
                m_auiEncounter[3] = data3;

                for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
                {
                    if (m_auiEncounter[i] == IN_PROGRESS)
                        m_auiEncounter[i] = NOT_STARTED;
                }
            } else OUT_LOAD_INST_DATA_FAIL;

            OUT_LOAD_INST_DATA_COMPLETE;
        }

        bool IsEncounterInProgress() const
        {
            for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
                if (m_auiEncounter[i] == IN_PROGRESS)
                    return true;

            return false;
        }

        void OnCreatureCreate(Creature* creature)
        {
            switch (creature->GetEntry())
            {
                case NPC_SARTHARION:
                    m_uiSartharionGUID = creature->GetGUID();
                    break;
                //three dragons below set to active state once created.
                //we must expect bigger raid to encounter main boss, and then three dragons must be active due to grid differences
                case NPC_TENEBRON:
                    m_uiTenebronGUID = creature->GetGUID();
                    creature->setActive(true);
                    break;
                case NPC_SHADRON:
                    m_uiShadronGUID = creature->GetGUID();
                    creature->setActive(true);
                    break;
                case NPC_VESPERON:
                    m_uiVesperonGUID = creature->GetGUID();
                    creature->setActive(true);
                    break;
            }
        }

        void SetData(uint32 uiType, uint32 uiData)
        {
            if (uiType == TYPE_SARTHARION_EVENT)
                m_auiEncounter[0] = uiData;
            else if (uiType == TYPE_TENEBRON_PREKILLED)
                m_auiEncounter[1] = uiData;
            else if (uiType == TYPE_SHADRON_PREKILLED)
                m_auiEncounter[2] = uiData;
            else if (uiType == TYPE_VESPERON_PREKILLED)
                m_auiEncounter[3] = uiData;

            if (uiData == DONE)
                SaveToDB();
        }

        uint32 GetData(uint32 uiType)
        {
            if (uiType == TYPE_SARTHARION_EVENT)
                return m_auiEncounter[0];
            else if (uiType == TYPE_TENEBRON_PREKILLED)
                return m_auiEncounter[1];
            else if (uiType == TYPE_SHADRON_PREKILLED)
                return m_auiEncounter[2];
            else if (uiType == TYPE_VESPERON_PREKILLED)
                return m_auiEncounter[3];

            return 0;
        }

        uint64 GetData64(uint32 uiData)
        {
            switch (uiData)
            {
                case DATA_SARTHARION:
                    return m_uiSartharionGUID;
                case DATA_TENEBRON:
                    return m_uiTenebronGUID;
                case DATA_SHADRON:
                    return m_uiShadronGUID;
                case DATA_VESPERON:
                    return m_uiVesperonGUID;
            }
            return 0;
        }
    };

};

void AddSC_instance_obsidian_sanctum()
{
    new instance_obsidian_sanctum();
}
