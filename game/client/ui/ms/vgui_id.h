//
//  This should only be included by vgui_HUD.cpp
//

class VGUI_ID : public Panel
{
public:
	COLOR cHPColor = COLOR(0, 0, 0, 0);;
	VGUI_FadeText* m_Label[3];
	entinfo_t* m_LastID;
	cl_entity_s* m_pClientEnt;

#define ID_X XRES(320)
#define ID_Y YRES(240)
#define HEALTHY_HPMOD 0.75
#define WOUNDED_HPMOD 0.50
#define CRITICAL_HPMOD 0.25

	VGUI_ID(Panel* pParent, int x, int y) : Panel(x, y, XRES(200), YRES(54))
	{
		setParent(pParent);
		setBgColor(0, 0, 0, 255);

		for (int i = 0; i < 3; i++)
		{
			m_Label[i] = new VGUI_FadeText(this, 1, "", 0, i * g_FontID->getTall(), MSLabel::a_west);
			m_Label[i]->setFont(g_FontID);
		}
		m_Label[0]->SetFGColorRGB(Color_Text_White);

		m_LastID = NULL;
		m_pClientEnt = NULL;
	}

	void SetStatus(void)
	{
		if (m_LastID == NULL)
			return;

		static char pchNameString[MSSTRING_SIZE];
		static char pchHealthString[MSSTRING_SIZE];
		_snprintf(pchNameString, MSSTRING_SIZE, "%s", m_LastID->Name.c_str());

		if (m_pClientEnt == NULL)
		{
			m_Label[0]->setText(pchNameString);
			return;
		}

		int maxHP = ((int)m_pClientEnt->curstate.vuser3.x);
		int currHP = ((int)m_pClientEnt->curstate.vuser3.y);
		if ((maxHP <= 0) || (currHP <= 0))
		{
			m_Label[0]->setText(pchNameString);
			return;
		}

		_snprintf(pchNameString, MSSTRING_SIZE, "%s", m_LastID->Name.c_str());
		m_Label[0]->setText(pchNameString);

		//move hp text to new line and make it colorful
		msstring sHPBracketName;

		//color the health portion appropriately
		if (currHP > maxHP * HEALTHY_HPMOD) {
			cHPColor = COLOR(0, 255, 0, 0);
			sHPBracketName = "Healthy";
		}
		else if (currHP > maxHP * WOUNDED_HPMOD)
		{
			cHPColor = COLOR(255, 255, 0, 0);
			sHPBracketName = "Wounded";
		}
		else if (currHP < maxHP * CRITICAL_HPMOD) {
			cHPColor = COLOR(255, 0, 0, 0);
			sHPBracketName = "Near Death";
		}

		_snprintf(pchHealthString, MSSTRING_SIZE, "%s (%i / %i)", sHPBracketName.c_str(), currHP, maxHP);
		m_Label[2]->setText(pchHealthString);
	}

	void Update(entinfo_t* pEntInfo)
	{
		SetStatus();

		if (pEntInfo == m_LastID)
			return;

		if (pEntInfo)
		{
			msstring String;
			COLOR DifficultyColor = COLOR(0, 255, 0, 0);

			switch (pEntInfo->Type)
			{
			case ENT_FRIENDLY:
			{
				String = "Friendly";
				DifficultyColor = COLOR(0, 255, 0, 0);
				break;
			}
			case ENT_WARY:
			{
				String = "Wary";
				DifficultyColor = COLOR(255, 255, 0, 0);
				break;
			}
			case ENT_HOSTILE:
			{
				String = "Hostile";
				DifficultyColor = COLOR(255, 0, 0, 0);
				break;
			}
			case ENT_DEADLY:
			{
				String = "Deadly";
				DifficultyColor = COLOR(255, 0, 0, 0);
				break;
			}
			case ENT_BOSS:
			{
				String = "Elite";
				DifficultyColor = COLOR(255, 128, 0, 0);
				break;
			}
			case ENT_NEUTRAL:
			{
				String = "Neutral";
				DifficultyColor = COLOR(200, 200, 200, 0);
				break;
			}

			}

			m_Label[1]->setText(String);
			m_Label[1]->SetFGColorRGB(DifficultyColor);
			m_Label[0]->StartFade(false);
			m_Label[1]->StartFade(false);
		}
		else
		{
			for (int i = 0; i < 3; i++)
			{
				float PrevDelta = gpGlobals->time - m_Label[i]->m_StartTime;
				PrevDelta = min(PrevDelta, m_Label[i]->m_FadeDuration);
				m_Label[i]->StartFade(true);
				m_Label[i]->m_StartTime = gpGlobals->time - (m_Label[i]->m_FadeDuration - PrevDelta);
			}
		}

		m_LastID = pEntInfo;
		m_pClientEnt = (pEntInfo ? gEngfuncs.GetEntityByIndex(pEntInfo->entindex) : NULL);
		SetStatus();
		for (int i = 0; i < 3; i++)
			m_Label[i]->Update();
	}
	void Update()
	{
		for (int i = 0; i < 3; i++)
			m_Label[i]->Update();

		//get our wanted position
		float flCVARIdOffsetX = CVAR_GET_FLOAT("msui_id_offsetx");
		float flCVARIdOffsetY = CVAR_GET_FLOAT("msui_id_offsety");

		//wee bad engine functions making me define four ints!
		int iIntendedX = ID_X + flCVARIdOffsetX;
		int iIntendedY = ID_Y + flCVARIdOffsetY;
		int iCurrentX;
		int iCurrentY;
		getPos(iCurrentX, iCurrentY);

		if (iCurrentX != iIntendedX || iCurrentY != iIntendedY)
		{
			setPos(iIntendedX, iIntendedY);
		}

		SetStatus();
		//attempting to make it update correctly since the health color is late to update.
		//i hate this vgui engine it is so annoying to deal with.

		int iTemp; // because who would want just a single one of the colors of an element ever right?
		int iCurrentIndendedAlpha;
		m_Label[0]->getFgColor(iTemp, iTemp, iTemp, iCurrentIndendedAlpha);
		//make sure we have the same alpha as the first element, fixes fade in as well as correctly updating health color in realtime
		m_Label[2]->setFgColor(cHPColor.r, cHPColor.g, cHPColor.b, iCurrentIndendedAlpha);

		setVisible(ShowHUD());
	}
	void NewLevel()
	{
		for (int i = 0; i < 3; i++)
		{
			m_Label[i]->m_StartTime = -1000; //Ensure ID doesn't show up after a level change
			m_Label[i]->setText("");
		}
	}
};