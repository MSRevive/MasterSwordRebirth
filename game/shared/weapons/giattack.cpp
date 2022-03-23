/***
*
*	Copyright (c) 2000, Kenneth "Dogg" Early.
*
*	Email kene@maverickdev.com or
*		  kearly@crosswinds.net
*
****/
/*
	Generic Item Attacks
	��������������������
*/
#ifndef VALVE_DLL
#include "../hud.h"
#include "../cl_util.h"
#endif

#include "inc_weapondefs.h"
#include "global.h"
#include "stats/statdefs.h"
#include "logfile.h"

#ifdef VALVE_DLL
#include "shield.h"
#include "soundent.h"
#endif

//extern iBeam;
bool GetString(char *Return, size_t size, const char *sentence, int start, char *endchars);
#define ReadStat(Name, StatVar, PropVar)            \
	{                                               \
		Temp = GetFirstScriptVar(Name);             \
		if (Temp != Name)                           \
		{                                           \
			StatVar = 0, PropVar = -1;              \
			GetStatIndices(Temp, StatVar, PropVar); \
		}                                           \
	}

bool CGenericItem::CheckKeys(attackdata_t *pAttData)
{
#ifndef VALVE_DLL
	bool fIsActiveItem = (m_pOwner->ActiveItem() == this);

	if (m_Hand == HAND_PLAYERHANDS && !fIsActiveItem)
		return false; //Player hands must be the active item in order to be used
	//MiB JUN2010 - Oh, he can.
	//Thothie - Oh no he can't - until we balance this somehow
	//MiB MAR2012_07 - I SAID YES.
	//if( FBitSet(m_pOwner->m_StatusFlags,PLAYER_MOVE_ATTACKING) ) return false;		//Can't already be attacking

	bool fReturn = false;
	int iLen = 0, iCmds = pAttData->ComboKeys.size();
	int iThisCmd = 0;

	//Count the number of keys required
	for (int i = 0; i < pAttData->ComboKeys.size(); i++)
	{
		if (pAttData->ComboKeys[i][0] == '~')
			iThisCmd += 2;
		else
			iThisCmd++;
	}

	//Count the number of keys required
	/*while( GetString( cTemp, pAttData->cKeyCombo, iLen, "~+-\n" ) ) {
		iLen += strlen(cTemp) + 1;

		if( pAttData->cKeyCombo[iLen-1] == '\n' ) break;

		if( pAttData->cKeyCombo[iLen-1] == '~' ) iThisCmd += 2;
		else iThisCmd++;

		iCmds++;
	}*/

	iThisCmd--; //Subtract 1, because this is zero-based.

	if (iCmds > 10)
	{
		Print("%s - Combos must be under 10 keys!\n", DisplayName());
		return false;
	}
	if (iCmds <= 0)
		return false;

	int KeyMask = 0;
	char LastType = 0;
	int Mask = 0, ClearMask = 0;
	int LastCmdIdx = 0;
	iLen = 0;
	bool fFirstKey = true;
	for (int i = 0; i < pAttData->ComboKeys.size(); i++)
	{
		msstring &FullCmdString = pAttData->ComboKeys[i];

		msstring CmdString = FullCmdString;
		if (FullCmdString.len() > 1 &&
			(FullCmdString[0] == '+' || FullCmdString[0] == '~' || FullCmdString[0] == '-'))
		{
			LastType = FullCmdString[0];
			CmdString = FullCmdString.substr(1);
		}
		else
			LastType = 0;

		if (!LastType)
			continue;

		int *TheMask;
		if (LastType == '+' || LastType == '~')
			TheMask = &Mask;
		else
			TheMask = &ClearMask;

		Mask = ClearMask = 0;

		if (CmdString == "attack1")
			SetBits(*TheMask, fIsActiveItem ? IN_ATTACK : IN_ATTACK2);
		if (CmdString == "attack2")
			SetBits(*TheMask, IN_ATTACK2); //Thothie JUL2010_30
		if (CmdString == "forward")
			SetBits(*TheMask, IN_FORWARD);
		if (CmdString == "back")
			SetBits(*TheMask, IN_BACK);
		if (CmdString == "moveleft")
			SetBits(*TheMask, IN_MOVELEFT);
		if (CmdString == "moveright")
			SetBits(*TheMask, IN_MOVERIGHT);
		if (CmdString == "use")
			SetBits(*TheMask, IN_USE);
		if (CmdString == "jump")
			SetBits(*TheMask, IN_JUMP);

		//Print( "Mask %i (%i)\n", *TheMask, iThisCmd );

		//Check this button mask against the key history
		//Whatever buttons are different from the mask, have been either pressed or released.
		//If those changed bits are a part of the check mask, then continue

		if (LastType == '-')
		{
			if (FBitSet(KeyHistory[iThisCmd].Buttons, ClearMask))
				break;
		}
		else
		{
			if ((KeyHistory[iThisCmd].Buttons ^ *TheMask) & *TheMask)
				break;
		}

		if (!fFirstKey)
		{
			float TimeTooLate = KeyHistory[LastCmdIdx].Time + 0.2;
			if (KeyHistory[iThisCmd].Time > TimeTooLate)
				break;
		}

		//Print( "Pressed %s(%i)\n", cTemp, iThisCmd );

		LastCmdIdx = iThisCmd;

		int IncAmt = 1;
		if (LastType == '~' && iThisCmd > 0)
			IncAmt = 2;

		iThisCmd -= IncAmt;

		if (i == (pAttData->ComboKeys.size() - 1) &&
			(fFirstKey || KeyHistory[0].Time > gpGlobals->time - 0.3))
		{
			fReturn = true;
			break;
		}
		fFirstKey = false;
	}
	//	for( int i = 0; i < 10; i++ )
	//		Print( "[%i] ", KeyHistory[i] );
	//	Print( "\n" );
	//if( fReturn ) Print( "Combo Pri: %i\n", pAttData->iPriority );

	return fReturn;
#endif
	return false;
}

bool CGenericItem::Attack_CanAttack()
{
	if (!m_pOwner || !m_pOwner->IsAlive())
		return false; //Owner must be alive
	if (m_Location != ITEMPOS_HANDS)
		return false; //Must be in owner's hands
	if (!m_Attacks.size())
		return false; //Must have registered an attack
	if (!Spell_CanAttack())
		return false; //Can't be in the process of preparing this spell

	return true;
}

//StartAttack - called every frame to try and start a new attack (along with attack())
//Can be called with a parameter to force an attack
bool CGenericItem::StartAttack(int ForceAttackNum)
{
	startdbg;
	dbg("Begin");

#ifdef VALVE_DLL

	//Check if I have a command queued from the client
	//The client performs and attack and sends the attack command.
	//It's queued in client.cpp and this checks if it's time to perform it yet
	//Without the queue, attacks were being missed. The client was sending attacks slightly before
	//the previous attack was finished, causing the server to reject the second attack.
	if (m_pPlayer && ForceAttackNum == -1)
	{
		// MiB JUN2010_19 - Below was the original conditional for the statement below it. Changed to allow both
		// hands to attack. We don't check what the client is doing with his other hands - only if this item is
		// attacking
		//if( m_pPlayer->m_ClientAttack && m_pPlayer->m_ClientAttackHand == m_Hand )

		if (ClientAttacking)
		{
			bool DoAttack = false;

			//Item keeps track of its own AttackNum
			if (AttackNum == -1) //m_pPlayer->m_ClientAttackNum == -1 )
			{
				m_ReleaseAttack = true;
				if (CurrentAttack)
					CurrentAttack->fAttackReleased = true;
				DoAttack = true;
			}
			//Don't care if the client is already attacking - We do care if he's shielding, though (see below)
			else if (!m_pPlayer->IsShielding() && /*!m_pPlayer->IsActing( ) && */ Attack_CanAttack())
				DoAttack = true;

			if (DoAttack)
			{
				//Again, item keeping track of itself.
				ForceAttackNum = AttackNum; //m_pPlayer->m_ClientAttackNum;
				ClientAttacking = false;	//m_pPlayer->m_ClientAttack = false;
			}
		}
	}
#endif

	dbg("Check Restrictions");
	//MiB JUN2010_19 - Letting both hands attack at the same time. Commented out IsActing as we don't care if another
	//				   weapon is attacking, only if this one is already attacking. We do, however care if the player
	//				   is using a shield. Would be exploitable to allow attacking from behind a shield
	if (!m_pOwner || m_pOwner->IsShielding() //|| m_pOwner->IsActing( ) //Cant attack while acting (sitting, etc.)
		|| !m_pPlayer || FBitSet(m_pPlayer->m_StatusFlags, PLAYER_MOVE_NOATTACK))
		return false;

	int iNewAttack = -1;

	if (ForceAttackNum >= 0)
	{
		dbg("Force Attack");
		if (ForceAttackNum >= (signed)m_Attacks.size())
			return true;

		CurrentAttack = &m_Attacks[ForceAttackNum];
		iNewAttack = ForceAttackNum;
	}
#ifndef VALVE_DLL
	else if (!CurrentAttack)
	{
		dbg("Choose attack");
		//bool thoth_unskill_base = false;
		for (int a = 0; a < m_Attacks.size(); a++)
		{
			attackdata_t &AttData = m_Attacks[a];

			//Take the required keys, remove the pressed keys, and if there aren't any keys
			//left (from the reqired list), the user is pressing all the required keys

			if (!CheckKeys(&AttData))
				continue;

			//SEP2007b if you can't wield profficiently, do not allow secondary atk
			//- this bit would require a client recompile, dont want to do on a B patch
			//if ( a > 0 && thoth_unskill_base ) continue;

			if (m_TimeChargeStart || m_LastChargedAmt)
			{
				//If I just finished an attack and I charged during that attack...

				if (!AttData.flChargeAmt)
					continue; //Don't do 'uncharged' while charging

				float ChargetAmt = m_LastChargedAmt;
				if (ChargetAmt < AttData.flChargeAmt)
				{
					// MiB MAR2012_07 - If charged between 0 and 1, we're going to still let them attack, and powered up
					if (!CurrentAttack)
					{
						CurrentAttack = &m_Attacks[0]; // The real attack we'll be using
						iNewAttack = -2;			   // Special code for later
					}
					continue; // Haven't charged enough for this charge attack
				}

				// Got here - Attempting to start this charged attack
			}
			else if (AttData.flChargeAmt)
				continue; //Don't do 'charge' when not charging

			if (AttData.RequiredSkill) //Do I have the skill required to use this attack
			{
				int Stat = m_pOwner->GetSkillStat(AttData.StatProf, AttData.PropProf);
				//Thothie SEP2007 - reduce accuracy and inc stanima drain instead of making unswingable
				if (Stat < AttData.RequiredSkill && a > 0)
				{
					//thoth_unskill_base = true;
					continue;
				}
			}
			//If this attack isn't forced, compare this attack against the already-selected attack
			if (CurrentAttack)
			{
				if (AttData.iPriority < CurrentAttack->iPriority)
					continue; //Selected attack has higher priority than this one
				if (AttData.iPriority == CurrentAttack->iPriority && !RANDOM_LONG(0, 1))
					continue; //Same priority - random chance of using this attack
			}

			//Mark the attack as selected - Could get overridden with another attack
			if (iNewAttack != -2)
			{
				CurrentAttack = &AttData;
				iNewAttack = a;
			}
		}
	}
#endif

	// MiB MAR2012_07 -  -2 means mouse was released before it got to the first charge. Changing this to do something.
	if (iNewAttack >= 0 || iNewAttack == -2)
	{
		//Start attack
		dbg("Initiate attack");
		SetBits(m_pOwner->m_StatusFlags, PLAYER_MOVE_ATTACKING);
		CallScriptEvent(CurrentAttack->CallbackName + "_start");

		CurrentAttack->tStart = CurrentAttack->tTrueStart = gpGlobals->time;
		CurrentAttack->fAttackLanded = false;
		CurrentAttack->fAttackReleased = false;
		CurrentAttack->fAttackThisFrame = false;
		CurrentAttack->fCanCancel = true;
		//pev->iuser1 = i;
		m_ReleaseAttack = false; //Start with the attack un-released (attack button still held)

		//Use up stamina
		m_pOwner->Stamina -= CurrentAttack->flEnergy;
		if (m_pOwner->Stamina < 0)
			m_pOwner->Stamina = 0;
		if (m_pPlayer)
			m_pPlayer->AddNoise(CurrentAttack->flNoise); //Make noise in world
		//Thothie FEB2008a
		//- fuck +special breaking me wrists, and dbl click lag - just let players charge their attacks as long as they hold the button
		//- downside: cant hold button for continuous attacks
		//m_TimeChargeStart = gpGlobals->time;
		if (CurrentAttack->flChargeAmt)
			m_TimeChargeStart = 0;

		// MiB MAR2012_07 - Reset the charge-start and set the attack to its real index.
		if (iNewAttack == -2)
		{
			m_TimeChargeStart = 0;
			iNewAttack = 0;
		}

#ifndef VALVE_DLL
		//Send the current attack to the server for syncronization
		//MiB JUN2010_19 - Don't need this..
		/*if( m_pPlayer )
		{
			m_pPlayer->m_ClientAttackHand = m_Hand;
			m_pPlayer->m_ClientAttackNum = iNewAttack;
		}*/
		//Print("ATTACKDEBUG: %f",m_pOwner->m_HP );

		SendAttackCmd(iNewAttack);
#endif

		//Check for ammo
		if (!UseAmmo(CurrentAttack->iAmmoDrain))
			CancelAttack();
		m_LastChargedAmt = 0; //Clear after I've checked for valid attack.  This way if I let go of charge early, it still waits for the attack to finish
	}

	enddbg;
	return true;
}

