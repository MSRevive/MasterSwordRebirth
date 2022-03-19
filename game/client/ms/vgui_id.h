//
//  This should only be included by vgui_HUD.cpp
//

class VGUI_ID : public Panel
{
public:
	VGUI_FadeText *m_Label[2];
	entinfo_t *m_LastID;
	cl_entity_s *m_pClientEnt;

	VGUI_ID(Panel *pParent, int x, int y) : Panel(x, y, XRES(200), YRES(36))
	{
		setParent(pParent);
		setBgColor(0, 0, 0, 255);

		for (int i = 0; i < 2; i++)
		{
			m_Label[i] = new VGUI_FadeText(this, 1, "", 0, 0 + i * g_FontID->getTall(), MSLabel::a_west);
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

		static char pchOutputString[MSSTRING_SIZE];
		_snprintf(pchOutputString, MSSTRING_SIZE, "%s", m_LastID->Name.c_str());

		if (m_pClientEnt == NULL)
		{
			m_Label[0]->setText(pchOutputString);
			return;
		}

		int maxHP = ((int)m_pClientEnt->curstate.vuser3.x);
		int currHP = ((int)m_pClientEnt->curstate.vuser3.y);
		if ((maxHP <= 0) || (currHP <= 0))
		{
			m_Label[0]->setText(pchOutputString);
			return;
		}

		_snprintf(pchOutputString, MSSTRING_SIZE, "%s (%i / %i)", m_LastID->Name.c_str(), currHP, maxHP);
		m_Label[0]->setText(pchOutputString);
	}

	void Update(entinfo_t *pEntInfo)
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

			}

			m_Label[1]->setText(String);
			m_Label[1]->SetFGColorRGB(DifficultyColor);
			m_Label[0]->StartFade(false);
			m_Label[1]->StartFade(false);
		}
		else
		{
			for (int i = 0; i < 2; i++)
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
		for (int i = 0; i < 2; i++)
			m_Label[i]->Update();
	}
	void Update()
	{
		for (int i = 0; i < 2; i++)
			m_Label[i]->Update();
		setVisible(ShowHUD());
	}
	void NewLevel()
	{
		for (int i = 0; i < 2; i++)
		{
			m_Label[i]->m_StartTime = -1000; //Ensure ID doesn't show up after a level change
			m_Label[i]->setText("");
		}
	}
};