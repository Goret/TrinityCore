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

/* ScriptData
SDName: Boss Black Knight
SD%Complete: 80%
SDComment: missing yells. not sure about timers.
SDCategory: Trial of the Champion
EndScriptData */

#include "ScriptPCH.h"
#include "ScriptedEscortAI.h"
#include "trial_of_the_champion.h"
#include "Vehicle.h"

enum eEnums
{
    //Yell
    SAY_DEATH_3                             = -1999935,
    SAY_AGGRO                               = -1999929,
    SAY_AGGRO_2                             = -1999930,
    SAY_SLAY                                = -1999932,
    SAY_DEATH_1                             = -1999933,
    SAY_DEATH                               = -1999934,
    SAY_START5                              = -1999936,
    SAY_START6                              = -1999937,
    SAY_START7                              = -1999928,
    SAY_START8                              = -1999929,
    SAY_START9                              = -1999952,
    SAY_START10                             = -1999932,
    SAY_START11                             = -1999953,
};
enum eSpells
{
    //phase 1
    SPELL_PLAGUE_STRIKE     = 67724,
    SPELL_ICY_TOUCH         = 67718,
    SPELL_ICY_TOUCH_H       = 67881,
    SPELL_DEATH_RESPITE     = 67745,
    SPELL_OBLITERATE        = 67725,
    SPELL_OBLITERATE_H      = 67883,

    //phase 2 - During this phase, the Black Knight will use the same abilities as in phase 1, except for Death's Respite
    SPELL_ARMY_DEAD         = 67761,
    SPELL_DESECRATION       = 68766,
    SPELL_GHOUL_EXPLODE     = 67751,

    //phase 3
    SPELL_DEATH_BITE        = 67875,
    SPELL_DEATH_BITE_H      = 67808,
    SPELL_MARKED_DEATH      = 67882,

    SPELL_BLACK_KNIGHT_RES  = 67693,

    SPELL_LEAP              = 67749,
    SPELL_LEAP_H            = 67880,

    SPELL_KILL_CREDIT       = 68663
};

enum eModels
{
    MODEL_SKELETON = 29846,
    MODEL_GHOST    = 21300
};

enum eEquip
{
     EQUIP_SWORD               = 40343
};

enum IntroPhase
{
    IDLE,
    INTRO,
    FINISHED
};

enum ePhases
{
    PHASE_UNDEAD    = 1,
    PHASE_SKELETON  = 2,
    PHASE_GHOST     = 3
};

class boss_black_knight : public CreatureScript
{
public:
    boss_black_knight() : CreatureScript("boss_black_knight") { }

    struct boss_black_knightAI : public ScriptedAI
    {
        boss_black_knightAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;

        std::list<uint64> SummonList;

        bool bEventInProgress;
        bool bEvent;
        bool bSummonArmy;
        bool bDeathArmyDone;
        bool bEventInBattle;
        bool bFight;

        uint8 uiPhase;
        uint8 uiIntroPhase;

        IntroPhase Phase;

        uint32 uiIntroTimer;
        uint32 uiPlagueStrikeTimer;
        uint32 uiPlagueStrike1Timer;
        uint32 uiIcyTouchTimer;
        uint32 uiIcyTouch1Timer;
        uint32 uiDeathRespiteTimer;
        uint32 uiObliterateTimer;
        uint32 uiObliterate1Timer;
        uint32 uiDesecrationTimer;
        uint32 uiResurrectTimer;
        uint32 uiDeathArmyCheckTimer;
        uint32 uiGhoulExplodeTimer;
        uint32 uiDeathBiteTimer;
        uint32 uiMarkedDeathTimer;

        void Reset()
        {
            RemoveSummons();
            me->SetDisplayId(me->GetNativeDisplayId());
            me->ClearUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED);

            bEventInProgress = false;
            bEvent = false;
            bSummonArmy = false;
            bDeathArmyDone = false;
            bFight = false;