//Attack - Called every frame to continue an attack
void CGenericItem::Attack()
{
	startdbg;
	dbg("Begin");

	if (!m_pOwner || !CurrentAttack)
		return;

	dbg("Projectile checks");

	//Special stuff for projectiles... move to subroutine?
	bool fCanLandAttack = true;
	if (CurrentAttack->Type == ATT_CHARGE_THROW_PROJ && !CurrentAttack->fAttackReleased)
	{
		if (m_ReleaseAttack) //Let go this frame
		{
			//if( gpGlobals->time < CurrentAttack->tStart + CurrentAttack->tProjMinHold )
			//{   //Released too early
			//	CurrentAttack->tStart = gpGlobals->time - CurrentAttack->tDuration;
			//}
			//else
			CurrentAttack->tStart = gpGlobals->time;

			m_ReleaseAttack = false;
		}
		//Don't cancel while charging the projectile...
		else
		{
			CurrentAttack->fCanCancel = false;
			fCanLandAttack = false;
		}
	}
	else if (CurrentAttack->Type == ATT_STRIKE_HOLD && m_ReleaseAttack)
	{
		CancelAttack();
		SetBits(m_pOwner->pev->effects, EF_NOINTERP);
		return;
	}

	dbg("Duration checks");
	//Quit if the attack is done (it's duration has expired)
	if (CurrentAttack->tStart + CurrentAttack->tDuration <= gpGlobals->time &&
		CurrentAttack->tDuration >= 0 && CurrentAttack->fCanCancel)
	{

		CancelAttack();
		return;
	}

	//Quit if I can't land my attack now
	if (CurrentAttack->fAttackLanded)
		return; //Attack already landed, never attack twice

	if (!CurrentAttack->fAttackThisFrame)
	{
		if (!m_pPlayer)
			return; //Monsters MUST have fAttackThisFrame set to attack
		if (!fCanLandAttack || gpGlobals->time < CurrentAttack->tStart + CurrentAttack->tLandDelay)
			return;
	}

	//Attack has successfully been landed
	CurrentAttack->fAttackLanded = true;

	dbg("Call strike function");
	//Don't refer to CurrentAttack after calling StrikeLand, StrikeHold, etc.
	//because CancelAttack might get called, which nulls CurrentAttack.
	int Type = CurrentAttack->Type;
	if (Type == ATT_STRIKE_LAND)
		StrikeLand();
	else if (Type == ATT_STRIKE_HOLD)
		StrikeHold();
	else if (Type == ATT_CHARGE_THROW_PROJ)
		ChargeThrowProj();

	enddbg;
}
void CGenericItem::RegisterAttack()
{
	attackdata_t attData;
	ZeroMemory(&attData, sizeof(attackdata_t));
	attData.flRange = atof(GetFirstScriptVar("reg.attack.range"));
	attData.flDamage = atof(GetFirstScriptVar("reg.attack.dmg"));
	attData.f1DmgMulti = atof(GetFirstScriptVar("reg.attack.dmg.multi")); //Thothie OCT2007a

	//not needed, just make sure ammodrain is 0
	//attData.InfAmmo = GetFirstScriptVar("reg.attack.infammo"); //Thothie OCT2007a

	attData.sDamageType = GetFirstScriptVar("reg.attack.dmg.type");
	attData.flDamageRange = atof(GetFirstScriptVar("reg.attack.dmg.range"));
	attData.flDamageAOERange = atof(GetFirstScriptVar("reg.attack.aoe.range"));
	attData.flDamageAOEAttn = atof(GetFirstScriptVar("reg.attack.aoe.falloff"));
	attData.flEnergy = atof(GetFirstScriptVar("reg.attack.energydrain"));
	attData.flMPDrain = atof(GetFirstScriptVar("reg.attack.mpdrain"));
	attData.iAmmoDrain = !strcmp(GetFirstScriptVar("reg.attack.ammodrain"), "reg.attack.ammodrain") ? 1 : atoi(GetFirstScriptVar("reg.attack.ammodrain")); //default to 1 if not set
	attData.tDuration = atof(GetFirstScriptVar("reg.attack.delay.end"));
	attData.tLandDelay = atof(GetFirstScriptVar("reg.attack.delay.strike"));
	attData.flAccuracyDefault = atof(GetFirstScriptVar("reg.attack.hitchance"));
	attData.flNoise = atof(GetFirstScriptVar("reg.attack.noise"));
	attData.NoDamage = atoi(GetFirstScriptVar("reg.attack.dmg.ignore")) ? true : false;
	attData.NoAutoAim = atoi(GetFirstScriptVar("reg.attack.noautoaim")) ? true : false;
	attData.flChargeAmt = atof(GetFirstScriptVar("reg.attack.chargeamt")) / 100.0f;
	if (attData.flChargeAmt > 0)
		attData.flChargeAmt = GET_CHARGE_FROM_TIME(attData.flChargeAmt);
	attData.iPriority = atoi(GetFirstScriptVar("reg.attack.priority"));
	attData.CallbackName = GetFirstScriptVar("reg.attack.callback");
	attData.RequiredSkill = atoi(GetFirstScriptVar("reg.attack.reqskill"));
	/*msstring Temp = GetFirstScriptVar("reg.attack.stat"); //Legecy support
	if( Temp != "reg.attack.stat" )
	{
		int Stat = 0, Prop = 0;
		GetStatIndices( Temp, Stat, Prop );
		attData.StatBalance = Stat;
		attData.StatPower = Stat;
		attData.StatProf = Stat;
		attData.StatExp = Stat;
		attData.PropBalance = STATPROP_BALANCE;
		attData.PropPower = STATPROP_POWER;
		attData.PropProf = STATPROP_BALANCE;
		attData.PropExp = -1;	//Random
	}*/

	msstring Temp = GetFirstScriptVar("reg.attack.stat"); //Used to specify a default stat/prop
	if (Temp != "reg.attack.stat")
	{
		int Stat = 0, Prop = -1;
		GetStatIndices(Temp, Stat, Prop);
		bool BadProp = false;
		if (Prop < 0)
		{
			BadProp = true;
			Prop = 0; //Make the prop valid, because prof/balance/power only accept positive indexes
		}
		attData.StatBalance = Stat;
		attData.StatPower = Stat;
		attData.StatProf = Stat;
		attData.StatExp = Stat;
		attData.PropProf = !BadProp ? Prop : 0;
		attData.PropBalance = !BadProp ? Prop : 1;
		attData.PropPower = !BadProp ? Prop : 2;
		attData.PropExp = !BadProp ? Prop : -1; //If prop was invalid, use random (-1)
	}

	ReadStat("reg.attack.stat.prof", attData.StatProf, attData.PropProf);
	ReadStat("reg.attack.stat.balance", attData.StatBalance, attData.PropBalance);
	ReadStat("reg.attack.stat.power", attData.StatPower, attData.PropPower);
	ReadStat("reg.attack.stat.exp", attData.StatExp, attData.PropExp);

	msstring AttackType = GetFirstScriptVar("reg.attack.type");

	if (AttackType == "hold-strike")
	{
		//Strike, then hold... (shield, sword parry)
		attData.Type = ATT_STRIKE_HOLD;
		attData.tDuration = -1;
	}

	else if (AttackType == "charge-throw-projectile")
	{
		//Charge until the button's released
		attData.Type = ATT_CHARGE_THROW_PROJ;
		msstringlist HoldDelays;
		TokenizeString(GetFirstScriptVar("reg.attack.hold_min&max"), HoldDelays);

		if (HoldDelays.size() > 0)
			attData.tProjMinHold = atof(HoldDelays[0]);
		if (HoldDelays.size() > 1)
			attData.tMaxHold = atof(HoldDelays[1]);

		attData.sProjectileType = GetFirstScriptVar("reg.attack.projectile");

		msstringlist Stats;

		TokenizeString(GetFirstScriptVar("reg.attack.COF"), Stats);
		if (Stats.size() >= 2)
		{
			attData.flAccuracyDefault = atof(Stats[0]);
			attData.flAccBest = atof(Stats[1]);
		}
	}
	/*else if( AttackType == "charge-strike" )
	{
		//Must Charge to a percent before attacking
		attData.Type = ATT_CHARGE_STRIKE;
		//attData.tDuration = -1;			// <<-- indicates charge

		//Here, tMaxHold is the percent I must charge to (100%, 200%)
		attData.tMaxHold = atof( GetFirstScriptVar("reg.attack.chargeamt") );

	}*/
	else if (AttackType == "target")
	{
		//Same as strike-land, except don't auto-aim
		attData.Type = ATT_STRIKE_LAND;
		attData.NoAutoAim = true;
		attData.NoDamage = true;
	}
	//strike-land
	else
		attData.Type = ATT_STRIKE_LAND;

	attData.StartOffset = StringToVec(GetFirstScriptVar("reg.attack.ofs.startpos"));
	attData.AimOffset = StringToVec(GetFirstScriptVar("reg.attack.ofs.aimang"));

	TokenizeString(GetFirstScriptVar("reg.attack.keys"), attData.ComboKeys);

	m_Attacks.add(attData);
}

float CGenericItem::GetHighestAttackCharge()
{
	float HighestCharge = 0;

	for (int i = 0; i < m_Attacks.size(); i++)
	{
		attackdata_t &Attack = m_Attacks[i];

		if (Attack.flChargeAmt <= HighestCharge)
			continue;

		if (m_pOwner)
		{
			if (Attack.RequiredSkill) //Do I have the skill required to use this attack
			{
				int Stat = m_pOwner->GetSkillStat(Attack.StatProf, Attack.PropProf);
				//Thothie SEP2007 - reduce accuracy and inc stanima drain instead of making unswingable
				if (Stat < Attack.RequiredSkill && i > 0)
					continue;
			}
		}

		HighestCharge = Attack.flChargeAmt;
	}

	return HighestCharge;
}

void CGenericItem::CancelAttack()
{
	if (CurrentAttack)
		CallScriptEvent(CurrentAttack->CallbackName + "_end");

	if (DrinkData)
		DrinkCancel();
	if (m_pOwner)
	{
		ClearBits(m_pOwner->m_StatusFlags, PLAYER_MOVE_ATTACKING);
#ifdef VALVE_DLL
		m_pOwner->StopWalking();
#endif
	}

	CurrentAttack = NULL;
	m_ReleaseAttack = false;

	CallScriptEvent("game_attack_cancel");
}
void CGenericItem::ItemPostFrame()
{
	startdbg;

	dbg("Begin");

	if (!m_pPlayer)
		return;

	/*dbg( "AttackButtonDown" );
	if( FBitSet(m_pPlayer->pbs.ButtonsDown,IN_ATTACK) )
		AttackButtonDown( );
	else AttackButtonUp( );*/

	dbg("Attack2ButtonDown");
	if (FBitSet(m_pPlayer->pbs.ButtonsDown, IN_ATTACK2))
		Attack2ButtonDown(); // +attack2
	else
		Attack2ButtonUp();

	if (m_Location == ITEMPOS_HANDS) //Must be in hand to get activated
	{
		int iActivateButton = ActivateButton();

		if (FBitSet(m_pPlayer->m_afButtonPressed, iActivateButton)) //Button pressed this frame
			CallScriptEvent("game_playeractivate");

		if (FBitSet(m_pPlayer->pbs.ButtonsDown, iActivateButton))
			ActivateButtonDown(); //Button is currently down
		else
			ActivateButtonUp(); //Button is currently up
	}

	dbg("AllButtonsReleased");
	if (!FBitSet(m_pPlayer->pbs.ButtonsDown, IN_ATTACK | IN_ATTACK2))
		AllButtonsReleased(); // no fire buttons down

	dbg("Idle");
	if (ShouldIdle())
		Idle();

#ifndef VALVE_DLL
	//Handle drink stuff - client only
	if (DrinkData)
		Drink();

	//tProjMinHold is used to force a delay on auto-parrys
	if (CurrentAttack && CurrentAttack->Type == ATT_STRIKE_HOLD && !ActivatedByOwner())
	{
		if (gpGlobals->time > CurrentAttack->tStart + CurrentAttack->tLandDelay &&
			gpGlobals->time > CurrentAttack->tStart + CurrentAttack->tProjMinHold)
		{
			SendCancelAttackCmd();
			m_ReleaseAttack = true;
			CurrentAttack->tProjMinHold = 0;
		}
	}
#endif

	DrinkThink(); //Run on client & server

#ifdef VALVE_DLL
	dbg("Think");
	if (MSProperties() & ITEM_GENERIC)
		Think();
#endif
	dbg("End");

	enddbg;
}
// Attack variations
void CGenericItem::StrikeLand()
{
#ifdef VALVE_DLL

	SetDebugProgress(ItemThinkProgress, "CGenericItem::StrikeLand - Begin");
	//m_pOwner->WorldVolume = LOUD_GUN_VOLUME;

	UTIL_MakeVectors(m_pOwner->pev->v_angle);

	int SideMultiplier = 1;
	if (m_pOwner->m_CurrentHand == LEFT_HAND)
		SideMultiplier = -1;

	Vector vecSrc = m_pOwner->EyePosition() + gpGlobals->v_forward * CurrentAttack->StartOffset.y + gpGlobals->v_right * CurrentAttack->StartOffset.x * SideMultiplier + gpGlobals->v_up * CurrentAttack->StartOffset.z;

	Vector vForward;
	UTIL_MakeVectorsPrivate(m_pOwner->pev->v_angle + CurrentAttack->AimOffset, vForward, NULL, NULL);

	Vector vecEnd = vecSrc + vForward * CurrentAttack->flRange;

	float flDmgFraction = m_pOwner->GetSkillStat(CurrentAttack->StatBalance, CurrentAttack->PropBalance) / STATPROP_MAX_VALUE;
	flDmgFraction = max(flDmgFraction, 0);

	//Thothie SEP2007 - attempting to make level limit work by decreasing accuracy
	bool thoth_unskilled = false;
	if (m_pOwner->GetSkillStat(CurrentAttack->StatProf, CurrentAttack->PropProf) < CurrentAttack->RequiredSkill)
		thoth_unskilled = true;

	float flMissPercentage = 0.0;
	flMissPercentage = (100 - CurrentAttack->flAccuracyDefault);
	float flHitPercentage = CurrentAttack->flAccuracyDefault + (flMissPercentage * flDmgFraction);
	if (thoth_unskilled)
		flHitPercentage = flHitPercentage * 0.25; //thothie - seems to work
	if (m_pOwner->m_HITMulti > 0)
		flHitPercentage *= m_pOwner->m_HITMulti; //FEB2009_18

	flDmgFraction = m_pOwner->GetSkillStat(CurrentAttack->StatPower, CurrentAttack->PropPower) / STATPROP_MAX_VALUE;
	flDmgFraction = max(flDmgFraction, 0.001f);

	//	flDmgFraction = max(mSStat( CurrentAttack->sAttackStat, STATPROP_POWER ) / STATPROP_MAX_VALUE,0);
	//	ALERT( at_console, "Use stat: %i  Value: %i %i %i\n", GetStatByName( (char*)STRING(CurrentAttack->sAttackStat) ), mSStat( CurrentAttack->sAttackStat, STATPROP_SPEED ), mSStat( CurrentAttack->sAttackStat, STATPROP_BALANCE ), mSStat( CurrentAttack->sAttackStat, STATPROP_POWER ) );
	float flDamage = CurrentAttack->flDamage + RANDOM_LONG(-CurrentAttack->flDamageRange, CurrentAttack->flDamageRange);
	flDamage *= flDmgFraction;
	if (thoth_unskilled)
		flDamage *= 0.5; //this might be working

	//Print( "In damage: %f\nLast charged: %f\n", flDamage, m_LastChargedAmt );
	if (m_LastChargedAmt > 0 && m_LastChargedAmt < 1)
	{
		Print("Out damage: %f\n", flDamage *= m_LastChargedAmt);
		flDamage *= m_LastChargedAmt;
	}

	SetDebugProgress(ItemThinkProgress, "CGenericItem::StrikeLand - Call DoDamage");
	int bitsDamage = DMG_CLUB;
	if (!m_pPlayer)
		bitsDamage |= DMG_SIMPLEBBOX;
	if (CurrentAttack->flDamageAOERange)
		SetBits(bitsDamage, DMG_AOE);
	if (CurrentAttack->NoDamage)
		SetBits(bitsDamage, DMG_NONE);

	damage_t Damage;
	clrmem(Damage);

	Damage.pInflictor = this;
	Damage.ItemCallBackPrefix = &CurrentAttack->CallbackName;
	Damage.pAttacker = m_pOwner;
	Damage.vecSrc = vecSrc;
	Damage.vecEnd = vecEnd;
	Damage.flDamage = flDamage;
	Damage.iDamageType = bitsDamage;
	Damage.flHitPercentage = flHitPercentage;
	Damage.sDamageType = CurrentAttack->sDamageType;
	Damage.flRange = CurrentAttack->NoAutoAim ? 0 : CurrentAttack->flRange; //Normal attacks have an 'Autoaim' AOE range to make hitting easier
																			//(only hits the closest ent within range)
	Damage.flAOERange = CurrentAttack->flDamageAOERange;
	Damage.flAOEAttn = CurrentAttack->flDamageAOEAttn;

	Damage.ExpStat = CurrentAttack->StatExp; // MiB JUN2010_19 - Fixing exp for the off-hand.
	Damage.ExpProp = CurrentAttack->PropExp; // Exp goes to the active weapon if it doesn't know where to go
	Damage.ExpUseProps = true; // Make sure it knows where to go!

	hitent_list Hits;
	DoDamage(Damage, Hits);
	CBaseEntity *pHit = Hits.ClosestHit();

	//In case I kill myself
	if (!CurrentAttack)
		return;

	SetDebugProgress(ItemThinkProgress, "CGenericItem::StrikeLand - Set script variables");

	//Reset hit/miss variables
	//SetScriptVar( "game_lastattack.hitwall", 0 );
	//SetScriptVar( "game_lastattack.hitmonster", 0 );

	//Handle the script callback
	//if( pHit && pHit->MyMonsterPointer() )
	//	SetScriptVar( "game_lastattack.hitnpc", 1 );

	//else SetScriptVar( "game_lastattack.hitwall", 1 );
	//SetScriptVar( "game_lastattack.endpos", VecToString(Damage.outTraceResult.vecEndPos) );

	SetDebugProgress(ItemThinkProgress, "CGenericItem::StrikeLand - Script callback: land event");
	msstringlist Parameters;
	Parameters.add(pHit ? (pHit->IsMSMonster() ? "npc" : "world") : "none");
	Parameters.add(VecToString(Damage.outTraceResult.vecEndPos));
	Parameters.add(pHit ? EntToString(pHit) : "�NONE�");
	Parameters.add(Damage.AttackHit ? "1" : "0");
	CallScriptEvent(CurrentAttack->CallbackName + "_strike", &Parameters);

	SetDebugProgress(ItemThinkProgress, "CGenericItem::StrikeLand - Reset script variables");
#endif
}
void CGenericItem::StrikeHold()
{
	CallScriptEvent(CurrentAttack->CallbackName + "_hold");
}