            if (instance)
            {
                if (GameObject* go = GameObject::GetGameObject(*me, instance->GetData64(DATA_MAIN_GATE)))
                    instance->HandleGameObject(go->GetGUID(), false);

                if (GameObject* go = GameObject::GetGameObject(*me, instance->GetData64(DATA_MAIN_GATE1)))
                    instance->HandleGameObject(go->GetGUID(), true);
            }

            if (bEventInBattle)
            {
                me->GetMotionMaster()->MovePoint(1, 743.396f, 635.411f, 411.575f);
                me->setFaction(14);
                me->SetReactState(REACT_AGGRESSIVE);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE|UNIT_FLAG_NOT_SELECTABLE);
            }

            uiPhase = PHASE_UNDEAD;

            uiIcyTouchTimer = urand(5000, 9000);
            uiIcyTouch1Timer = urand(15000, 15000);
            uiPlagueStrikeTimer = urand(10000, 13000);
            uiDeathRespiteTimer = 17000;
            uiPlagueStrike1Timer = urand(14000, 14000);
            uiObliterateTimer = urand(17000, 19000);
            uiObliterate1Timer = urand(15000, 15000);
            uiDesecrationTimer = urand(15000, 16000);
            uiDesecrationTimer = 22000;
            uiDeathArmyCheckTimer = 7000;
            uiResurrectTimer = 4000;
            uiGhoulExplodeTimer = 8000;
            uiDeathBiteTimer = urand (2000, 4000);
            uiMarkedDeathTimer = urand (5000, 7000);
            uiIntroTimer = 5000;
        }

        void RemoveSummons()
        {
            if (SummonList.empty())
                return;

            for (std::list<uint64>::const_iterator itr = SummonList.begin(); itr != SummonList.end(); ++itr)
            {
                if (Creature* temp = Unit::GetCreature(*me, *itr))
                {
                    if (temp)
                    {
                        // Let all remaining ghouls explode
                        if ((temp->GetEntry() == 35590 || temp->GetEntry() == 12444) && temp->isAlive())
                        {
                            me->CastSpell(temp, SPELL_GHOUL_EXPLODE, true);
                        }
                        else
                            temp->DisappearAndDie();
                    }
                }
            }
            SummonList.clear();
        }

        void JustSummoned(Creature* summon)
        {
            SummonList.push_back(summon->GetGUID());
            summon->AI()->AttackStart(me->getVictim());
        }

        void UpdateAI(const uint32 uiDiff)
        {

            if (!UpdateVictim())
                return;

            if (bEventInProgress)
                if (uiResurrectTimer <= uiDiff)
                {
                    me->SetFullHealth();
                    me->AttackStop();

                    switch (uiPhase)
                    {
                        case PHASE_UNDEAD:
                            DoScriptText(SAY_DEATH_1, me);
                            break;
                        case PHASE_SKELETON:
                            DoScriptText(SAY_DEATH, me);
                            break;
                    }

                    DoCast(me, SPELL_BLACK_KNIGHT_RES, true);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    uiPhase++;
                    uiResurrectTimer = 3000;
                    bEventInProgress = false;
                    me->ClearUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED);
                } else uiResurrectTimer -= uiDiff;
                
            switch (uiPhase)
            {
                case PHASE_UNDEAD:
                {
                    if (uiPlagueStrikeTimer <= uiDiff)
                    {
                        DoCastVictim(SPELL_PLAGUE_STRIKE);
                        uiPlagueStrikeTimer = urand(12000, 15000);
                    } else uiPlagueStrikeTimer -= uiDiff;
                    
                    if (uiObliterateTimer <= uiDiff)
                    {
                        DoCastVictim(DUNGEON_MODE(SPELL_OBLITERATE, SPELL_OBLITERATE_H));
                        uiObliterateTimer = urand(17000, 19000);
                    } else uiObliterateTimer -= uiDiff;
                    
                    if (uiIcyTouchTimer <= uiDiff)
                    {
                        DoCastVictim(DUNGEON_MODE(SPELL_ICY_TOUCH, SPELL_ICY_TOUCH_H));
                        uiIcyTouchTimer = urand(5000, 7000);
                    } else uiIcyTouchTimer -= uiDiff;
                    break;
                }
                case PHASE_SKELETON:
                {
                    if (!bSummonArmy)
                    {
                        bSummonArmy = true;
                        me->AddUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED);
                        me->SetFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_NON_ATTACKABLE);
                        DoCast(me, SPELL_ARMY_DEAD);
                    }

                    if (!bDeathArmyDone)
                        if (uiDeathArmyCheckTimer <= uiDiff)
                        {
                            me->ClearUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED);
                            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                            uiDeathArmyCheckTimer = 0;
                            bDeathArmyDone = true;
                        } else uiDeathArmyCheckTimer -= uiDiff;
                    
                    if (uiDesecrationTimer <= uiDiff)
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                        {
                            if (target->isAlive())
                                DoCast(target, SPELL_DESECRATION);
                        }
                        uiDesecrationTimer = urand(15000,16000);
                    } else uiDesecrationTimer -= uiDiff;
                        
                    if (uiGhoulExplodeTimer <= uiDiff)
                    {
                        if (!SummonList.empty())
                        {
                            for(std::list<uint64>::const_iterator itr = SummonList.begin(); itr != SummonList.end(); ++itr)
                            {
                                if (Creature* temp = Unit::GetCreature(*me, *itr))
                                {
                                    if (temp)
                                    {
                                        // Let all remaining ghouls explode
                                        if (temp->GetEntry() == 35590 || temp->GetEntry() == 12444)
                                        {
                                            if (temp->isAlive())
                                            {
                                                me->CastSpell(temp, SPELL_GHOUL_EXPLODE, true);
                                                break;
                                            }
                                            else
                                                continue;
                                        }
                                    }
                                }
                            }
                        }

                        uiGhoulExplodeTimer = 8000;
                    } else uiGhoulExplodeTimer -= uiDiff;
                    
                    if (uiPlagueStrike1Timer <= uiDiff)
                    {
                        DoCastVictim(SPELL_PLAGUE_STRIKE);
                        uiPlagueStrike1Timer = urand(12000, 15000);
                    } else uiPlagueStrike1Timer -= uiDiff;
                    
                    if (uiObliterate1Timer <= uiDiff)
                    {
                        DoCastVictim(SPELL_OBLITERATE);
                        uiObliterate1Timer = urand(17000, 19000);
                    } else uiObliterate1Timer -= uiDiff;
                    
                    if (uiIcyTouch1Timer <= uiDiff)
                    {
                        DoCastVictim(SPELL_ICY_TOUCH);
                        uiIcyTouch1Timer = urand(5000, 7000);
                    } else uiIcyTouch1Timer -= uiDiff;
                    
                    if (uiDeathRespiteTimer <= uiDiff)
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                        {
                            if (target && target->isAlive())
                            DoCast(target, SPELL_DEATH_RESPITE);
                        }
                        uiDeathRespiteTimer = urand(15000, 16000);
                    } else uiDeathRespiteTimer -= uiDiff;
                    break;
                }

                case PHASE_GHOST:
                {
                    if (uiDeathBiteTimer <= uiDiff)
                    {
                        SetEquipmentSlots(false, EQUIP_UNEQUIP, EQUIP_NO_CHANGE, EQUIP_NO_CHANGE);
                        DoCast(me, DUNGEON_MODE(SPELL_DEATH_BITE, SPELL_DEATH_BITE_H));
                        uiDeathBiteTimer = urand (2000, 4000);
                    } else uiDeathBiteTimer -= uiDiff;
                
                    if (uiMarkedDeathTimer <= uiDiff)
                    {
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                        {
                            if (target && target->isAlive())
                                DoCast(target, SPELL_MARKED_DEATH);
                        }
                        uiMarkedDeathTimer = urand (5000, 7000);
                    } else uiMarkedDeathTimer -= uiDiff;
                    break;
                }
            }

            if (!me->HasUnitState(UNIT_STATE_ROOT) && !me->HealthBelowPct(1))
                DoMeleeAttackIfReady();
        }

        void EnterCombat(Unit* who)
        {
            bEventInBattle = true;
            DoScriptText(SAY_AGGRO_2, me);
            SetEquipmentSlots(false, EQUIP_SWORD, EQUIP_NO_CHANGE, EQUIP_NO_CHANGE);

            if (me->ToTempSummon())
            {
                me->ToTempSummon()->InitStats(9000000);
                me->ToTempSummon()->SetTempSummonType(TEMPSUMMON_CORPSE_TIMED_DESPAWN);
            }

            if (instance)
            {
                if (GameObject* go = GameObject::GetGameObject(*me, instance->GetData64(DATA_MAIN_GATE1)))
                    instance->HandleGameObject(go->GetGUID(), false);

                if (GameObject* go = GameObject::GetGameObject(*me, instance->GetData64(DATA_MAIN_GATE)))
                    instance->HandleGameObject(go->GetGUID(), false);
            }
        }

        void KilledUnit(Unit* /*victim*/)
        {
            if (instance)
                instance->SetData(BOSS_BLACK_KNIGHT, IN_PROGRESS);
        }

        void DamageTaken(Unit* /*pDoneBy*/, uint32& uiDamage)
        {
            if (uiDamage >= me->GetHealth() && uiPhase <= PHASE_SKELETON)
            {
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                uiDamage = 0;
                me->SetHealth(0);
                me->AddUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED);
                RemoveSummons();
                switch (uiPhase)
                {
                    case PHASE_UNDEAD:
                        me->SetDisplayId(MODEL_SKELETON);
                        break;
                    case PHASE_SKELETON:
                        me->SetDisplayId(MODEL_GHOST);
                        SetEquipmentSlots(false, EQUIP_UNEQUIP, EQUIP_NO_CHANGE, EQUIP_NO_CHANGE);
                        break;
                }
                bEventInProgress = true;
            }
        }

        void JustDied(Unit* /*killer*/)
        {
            //DoCast(me, SPELL_KILL_CREDIT);
            DoScriptText(SAY_DEATH_3, me);

            if (instance)
            {
                instance->SetData(BOSS_BLACK_KNIGHT, DONE);

                // Instance encounter counting mechanics
                instance->UpdateEncounterState(ENCOUNTER_CREDIT_CAST_SPELL, 68663, me);

                if (GameObject* go = GameObject::GetGameObject(*me, instance->GetData64(DATA_MAIN_GATE1)))
                    instance->HandleGameObject(go->GetGUID(), true);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_black_knightAI (creature);
    }
};