//Uses ammo.  Projectiles or MP
bool CGenericItem::UseAmmo(int iAmt)
{
	startdbg;
	dbg("Begin");
	if (!m_pOwner || !CurrentAttack)
		return false;

	//Attack requires mana... do we have enoguh?
	if (CurrentAttack->flMPDrain && m_pOwner->m_MP < CurrentAttack->flMPDrain && !FBitSet(m_pOwner->pev->flags, FL_GODMODE))
	{
		if (m_pPlayer)
		{
#ifndef VALVE_DLL
			m_pPlayer->SendEventMsg(HUDEVENT_UNABLE, "You don't have enough MP");
#endif
			m_pPlayer->BlockButton(IN_ATTACK);
		}
		return false;
	}

	bool RequiresAmmo = false; //Whether weapon requires a player to carry ammo for it

	if (CurrentAttack->Type == ATT_CHARGE_THROW_PROJ && iAmt)
		RequiresAmmo = true;

	CGenericItem *pProjInPack = NULL, *pPack = NULL;

	//Skip ammo check if the attack doesn't require ammo
	if (RequiresAmmo)
	{

		//Monsters have unlimited ammo for now
		if (!m_pPlayer)
		{
			CurrentAttack->sProjectile = 0;
			CGenericItem *pGlobalItem = CGenericItemMgr::GetGlobalGenericItemByName("proj_arrow_wooden");
			if (pGlobalItem)
				CurrentAttack->sProjectile = pGlobalItem->ItemName;
		}
		else
		{
			//MiB JUN2010_17 - Oh, the massive changes. Allow players to choose arrows.
			bool PreSelectedArrow = false;
			if (m_pPlayer->m_ChosenArrow)
			{

				CGenericItem *pArrow = m_pPlayer->m_ChosenArrow;
				bool GENERIC = msstring(pArrow->m_Name).ends_with("_generic");
				//Make sure I have the right type of arrow/bolt!
				bool REQUIRE_ARROW = msstring(CurrentAttack->sProjectileType).contains("arrow");
				bool HAVE_ARROW = msstring(pArrow->m_Name).contains("arrow");

				if (((REQUIRE_ARROW && HAVE_ARROW) || (!REQUIRE_ARROW && !HAVE_ARROW)) &&
					(GENERIC || ((pArrow->iQuantity || iAmt > 1) && pArrow->iQuantity >= iAmt)))
				{
					CurrentAttack->sProjectile = pArrow->ItemName;
					CurrentAttack->iLoadedAmmo = iAmt;

					if (!GENERIC)
					{
						pArrow->iQuantity -= iAmt;
						if (pArrow->iQuantity <= 0)
						{
#ifdef VALVE_DLL
							if (pPack)
								pPack->Container_RemoveItem(pArrow);
							else
								m_pOwner->RemoveItem(pArrow);

							pArrow->SUB_Remove();
							m_pPlayer->m_ChosenArrow = NULL;
#else
							m_pPlayer->SendEventMsg(HUDEVENT_UNABLE, msstring("This is your last ") + pArrow->m_DisplayName);
							m_pPlayer->m_ChosenArrow = NULL;
#endif
						}
					}

					PreSelectedArrow = true;
				}
			}

			if (!PreSelectedArrow)
			{
				//Find iAmt projectiles from any worn pack
				pProjInPack = m_pOwner->GetItem(CurrentAttack->sProjectileType, &pPack);

				if (pProjInPack)
				{
					//Successfully found ammo
					//Decrement ammo by iAmt
					if (FBitSet(pProjInPack->MSProperties(), ITEM_GROUPABLE))
					{
						if ((pProjInPack->iQuantity || iAmt > 1) && pProjInPack->iQuantity >= iAmt)
						{

							CurrentAttack->sProjectile = pProjInPack->ItemName;
							CurrentAttack->iLoadedAmmo = iAmt;

							pProjInPack->iQuantity -= iAmt;
						}
					}

#ifdef VALVE_DLL
					//Remove the item after it has been depleted
					if (pProjInPack->iQuantity <= 0)
					{
						if (pPack)
							pPack->Container_RemoveItem(pProjInPack);
						else
							m_pOwner->RemoveItem(pProjInPack);

						pProjInPack->SUB_Remove();
					}
#endif
				}
				else
				{
					//Player not carrying any of the required ammo
					if (!stricmp(CurrentAttack->sProjectileType, "arrow"))
					{
						//New! Give free 'blunt' arrows
						CurrentAttack->sProjectile = "";
						CGenericItem *pGlobalItem = CGenericItemMgr::GetGlobalGenericItemByName("proj_arrow_generic");
						if (pGlobalItem)
							CurrentAttack->sProjectile = pGlobalItem->ItemName;

						CurrentAttack->iLoadedAmmo = iAmt;
					}
					else if (!stricmp(CurrentAttack->sProjectileType, "bolt"))
					{
						CurrentAttack->sProjectile = "";
						CGenericItem *pGlobalItem = CGenericItemMgr::GetGlobalGenericItemByName("proj_bolt_generic");
						if (pGlobalItem)
							CurrentAttack->sProjectile = pGlobalItem->ItemName;
						CurrentAttack->iLoadedAmmo = iAmt;
					}
					else
					{
						//Player not carrying any ammo and no free ammo is specified for this ammo type... can't attack
						if (m_pPlayer)
						{
#ifndef VALVE_DLL
							CallScriptEvent(CurrentAttack->CallbackName + "_noammo");
							m_pPlayer->SendEventMsg(HUDEVENT_UNABLE, msstring("You don't have any ") + CurrentAttack->sProjectileType.c_str());
#endif
							m_pPlayer->BlockButton(IN_ATTACK);
						}
						return false;
					}
				}
			}
		}
	}
	else
	{
		//Attack spawns ammo - Player doesn't carry it
		CGenericItem *pGlobalItem = CGenericItemMgr::GetGlobalGenericItemByName(CurrentAttack->sProjectileType);
		if (pGlobalItem)
			CurrentAttack->sProjectile = pGlobalItem->ItemName;
	}

	//Drain the required mana
	if (CurrentAttack->flMPDrain)
		m_pOwner->Give(GIVE_MP, -CurrentAttack->flMPDrain);

	enddbg;
	return true;
}

void CGenericItem::ChargeThrowProj()
{
	if (!CurrentAttack)
		return;

#ifdef VALVE_DLL

	CGenericItem *pProjectile = NewGenericItem(CurrentAttack->sProjectile);
	if (pProjectile)
	{
		Vector vForward, vRight, vUp;
		Vector vAngle = m_pOwner->pev->v_angle,
			   vOrigin = m_pOwner->EyePosition();

		float flTimeHeld = gpGlobals->time - CurrentAttack->tTrueStart;
		float flTimeHeldAdjusted = max(min(flTimeHeld, CurrentAttack->tMaxHold), CurrentAttack->tProjMinHold);
		flTimeHeldAdjusted = CurrentAttack->tMaxHold ? (flTimeHeldAdjusted / CurrentAttack->tMaxHold) : 0; //[0.0 to 1.0]

		//Shoot more accurately for higher skill
		float flAccFraction = m_pOwner->GetSkillStat(CurrentAttack->StatBalance, CurrentAttack->PropBalance) / STATPROP_MAX_VALUE;
		flAccFraction = 1 - min(max(flAccFraction, 0.0f), 1.0f);

		//Shoot more accurately for drawing the bow back longer
		float LoweredSpreadDeg = CurrentAttack->flAccuracyDefault - (CurrentAttack->flAccuracyDefault - CurrentAttack->flAccBest) * flTimeHeldAdjusted;

		float Spread = max(LoweredSpreadDeg, 0) * flAccFraction * RANDOM_FLOAT(-1.0f, 1.0f); //Factor skill and randomness into the Spread
		float VeerAng = RANDOM_FLOAT(0.0f, M_PI * 2);										 //Determine an angle to veer off 0-360
		vAngle.x += cosf(VeerAng) * Spread;													 //Veer off at the angle, multiplied by the spread
		vAngle.y += sinf(VeerAng) * Spread;

		if (m_pPlayer)
			vAngle += CurrentAttack->AimOffset;

		UTIL_MakeVectorsPrivate(vAngle, vForward, vRight, vUp);

		//if( m_pPlayer )
		//	vOrigin = vOrigin + vRight * CurrentAttack->vAlignBase.x + Vector(0,0,1) * CurrentAttack->vAlignBase.y;

		float flRange = CurrentAttack->flRange * flTimeHeldAdjusted;
		Vector vStartPos = vOrigin + vForward * 10 + vForward * CurrentAttack->StartOffset.y + vRight * CurrentAttack->StartOffset.x * (!m_Hand ? -1 : 1) + vUp * CurrentAttack->StartOffset.z;

		Vector vTemp = vForward * flRange;
		pProjectile->TossProjectile(this, vStartPos, vTemp);
	}
#endif

	CurrentAttack->fCanCancel = true;
	CallScriptEvent(CurrentAttack->CallbackName + "_toss");
}

bool CGenericItem::Attack_IsCharging()
{
	//old
	//if( CurrentAttack && (CurrentAttack->Type == ATT_CHARGE_THROW_PROJ && !CurrentAttack->fCanCancel) )
	if (CurrentAttack && (CurrentAttack->Type == ATT_CHARGE_THROW_PROJ && !CurrentAttack->fCanCancel))
	{
		return true;
	}
	return m_TimeChargeStart ? true : false;
}
float CGenericItem::Attack_Charge()
{
	float ChargeDuration = 0;

	//For ranged attacks, the charge indicates your projectile force and accuracy
	//The range is from 0 to 1
	if (CurrentAttack && CurrentAttack->Type == ATT_CHARGE_THROW_PROJ)
	{
		ChargeDuration = gpGlobals->time - (CurrentAttack->tStart + CurrentAttack->tProjMinHold);			   //Duration
		ChargeDuration = ChargeDuration / ((CurrentAttack->tMaxHold - CurrentAttack->tProjMinHold) + 0.0001f); //Ratio of max hold
		ChargeDuration = max(ChargeDuration, 0);															   //Cap ratio at 0
		ChargeDuration = min(ChargeDuration, 1.0f);															   //Cap ratio at 1 second
		return ChargeDuration;
	}

	//For melee attacks, the charge indicates power-up levels for special attacks
	if (!m_TimeChargeStart)
		return 0;

	ChargeDuration = gpGlobals->time - m_TimeChargeStart;
	float Charge = GET_CHARGE_FROM_TIME(ChargeDuration);

	float HighestCharge = GetHighestAttackCharge();

	Charge = max(Charge, 0);			 //Cap ratio at 0
	Charge = min(Charge, HighestCharge); //Cap ratio at highest charge found

	return Charge;
}

#ifdef VALVE_DLL
void CGenericItem::ActivateShield(char *pszShieldAreaModel)
{
	/*CShieldArea *pShield = GetClassPtr( (CShieldArea *)NULL );
	pShield->pev->owner = edict();
	pShield->pev->model = ALLOC_STRING( pszShieldAreaModel );
	pShield->Spawn( );*/
}

//Give a chance to modify Damage
void CGenericItem::OwnerTakeDamage(damage_t &Damage)
{
	//Handle armor
	if (IsArmor() && IsWorn())
		Damage.flDamage = Armor_Protect(Damage);

	//Handle shields, anything else that absorbs damage
	static msstringlist Params;
	Params.clearitems();
	Params.add(Damage.pAttacker ? EntToString(Damage.pAttacker) : "none");
	Params.add(Damage.pInflictor ? EntToString(Damage.pInflictor) : "none");
	Params.add(FloatToString(Damage.flDamage));
	Params.add(Damage.sDamageType);
	m_CurrentDamage = &Damage; //m_CurrentDamage used by script commands to change damage
	CallScriptEvent("game_takedamage", &Params);
	m_CurrentDamage = NULL;
}

//
//			DoDamage - Generic function for doing damage
//          ��������

/*CBaseEntity *DoDamage( entvars_t *pInflictor, entvars_t *pAttacker, Vector &vecSrc,
					  Vector &vecEnd, float flDamage, int iDamageType,
					  float flHitPercentage, TraceResult *outTraceResult )
{
	damage_t Damage;

	Damage.pevInflictor = pInflictor;
	Damage.pevAttacker = pAttacker;
	Damage.vecSrc = vecSrc;
	Damage.vecEnd = vecEnd;
	Damage.flDamage = flDamage;
	Damage.iDamageType = iDamageType;
	Damage.flHitPercentage = flHitPercentage;
	if( outTraceResult ) Damage.outTraceResult = *outTraceResult;

	return DoDamage( Damage );
}*/