class npc_risen_ghoul : public CreatureScript
{
public:
    npc_risen_ghoul() : CreatureScript("npc_risen_ghoul") { }

    struct npc_risen_ghoulAI : public ScriptedAI
    {
        npc_risen_ghoulAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 uiAttackTimer;

        void Reset()
        {
            uiAttackTimer = 3500;
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim())
                return;

            if (uiAttackTimer <= uiDiff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 100, true))
                {
                    if (target && target->isAlive())
                        DoCast(target, DUNGEON_MODE(SPELL_LEAP, SPELL_LEAP_H));
                }
                uiAttackTimer = 3500;
            } else uiAttackTimer -= uiDiff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_risen_ghoulAI(creature);
    }
};

class npc_black_knight_skeletal_gryphon : public CreatureScript
{
public:
    npc_black_knight_skeletal_gryphon() : CreatureScript("npc_black_knight_skeletal_gryphon") { }

    struct npc_black_knight_skeletal_gryphonAI : public npc_escortAI
    {
        npc_black_knight_skeletal_gryphonAI(Creature* creature) : npc_escortAI(creature)
        {
            Start(false, true, 0, NULL);
            instance = creature->GetInstanceScript();
        }

        InstanceScript* instance;

        void WaypointReached(uint32 uiPointId)
        {
            switch(uiPointId)
            {
                    case 1:
                        me->SetSpeed(MOVE_FLIGHT, 2.0f);
                        break;
                    case 2:
                        me->SetSpeed(MOVE_FLIGHT, 2.0f);
                        if (instance)
                        {
                            instance->SetData(DATA_BLACK_KNIGHT, NOT_STARTED);
                        }
                        break;
                    case 3:
                        me->SetSpeed(MOVE_FLIGHT, 2.0f);
                        break;
                    case 4:
                        me->SetSpeed(MOVE_FLIGHT, 2.0f);
                        break;
                    case 5:
                        me->SetSpeed(MOVE_FLIGHT, 2.0f);
                        break;
                    case 6:
                        me->SetSpeed(MOVE_FLIGHT, 2.0f);
                        break;
                    case 7:
                        me->SetSpeed(MOVE_FLIGHT, 2.0f);
                        break;
                    case 8:
                        me->SetSpeed(MOVE_FLIGHT, 2.0f);
                        break;
                    case 9:
                        me->SetSpeed(MOVE_FLIGHT, 2.0f);
                        break;
                    case 10:
                        me->SetSpeed(MOVE_FLIGHT, 2.0f);
                        break;
                    case 11:
                        me->SetSpeed(MOVE_FLIGHT, 2.0f);
                        break;
                    case 12:
                        me->SetUnitMovementFlags(MOVEMENTFLAG_WALKING);
                        me->SetSpeed(MOVE_RUN, 2.0f);
                        break;
                    case 13:
                        if (me->GetVehicleKit())
                            me->GetVehicleKit()->RemoveAllPassengers();

                        me->SetUnitMovementFlags(MOVEMENTFLAG_WALKING);
                        break;
                    case 14:
                        me->SetUnitMovementFlags(MOVEMENTFLAG_FLYING);
                        if (instance)
                        {
                            instance->SetData(DATA_BLACK_KNIGHT, IN_PROGRESS);
                        }
                        break;
                    case 15:
                        me->SetUnitMovementFlags(MOVEMENTFLAG_FLYING);
                        if (instance)
                        {
                            instance->SetData(DATA_BLACK_KNIGHT, DONE);
                        }
                        break;
                    case 16:
                        me->SetUnitMovementFlags(MOVEMENTFLAG_FLYING);
                        break;
                    case 17:
                        me->SetUnitMovementFlags(MOVEMENTFLAG_FLYING);
                        if (instance)
                        {
                            instance->SetData(DATA_KNIGHT, NOT_STARTED);
                        }
                        break;
                    case 18:
                        me->SetUnitMovementFlags(MOVEMENTFLAG_FLYING);
                        if (instance)
                        {
                           instance->SetData(DATA_KNIGHT, IN_PROGRESS);
                        }
                        break;
                    case 19:
                        me->SetUnitMovementFlags(MOVEMENTFLAG_FLYING);
                        break;
                    case 20:
                        me->SetUnitMovementFlags(MOVEMENTFLAG_FLYING);
                        break;
                    case 21:
                        me->SetUnitMovementFlags(MOVEMENTFLAG_FLYING);
                        break;
                    case 22:
                        me->SetUnitMovementFlags(MOVEMENTFLAG_WALKING);
                        if (instance)
                        {
                            instance->SetData(DATA_KNIGHT, DONE);
                        }
                        break;
            }
        }

        void UpdateAI(const uint32 uiDiff)
        {
            npc_escortAI::UpdateAI(uiDiff);

            if (!UpdateVictim())
                return;
        }

    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_black_knight_skeletal_gryphonAI(creature);
    }
};

void AddSC_boss_black_knight()
{
    new boss_black_knight();
    new npc_risen_ghoul();
    new npc_black_knight_skeletal_gryphon();
}