/*CBaseEntity *DoDamage( damage_t &Damage )
{
	startdbg;
	dbg( "Begin" );

	//Old DoDamage parameters
	entvars_t *pInflictor = Damage.pevInflictor;
	entvars_t *pAttacker = Damage.pevAttacker;
	Vector &vecSrc = Damage.vecSrc;
	Vector &vecEnd = Damage.vecEnd;
	float &flDamage = Damage.flDamage;
	float flHitPercentage = Damage.flHitPercentage;
	//-----------------------

	CBaseEntity *pEntityAttacker = MSInstance( pAttacker );
	CBaseEntity *pEntityInflictor = MSInstance( pInflictor );

	char sz[256] = "";

	if( !pEntityAttacker )
		return NULL;  //This happens with arrows sometimes... for some reason....

	CMSMonster *pAttMonster = pEntityAttacker->IsMSMonster() ? (CMSMonster *)pEntityAttacker : NULL;		//Attacking Monster	(Can be NULL)
	CBasePlayer *pPlayerAttacker = pEntityAttacker->IsPlayer() ? (CBasePlayer *)pEntityAttacker : NULL;		//Attacking Player	(Can be NULL)

	//This might be a summon, or other effect that is doing damage for the player...
	if( pAttMonster && !pPlayerAttacker )
	{	//if so, assign the player pointer correctly
		CBaseEntity *pExpOwner = pAttMonster->RetrieveEntity( ENT_EXPOWNER );
		if( pExpOwner && pExpOwner->IsPlayer() )
			pPlayerAttacker = (CBasePlayer *)pExpOwner;
	}

	if( FBitSet(Damage.iDamageType,DMG_DIRECT) && Damage.pDirectDmgTarget )
	{
		Damage.outTraceResult.flFraction = 0.0f;
		Damage.outTraceResult.pHit = Damage.pDirectDmgTarget->edict();
	}
	else if( Damage.flAOERange > 0 )
	{
		Damage.outTraceResult.flFraction = 1.0f;
	}
	else {
		int trflags = MSTRACE_SOLIDSHIELDS;

		if( FBitSet(Damage.iDamageType,DMG_SIMPLEBBOX) ) gpGlobals->trace_flags |= FTRACE_SIMPLEBOX;
		else SetBits( trflags, MSTRACE_LARGEHITBOXES );

		if( pAttacker ) SetBits( trflags, MSTRACE_HITCORPSES );
		MSTraceLine( vecSrc, vecEnd, dont_ignore_monsters, ENT(pAttacker), Damage.outTraceResult, trflags );
	}

	//BeamEffect( vecSrc, vecEnd, iBeam, 0, 0, 100, 10, 0, 255, 255, 255, 255, 20 );

	//SetDebugProgress( ItemThinkProgress, "DoDamage - Check special power damage (rogue backstab)" );
	//bActualHit: Did the combination of luck&skill produce a hit?
	//bool fDidHit = FALSE, fHitWorld = TRUE, bActualHit = FALSE,
	//	fReportHit = FALSE, fDodged = FALSE;

	SetDebugProgress( ItemThinkProgress, "DoDamage - Check hit" );
	bool fReportHit = false;
	Damage.AttackHit = false;

	CBaseEntity *pEntity = NULL;
	CMSMonster *pVictim = NULL;

	if( Damage.outTraceResult.flFraction < 1.0 )
	{
		// hit
		//fDidHit = TRUE;

		pEntity = CBaseEntity::Instance(Damage.outTraceResult.pHit);

		SetDebugProgress( ItemThinkProgress, "DoDamage - Entity hit" );
		int iAccuracyRoll = 0;
		if( pEntity )
		{
			//Hit an entity
			//Check if I can damage it
			Damage.AttackHit = pEntityAttacker->CanDamage( pEntity );
			fReportHit = false;

			if( pAttMonster && Damage.AttackHit )
			{
				//Hit a monster...
				fReportHit = true;

				pVictim = pEntity->IsMSMonster() ? (CMSMonster *)pEntity : NULL;

				if( pVictim && pVictim->IsAlive() )
				{
					//If player is attacking himself, don't report
					if( pPlayerAttacker == pVictim )
						fReportHit = false;

					//Check if your horrid skill or bad luck made you miss:
					iAccuracyRoll = RANDOM_LONG( 0, 99 );
					if( iAccuracyRoll > flHitPercentage ) { Damage.AttackHit = false; Damage.AttackHit = false; }

					if( Damage.AttackHit ) //Hit Monster
					{
						if( pPlayerAttacker &&				//Only if player is attacking...
							!pVictim->IsPlayer( ) )			//Only when attacking monsters...
						{
							//Advance your skill based on monster difficulty and num of swings

							int ExpStat = -1, ExpProp = -1;//Stat that increases from this attack

							if( pPlayerAttacker->ActiveItem( ) )
							{
								CGenericItem *pItem = pPlayerAttacker->ActiveItem( );
								if( pItem->CurrentAttack )
								{
									ExpStat = pItem->CurrentAttack->StatExp;
									ExpProp = pItem->CurrentAttack->PropExp;
								}
							}

//							else if( pPlayer->Hand[pPlayer->iCurrentHand] )
//							{
//								CGenericItem *pItem = (CGenericItem *)pPlayer->Hand[pPlayer->iCurrentHand];
//								if( pItem->CurrentAttack )
//								{
//									AttackStat = GetStatByName( STRING(pItem->CurrentAttack->sAttackStat) );
//									//I missed the monster, so learn about accuracy
//									if( !bActualHit )
//										pPlayer->LearnSkill( AttackStat, STATPROP_BALANCE, pMonster->m_SkillLevel );
//									//I hit the monster, so learn about damage
//									else
//										pPlayer->LearnSkill( AttackStat, STATPROP_POWER, pMonster->m_SkillLevel );
//									//Always learn about proficiency
//									pPlayer->LearnSkill( AttackStat, STATPROP_SKILL, pMonster->m_SkillLevel * max(pItem->CurrentAttack->iPriority,0.5) );
//
//								}
//							}
							//Keep track of damage
							if( ExpStat > -1 )
							{
								float flActualDamage = max(min(flDamage,pVictim->m_HP),0);

								if( ExpProp < 0 )	//If ExpProp is negative, give the exp to a random property within ExpStat
								{
									ExpProp = 0;	//Default 0, in case the ExpStat is invalid
									CStat *pStat = pPlayerAttacker->FindStat( ExpStat );
									if( pStat ) ExpProp = RANDOM_LONG( 0, pStat->m_SubStats.size()-1 );
								}

								pVictim->m_PlayerDamage[(pPlayerAttacker->entindex()-1)]->dmg[ExpStat][ExpProp] += flActualDamage;
							}
						}
					}
				}
				else
				{
					// Always hit world entities or dead bodies

					Damage.AttackHit = true;
					fReportHit = false;
				}

			}

			SetDebugProgress( ItemThinkProgress, "DoDamage - Check if damage is dealt" );
			bool fDodged = false;
			if( Damage.AttackHit )
			{
				//Hit sounds are now played from TraceAttack
				ClearMultiDamage( );
				Damage.AccuracyRoll = (flHitPercentage - iAccuracyRoll);

				if( pEntityInflictor ) pEntityInflictor->StoreEntity( pEntity, ENT_LASTSTRUCKBYME );
				if( pAttMonster )
				{
					pAttMonster->StoreEntity( pEntity, ENT_LASTSTRUCKBYME );
					pAttMonster->CallScriptEvent( "game_damaged_other" );

					//Script decided to change or cancel damage from within game_damaged_other
					if( pAttMonster->m_ReturnData.len( ) )
					{
						//Each script either sets a ratio of damage to be dealt or cancels the damage
						msstringlist DamageRatios;
						TokenizeString( pAttMonster->m_ReturnData, DamageRatios );
						 for (int i = 0; i < DamageRatios.size(); i++)
						{
							if( DamageRatios[i] == "canceldamage" )
							{
								Damage.AttackHit = false; fReportHit = false; pEntity = NULL;
								goto EndDamage;  //Script canceled, skip the rest
							}

							Damage.flDamage *= atof( DamageRatios[i] );
						}
					}
				}

				if( Damage.AttackHit )
				{
					//Non-ranged attack
					if( !pEntity->IsMSMonster( ) )
						flDamage = pEntity->TraceAttack( pInflictor, pAttacker, flDamage, gpGlobals->v_forward, &Damage.outTraceResult, Damage.iDamageType, Damage.AccuracyRoll );
					else
						flDamage = ((CMSMonster *)pEntity)->TraceAttack( Damage );

					SetDebugProgress( ItemThinkProgress, "DoDamage - Apply damage" );
					if( flDamage > 0 )  // flDamage < 0 means monster dodged it
						ApplyMultiDamage( pInflictor, pAttacker );
					else if( !flDamage && !CBaseEntity::Instance(pAttacker)->IsPlayer() )
						//If monsters do 0 damage, just consider it not hitting
						//This also prevents orc archers arrows from sticking into
						//other orcs
						goto EndDamage;
					else if( flDamage == -1 )
					{
						ClearMultiDamage( );
						fDodged = true;
						Damage.AttackHit = false;
					}
				}
			}

			if( pVictim ) pVictim->Attacked( CBaseEntity::Instance( pAttacker ), Damage );

			SetDebugProgress( ItemThinkProgress, "DoDamage - Report damage" );
			if( fReportHit )
			{
				char szStats[32], szDamage[32], szHitMiss[32];
				 strncpy(szDamage,  Damage.AttackHit ? UTIL_VarArgs( " %.1f damage.",  flDamage ) : "", sizeof(szDamage) );
				 strncpy(szHitMiss,  Damage.AttackHit ? "HIT!" : (fDodged ? "PARRIED!" : "MISS!"), sizeof(szHitMiss) );
				 _snprintf(szStats, sizeof(szStats),  "(%i/%i)",  (100-iAccuracyRoll),  int(100-flHitPercentage) );

				if( pPlayerAttacker )
				{
					 _snprintf(sz, sizeof(sz),  "You attack %s. %s %s%s",  						pEntity->DisplayName(),
						szStats, szHitMiss, szDamage );
					pPlayerAttacker->SendEventMsg( HUDEVENT_ATTACK, sz );
				}
				if( pVictim && pVictim->IsPlayer() && pEntityAttacker && pVictim != pEntityAttacker )
				{
					CBasePlayer *pPlayer = (CBasePlayer *)pEntity;
					 _snprintf(sz, sizeof(sz),  "%s attacks you. %s %s%s",  						pEntityAttacker->DisplayName(),
						szStats, szHitMiss, szDamage );
					pPlayer->SendEventMsg( HUDEVENT_ATTACKED, sz );
				}
			}

			SetDebugProgress( ItemThinkProgress, "DoDamage - Countereffect" );
			pEntity->CounterEffect( pEntityInflictor, 0, (void *)&flDamage );
			//if( fHitWorld )
			//{
			//	if( pEntity->MSProperties()&ITEM_SHIELD )
			//		pEntity->CounterEffect( MSInstance(ENT(pInflictor)), ITEM_SHIELD, (void *)&flDamage );
			//}

			// delay the decal a bit
			DecalGunshot( &Damage.outTraceResult, BULLET_PLAYER_CROWBAR );

			if( pEntityAttacker )
			{
				float Range = 128 + flDamage * 90;	//Min 128 units, and add 90 units for each dmg pt
				CSoundEnt::InsertSound( pEntityAttacker, "combat", Damage.outTraceResult.vecEndPos, Range, 0.3f, 0 );
			}
		}
	}

EndDamage:

	//Set some script variables
	dbg( "Set post damage variables" );
	if( pAttMonster )
	{
		Vector &EndPos = Damage.AttackHit ? Damage.outTraceResult.vecEndPos : Damage.vecEnd;

		static msstringlist Parameters;
		Parameters.clearitems( );
		Parameters.add( Damage.AttackHit ? "1" : "0" );
		Parameters.add( pEntity ? EntToString(pEntity) : "none" );
		Parameters.add( VecToString(Damage.vecSrc) );
		Parameters.add( VecToString(EndPos) );
		pAttMonster->CallScriptEvent( "game_dodamage", &Parameters );
	}


	//Ranged attack... Find all valid targets and call non-ranged DoDamage on them
	CBaseEntity *pClosestHit = pEntity;
	dbg( "Do radius damages" );
	if( Damage.flAOERange )
	{
		CBaseEntity *pTarget = NULL;
		while ((pTarget = UTIL_FindEntityInSphere( pTarget, Damage.vecSrc, Damage.flAOERange )) != NULL)
		{
			if( pTarget->pev->takedamage == DAMAGE_NO ||		//Don't hurt world objects, godmode entities, or the entity I originally struck
				FBitSet( pTarget->pev->	flags, FL_GODMODE ) ||
				!pTarget->IsAlive() ||
				pTarget == pEntity ||
				pTarget->pev == Damage.pevInflictor ||
				!pTarget->pev->model )	//Special thing for the firewall spell
				continue;

			if( pTarget->pev == Damage.pevAttacker && !FBitSet(Damage.iDamageType,DMG_REFLECTIVE ) )
				continue;	//Only hit the owner if damage is reflective

			TraceResult	tr;
			dbg( "Do radius Traceline" );
			UTIL_TraceLine ( Damage.vecSrc, pTarget->Center(), ignore_monsters, ENT(Damage.pevInflictor), &tr );

			if( tr.flFraction < 1.0f )
				continue;

			float dist = (vecSrc - tr.vecEndPos).Length();
			float DmgRatio = 1.0f - (dist / Damage.flAOERange);
			if( DmgRatio <= 0 )
				continue;

			damage_t RadiusDmg = Damage;
			RadiusDmg.vecEnd = pTarget->Center();
			RadiusDmg.flAOERange = RadiusDmg.flAOEAttn = 0.0f;		//Ensure it doesn't don't do ranged damage again - would cause infinite loop
			RadiusDmg.flDamage *= pow(DmgRatio, Damage.flAOEAttn);  //AOE Damage = Base Damage * [(1-[distance to target/maxrange]) ^ attenuation]
			SetBits( RadiusDmg.iDamageType, DMG_DIRECT );			//Just do direct damage to this entity.  We've already check if its a valid ent
			RadiusDmg.pDirectDmgTarget = pTarget;
			CBaseEntity *pHit = DoDamage( RadiusDmg );
			if( pHit && (!pClosestHit || dist < (pClosestHit->Center() - Damage.vecSrc).Length() ) )	//If this is the closest thing I hit, return it later
				pClosestHit = pHit;
		}
	}

	return pClosestHit;
	enddbg( "DoDamage()" );
	return NULL;
}*/
CBaseEntity *DoDamage(damage_t &Damage, CBaseEntity *pTarget);

void DoDamage(damage_t &Damage, hitent_list &Hits)
{
	startdbg;
	//Find all valid targets
	//If AOE, hit all targets in area
	//If non-AOE, just hit the closest one in front of me
	dbg("Find Targets");
	//mslist<hitent_t> Hits;
	Hits.clearitems();

	if (FBitSet(Damage.iDamageType, DMG_DIRECT) && Damage.pDirectDmgTarget)
	{
		Hits.add(hitent_t(0, Damage.pDirectDmgTarget));
	}
	else if (Damage.flRange > 0)
	{
		CBaseEntity *pTarget = NULL, *pClosestHit = NULL;
		while ((pTarget = UTIL_FindEntityInSphere(pTarget, Damage.vecSrc, Damage.flRange)) != NULL)
		{
			if (pTarget->pev->takedamage == DAMAGE_NO || //Don't hurt world objects, godmode entities, or the entity I originally struck
				FBitSet(pTarget->pev->flags, FL_GODMODE) ||
				!pTarget->IsAlive() ||
				pTarget == Damage.pInflictor ||
				!pTarget->pev->model) //Special thing for the firewall spell
				continue;

			if (pTarget == Damage.pAttacker && !FBitSet(Damage.iDamageType, DMG_REFLECTIVE))
				continue; //Only hit the owner if damage is reflective

			TraceResult tr;
			dbg("Do radius Traceline");
			UTIL_TraceLine(Damage.vecSrc, pTarget->Center(), ignore_monsters, Damage.pInflictor->edict(), &tr);

			if (tr.flFraction < 1.0f)
				continue;

			float dist = (Damage.vecSrc - tr.vecEndPos).Length();

			if (!Damage.flAOERange) //Non-AOE
			{
				if (!Damage.pAttacker || Damage.pAttacker->FInViewCone(tr.vecEndPos, VIEW_FIELD_NARROW))
					if (!Hits.size() || Hits[0].Dist > dist)
					{
						Damage.outTraceResult = tr;
						hitent_t Hit(dist, pTarget);
						if (!Hits.size())
							Hits.add(Hit);
						else
							Hits[0] = Hit;
					}
			}
			else //AOE
			{
				float DmgRatio = 1.0f - (dist / Damage.flAOERange);
				if (DmgRatio <= 0)
					continue;

				damage_t RadiusDmg = Damage;
				RadiusDmg.vecEnd = pTarget->Center();
				RadiusDmg.flAOERange = RadiusDmg.flAOEAttn = 0.0f;	   //Ensure it doesn't don't do ranged damage again - would cause infinite loop
				RadiusDmg.flDamage *= pow(DmgRatio, Damage.flAOEAttn); //AOE Damage = Base Damage * [(1-[distance to target/maxrange]) ^ attenuation]
				SetBits(RadiusDmg.iDamageType, DMG_DIRECT);			   //Just do direct damage to this entity.  We've already check if its a valid ent
				RadiusDmg.pDirectDmgTarget = pTarget;
				CBaseEntity *pHit = DoDamage(RadiusDmg, pTarget);
				if (pHit)
					Hits.add(hitent_t(dist, pHit));
			}
		}
	}

	dbg("Damage Targets");
	if (!Damage.flAOERange)
	{
		if (Hits.size())
		{
			for (int h = 0; h < Hits.size(); h++)
			{
				//Thothie FEB2009 - critical crash fix - make sure attacker is alive when passing damage
				//- if he damages multiple targets while dead, it causes crash
				dbg("Damage Targets->DoDamage");
				CBaseEntity *pDmgEnt = Damage.pAttacker;
				//if ( pDmgEnt->IsAlive() ) DoDamage( Damage, Hits[h].pEntity );
				if (pDmgEnt)
				{
					if (!pDmgEnt->IsPlayer())
					{
						DoDamage(Damage, Hits[h].pEntity);
					}
					else
					{
						if (pDmgEnt->IsAlive())
							DoDamage(Damage, Hits[h].pEntity);
					}
				}
			}
		}
		else
		{
			dbg("Damage Targets->Tracelin");
			//Didn't hit any NPCs with a normal attack.  Do a traceline to hit worldmodels, breakables, etc.
			int trflags = MSTRACE_SOLIDSHIELDS;

			if (FBitSet(Damage.iDamageType, DMG_SIMPLEBBOX))
				gpGlobals->trace_flags |= FTRACE_SIMPLEBOX;
			else
				SetBits(trflags, MSTRACE_LARGEHITBOXES);

			if (Damage.pAttacker)
				SetBits(trflags, MSTRACE_HITCORPSES);
			MSTraceLine(Damage.vecSrc, Damage.vecEnd, dont_ignore_monsters, Damage.pAttacker->edict(), Damage.outTraceResult, trflags);

			if ((Damage.outTraceResult.flFraction < 1.0f) && Damage.outTraceResult.pHit)
			{
				CBaseEntity *pHit = DoDamage(Damage, CBaseEntity::Instance(Damage.outTraceResult.pHit));
				if (pHit)
					Hits.add(hitent_t((Damage.vecEnd - Damage.vecSrc).Length() * Damage.outTraceResult.flFraction, pHit));
			}
			//BeamEffect( vecSrc, vecEnd, iBeam, 0, 0, 100, 10, 0, 255, 255, 255, 255, 20 );
		}
	}

	enddbg;
}

//MIB MAR2008a - massive changes
CBaseEntity *DoDamage(damage_t &Damage, CBaseEntity *pTarget)
{
	startdbg;
	dbg("Hit Target");

	if (FBitSet(Damage.iDamageType, DMG_NONE))
		return pTarget; //Don't do any damage, but target the ent

	CBaseEntity *pEntityAttacker = Damage.pAttacker;
	CBaseEntity *pEntityInflictor = Damage.pInflictor;

	char sz[256] = "";

	if (!pEntityAttacker)
		return NULL; //This happens with arrows sometimes... for some reason....

	CMSMonster *pAttMonster = pEntityAttacker->IsMSMonster() ? (CMSMonster *)pEntityAttacker : NULL;	//Attacking Monster	(Can be NULL)
	CBasePlayer *pPlayerAttacker = pEntityAttacker->IsPlayer() ? (CBasePlayer *)pEntityAttacker : NULL; //Attacking Player	(Can be NULL)
	CGenericItem *pItemInflictor = pEntityInflictor ? (pEntityInflictor->IsMSItem() ? (CGenericItem *)pEntityInflictor : NULL) : NULL;

	//This might be a summon, or other effect that is doing damage for the player...
	if (pAttMonster && !pPlayerAttacker)
	{ //if so, assign the player pointer correctly
		CBaseEntity *pExpOwner = pAttMonster->RetrieveEntity(ENT_EXPOWNER);
		if (pExpOwner && pExpOwner->IsPlayer())
			pPlayerAttacker = (CBasePlayer *)pExpOwner;
	}

	//bActualHit: Did the combination of luck&skill produce a hit?
	//bool fDidHit = FALSE, fHitWorld = TRUE, bActualHit = FALSE,
	//	fReportHit = FALSE, fDodged = FALSE;

	SetDebugProgress(ItemThinkProgress, "DoDamage - Check hit");
	bool fReportHit = false;
	Damage.AttackHit = false;

	CMSMonster *pVictim = NULL;

	int iAccuracyRoll = 0;
	if (pTarget)
	{
		SetDebugProgress(ItemThinkProgress, "DoDamage - Entity hit");
		//Hit an entity
		//Check if I can damage it
		Damage.AttackHit = pEntityAttacker->CanDamage(pTarget);
		fReportHit = false;

		if (pAttMonster && Damage.AttackHit)
		{
			//Hit a monster...
			fReportHit = true;

			pVictim = pTarget->IsMSMonster() ? (CMSMonster *)pTarget : NULL;

			if (pVictim && pVictim->IsAlive())
			{
				//If player is attacking himself, don't report
				if (pPlayerAttacker == pVictim)
					fReportHit = false;

				//Check if your horrid skill or bad luck made you miss:
				iAccuracyRoll = RANDOM_LONG(0, 99);
				if (iAccuracyRoll > Damage.flHitPercentage)
				{
					Damage.AttackHit = false;
					Damage.AttackHit = false;
				}
			}
			else
			{
				// Always hit world entities or dead bodies

				Damage.AttackHit = true;
				fReportHit = false;
			}
		}

		SetDebugProgress(ItemThinkProgress, "DoDamage - Check if damage is dealt");
		bool fDodged = false;
		if (Damage.AttackHit) //check again, because it might have been canceled in game_damaged_other
		{
			//Hit sounds are now played from TraceAttack
			ClearMultiDamage();
			Damage.AccuracyRoll = (Damage.flHitPercentage - iAccuracyRoll);

			if (pEntityInflictor)
				pEntityInflictor->StoreEntity(pTarget, ENT_LASTSTRUCKBYME);
			if (pAttMonster)
			{
				pAttMonster->StoreEntity(pTarget, ENT_LASTSTRUCKBYME);
				static msstringlist Params;
				Params.clearitems();

				Params.add(EntToString(pTarget));
				Params.add(UTIL_VarArgs("%f", Damage.flDamage));
				Params.add(Damage.sDamageType.c_str());
				Params.add(Damage.dodamage_event.len() > 0 ? Damage.dodamage_event.c_str() : "(none)"); //Thothie DEC2017_01 - see below
				pAttMonster->CallScriptEvent("game_damaged_other", &Params);

				if (pItemInflictor)
				{
					msstring CallbackName = "game_damaged_other";
					if (Damage.ItemCallBackPrefix)
						CallbackName = *Damage.ItemCallBackPrefix + "_damaged_other";
					pItemInflictor->m_CurrentDamage = &Damage; //m_CurrentDamage used by script commands to change damage
					pItemInflictor->CallScriptEvent(CallbackName, &Params);
					if(Damage.ItemCallBackPrefix)
						pItemInflictor->CallScriptEvent("weapon_damaged_other", &Params); //Thothie OCT2016_05 adding generic callback for weapon mods
					pItemInflictor->m_CurrentDamage = NULL;
				}

				//Script decided to change or cancel damage from within game_damaged_other
				if (pAttMonster->m_ReturnData.len())
				{
					//Each script either sets a ratio of damage to be dealt or cancels the damage
					msstringlist DamageRatios;
					TokenizeString(pAttMonster->m_ReturnData, DamageRatios);
					for (int i = 0; i < DamageRatios.size(); i++)
					{
						if (DamageRatios[i] == "canceldamage")
						{
							Damage.AttackHit = false;
							fReportHit = false;
							pTarget = NULL;
							goto EndDamage; //Script canceled, skip the rest
						}

						Damage.flDamage *= atof(DamageRatios[i]);
					}
				}
			}

			//Deal Damage
			//===========
			dbg("Deal Damage->entvars_t");
			entvars_t *pevInflictor = Damage.pInflictor ? Damage.pInflictor->pev : NULL;
			entvars_t *pevAttacker = Damage.pAttacker ? Damage.pAttacker->pev : NULL;
			float flDamage = Damage.flDamage;
			//Non-ranged attack
			dbg("Deal Damage->pTarget");
			if (!pTarget->IsMSMonster())
				flDamage = pTarget->TraceAttack(pevInflictor, pevAttacker, Damage.flDamage, gpGlobals->v_forward, &Damage.outTraceResult, Damage.iDamageType, Damage.AccuracyRoll);
			else
				flDamage = ((CMSMonster *)pTarget)->TraceAttack(Damage);

			SetDebugProgress(ItemThinkProgress, "DoDamage - Apply damage");
			if (flDamage > 0) // flDamage < 0 means monster dodged it
			{
				//MiB Mar2008a - Relocated exp assigning here so that armor and other
				//damage recalculations could be done (stops exp from parry and things
				//of that sort)
				dbg("Store XP for Attack");
				if (pPlayerAttacker &&				 //Only if player is attacking...
					pVictim && !pVictim->IsPlayer()) //Only when attacking monsters...
				{
					//Advance your skill based on monster difficulty and num of swings
					//Find the stat that increases from this attack
					if(Damage.ExpUseProps)
					{
						pVictim->MarkDamage(pPlayerAttacker, Damage.ExpStat, Damage.ExpProp, flDamage);
					}
					else
					{
						pVictim->MarkDamage(pPlayerAttacker, pPlayerAttacker->ActiveItem(), flDamage);
					}
				}

				dbg("ApplyMultiDamage");
				//ALERT( at_console,"ApplyMultiDamage: %s \n",pPlayerAttacker->DisplayName());
				if (pevAttacker)
					ApplyMultiDamage(pevInflictor, pevAttacker); //Thothie FEB2009 - experimenting
			}
			else if (!flDamage && !Damage.pAttacker->IsPlayer())
			{
				//If monsters do 0 damage, just consider it not hitting
				//This also prevents orc archers arrows from sticking into
				//other orcs
				dbg("EndDamage");
				goto EndDamage;
			}
			else if (flDamage == -1)
			{
				dbg("NoDamage");
				ClearMultiDamage();
				fDodged = true;
				Damage.AttackHit = false;
			}
		}
		if (pVictim)
			pVictim->Attacked(pEntityAttacker, Damage);

		//Report Damage
		//=============

		SetDebugProgress(ItemThinkProgress, "DoDamage - Report damage");
		if (fReportHit)
		{
			if (Damage.flDamage > 0)
			{
				char szStats[32], szDamage[32], szHitMiss[32];
				//Thothie JAN2013b_25 - elemental codes
				msstring dtype_code = Damage.sDamageType.c_str();
				char element_code[11] = "";
				if (dtype_code.starts_with("fire"))
					 strncpy(element_code,  " fire", sizeof(element_code) );
				else if (dtype_code.starts_with("cold"))
					 strncpy(element_code,  " cold", sizeof(element_code) );
				else if (dtype_code.starts_with("lightning"))
					 strncpy(element_code,  " lightning", sizeof(element_code) );
				else if (dtype_code.starts_with("poison"))
					 strncpy(element_code,  " poison", sizeof(element_code) );
				else if (dtype_code.starts_with("acid"))
					 strncpy(element_code,  " acid", sizeof(element_code) );
				else if (dtype_code.starts_with("slash"))
					 strncpy(element_code,  " slash", sizeof(element_code) );
				else if (dtype_code.starts_with("blunt"))
					 strncpy(element_code,  " blunt", sizeof(element_code) );
				else if (dtype_code.starts_with("pierce"))
					 strncpy(element_code,  " pierce", sizeof(element_code) );
				else if (dtype_code.starts_with("magic"))
					 strncpy(element_code,  " magic", sizeof(element_code) );
				else if (dtype_code.starts_with("holy"))
					 strncpy(element_code,  " holy", sizeof(element_code) );
				else if (dtype_code.starts_with("dark"))
					 strncpy(element_code,  " dark", sizeof(element_code) );
				else if (dtype_code.starts_with("apostle"))
					 strncpy(element_code,  " apostle", sizeof(element_code) );
				else if (dtype_code.starts_with("earth"))
					 strncpy(element_code,  " earth", sizeof(element_code) );

				 strncpy(szDamage, Damage.AttackHit ? UTIL_VarArgs("%.1f%s damage.", Damage.flDamage, element_code) : "", sizeof(szDamage) );
				 strncpy(szHitMiss, Damage.AttackHit ? "HIT!" : (fDodged ? "PARRIED!" : "MISS!"), sizeof(szHitMiss) );
				 _snprintf(szStats, sizeof(szStats), "(%i/%i)",  (100 - iAccuracyRoll),  int(100 - Damage.flHitPercentage) );

				if (pPlayerAttacker)
				{
					 _snprintf(sz, sizeof(sz), "You attack %s %s. %s %s%s",
					 		pTarget->DisplayPrefix.c_str(), //Thothie AUG2007b - display name prefix when attacking - thought it already did?
							pTarget->DisplayName(),
							szStats, szHitMiss, szDamage);
					pPlayerAttacker->SendEventMsg(HUDEVENT_ATTACK, sz);
				}
				
				//Thothie SEP2019_22 - report resistance BEGIN
				bool tdm_found_entry = false;
				float tdm_modifier;
				if (pVictim)
				{
					for (int i = 0; i < pVictim->m.TakeDamageModifiers.size(); i++)
					{
						CMSMonster::takedamagemodifier_t &TDM = pVictim->m.TakeDamageModifiers[i];
						msstring tdm_damage_type = TDM.DamageType;
						if (tdm_damage_type.contains(dtype_code))
						{
							tdm_modifier = TDM.modifier;
							tdm_found_entry = true;
							break;
						}
					}
				}
				
				msstring tdm_engrish = " ";
				if ( tdm_found_entry )
				{
					if ( tdm_modifier < 1 )
					{
						tdm_engrish = UTIL_VarArgs("[%i%% resistant]", int((1 - tdm_modifier)*100.0));
					}
					else if ( tdm_modifier > 1 )
					{
						tdm_engrish = UTIL_VarArgs("[%i%% vulnerable]", int((tdm_modifier-1)*100.0));
					}
				}
				//Thothie SEP2019_22 - report resistance END	
				
				if (pVictim && pVictim->IsPlayer() && pEntityAttacker && pVictim != pEntityAttacker)
				{
					CMSMonster *pMonster = (CMSMonster *)pEntityAttacker;
					CBasePlayer *pPlayer = (CBasePlayer *)pTarget;
					
				 _snprintf(sz, sizeof(sz), "%s attacks you. %s %s %s %s", 
				 		SPEECH::NPCName(pMonster, true),
						szStats, 
						szHitMiss, 
						szDamage,
						tdm_engrish.c_str()
					);
						
					pPlayer->SendEventMsg(HUDEVENT_ATTACKED, sz);
				}
			} //endif Damage.flDamage > 0
		}

		SetDebugProgress(ItemThinkProgress, "DoDamage - Countereffect");
		pTarget->CounterEffect(pEntityInflictor, 0, (void *)&Damage.flDamage);
		//if( fHitWorld )
		//{
		//	if( pTarget->MSProperties()&ITEM_SHIELD )
		//		pTarget->CounterEffect( MSInstance(ENT(pInflictor)), ITEM_SHIELD, (void *)&flDamage );
		//}

		// Make a decal
		if (!Damage.nodecal)
			DecalGunshot(&Damage.outTraceResult, BULLET_PLAYER_CROWBAR);

		if (pEntityAttacker)
		{
			float Range = 128 + Damage.flDamage * 90; //Min 128 units, and add 90 units for each dmg pt
			CSoundEnt::InsertSound(pEntityAttacker, "combat", Damage.outTraceResult.vecEndPos, Range, 0.3f, 0);
		}
	}

EndDamage:

	//Set some script variables
	dbg("Set post damage variables");
	if (pAttMonster)
	{
		Vector &EndPos = Damage.AttackHit ? Damage.outTraceResult.vecEndPos : Damage.vecEnd;

		static msstringlist Parameters;
		Parameters.clearitems();
		Parameters.add(Damage.AttackHit ? "1" : "0");
		Parameters.add(pTarget ? EntToString(pTarget) : "none");
		Parameters.add(VecToString(Damage.vecSrc));
		Parameters.add(VecToString(EndPos));
		Parameters.add(Damage.sDamageType.c_str());
		char szDamage[32];
		strncpy(szDamage, Damage.AttackHit ? UTIL_VarArgs(" %.1f damage.",  Damage.flDamage) : "0", sizeof(szDamage) );
		Parameters.add(szDamage);
		pAttMonster->CallScriptEvent("game_dodamage", &Parameters);
		if (Damage.dodamage_event.len() > 0)
		{
			msstring dmgevent = Damage.dodamage_event;
			dmgevent += "_dodamage";
			if (dmgevent.starts_with("*"))
			{
				dmgevent = dmgevent.substr(1);
				if (pItemInflictor) pItemInflictor->CallScriptEvent(dmgevent.c_str(), &Parameters);
				else if (pEntityInflictor) pEntityInflictor->GetScripted()->CallScriptEvent(dmgevent.c_str(), &Parameters); //Allows dmgevent for non-item scripts
			}
			else
			{
				pAttMonster->CallScriptEvent(dmgevent.c_str(), &Parameters);
			}
		}

		if (pItemInflictor)
		{
			msstring CallbackName = "game_dodamage";
			if (Damage.ItemCallBackPrefix)
				CallbackName = *Damage.ItemCallBackPrefix + "_dodamage";
			pItemInflictor->CallScriptEvent("game_dodamage", &Parameters);
		}
	}

	return pTarget;
	enddbg;
	return NULL;
}
#endif
