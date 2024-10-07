//
//  This should only be included by vgui_HUD.cpp
//

#include <vector>
#include "vgui_ihud.h"

//Scales flasks down to only 40% wide of the screen if sprites are too big
#define BAR_SCALE (1.0f - ((730 - (ScreenWidth * 0.40f)) / ScreenHeight))
#define BAR_W (320 * BAR_SCALE)
#define BAR_H (40 * BAR_SCALE)

#define EMBLEM_SIZE (90 * BAR_SCALE)

namespace PrimaryHUD
{
	class VGUI_Bar : public Panel
	{
	private:
		VGUI_Image3D m_Image;
		MSLabel *m_Label;
		short m_Type;
		float m_CurrentAmt;

	public:
		VGUI_Bar(Panel *pParent, int Type, int x, int y) : Panel(x, y, BAR_W, BAR_H)
		{
			setParent(pParent);
			setBgColor(0, 0, 0, 255);
			m_Type = Type;

			m_Image.setParent(this);

			switch(Type) {
				case 0:
					m_Image.LoadImg("hud/healthbar", false, false);
					break;
				case 1:
					m_Image.LoadImg("hud/manabar", false, false);
					break;
				case 2:
					m_Image.LoadImg("hud/weightbar", false, false);
					break;
				case 3:
					m_Image.LoadImg("hud/stambar", false, false);
					break;
			}

			m_Image.setSize(getWide(), getTall());
			
			m_Label = new MSLabel(this, "0/0", 0, getTall()/5, getWide(), YRES(8), MSLabel::a_center);
		}

		void Update()
		{
			float Amt, MaxAmt;

			switch (m_Type) {
				case 0: 
					Amt = player.m_HP;
					MaxAmt = player.MaxHP();
					break;
				case 1: 
					Amt = player.m_MP;
					MaxAmt = player.MaxMP();
					break;
				case 2: 
					Amt = player.Weight();
					MaxAmt = player.Volume();
					break;
				case 3: 
					Amt = player.Stamina;
					MaxAmt = player.MaxStamina(); 
					break;
			}
			
			int LastFrame = m_Image.GetMaxFrames() - 1;

			//thothie attempting to fix scrolling flasks
			int AccelFlasks = 10;
			AccelFlasks = 15 * (MaxAmt / 100);
			if (AccelFlasks < 40)
				AccelFlasks = 40;
			if (AccelFlasks > MaxAmt)
				AccelFlasks = MaxAmt - 1;
			if (fabs(m_CurrentAmt - Amt) > 200)
				AccelFlasks = 1000;

			float MaxChange = gpGlobals->frametime * AccelFlasks; //original line:  float MaxChange = gpGlobals->frametime * 40;

			if (m_CurrentAmt < 0)
				m_CurrentAmt = 0.0;
			if (m_CurrentAmt > 3000)
				m_CurrentAmt = 3000.0;
			//[/Thothie]

			if (m_CurrentAmt < Amt)
				m_CurrentAmt += V_min(MaxChange, Amt - m_CurrentAmt);
			else if (m_CurrentAmt > Amt)
				m_CurrentAmt -= V_min(MaxChange, m_CurrentAmt - Amt);

			float frame = (m_CurrentAmt / MaxAmt) * LastFrame;
			if (frame > 0 && frame < 1)
				frame = 1; //Cap at 1, unless dead
			if (frame > LastFrame)
				frame = LastFrame;
			frame = V_max(frame, 0);

			m_Image.SetFrame(frame);

			m_Label->setText(UTIL_VarArgs("%i/%i ", (int)m_CurrentAmt, (int)MaxAmt)); //the space is intentional
			m_Label->SetFGColorRGB(COLOR(255, 255, 255, 10));
			if (m_Type != 2) {
				if (m_CurrentAmt > MaxAmt / 4.0f)
					m_Label->SetFGColorRGB(COLOR(255, 255, 255, 10));
				else
					m_Label->SetFGColorRGB(COLOR(250, 0, 0, 10));
			}

			setVisible(ShowHealth());
		}
	};

	class VGUI_Health : public Panel, public IHUD
	{
	private:
		float vChargeLevelAmt = 0;
		float vCurChargeAmt = 0;
		float vDisplayChargeLevel = 0;
		int mCurChargeLevel = 1;
		int vCurChargeLevel = 0;

		int CHARGE_W = 0;
		int CHARGE_H = 0;
		int CHARGE_SPACER_W = 0;

		class VGUI_Bar *m_Bar[4];

		CStatusBar *m_Charge[2];
		MSLabel *m_ChargeLbl[2];

		//Emblem
		VGUI_Image3D m_HUDImage;

	public:
		//Main HUD Image
		VGUI_Health(Panel* pParent) : Panel(0, 0, ScreenWidth, ScreenHeight)
		{
			CHARGE_W = XRES(30);
			CHARGE_H = YRES(6);
			CHARGE_SPACER_W = XRES(2);

			setParent(pParent);
			SetBGColorRGB(Color_Transparent);

			//Point defines where status bars are positioned relative to and the max screen space its allowed to take before scaling
			float coords[2];

			coords[0] = 10; //x
			coords[1] = (ScreenHeight - (2 * BAR_H) - 10); //y, from the bottom of the screen, as high as the sprites are

			//	Status Bars
			
			//Health bar
			m_Bar[0] = new VGUI_Bar(this, 0, coords[0], coords[1]);

			//weight bar
			m_Bar[2] = new VGUI_Bar(this, 2, coords[0], coords[1] + BAR_H);

			//Mana bar
			m_Bar[1] = new VGUI_Bar(this, 1, coords[0] + BAR_W + EMBLEM_SIZE - 1, coords[1]);

			//stam bar
			m_Bar[3] = new VGUI_Bar(this, 3, coords[0] + BAR_W + EMBLEM_SIZE - 1, coords[1] + BAR_H);

			m_HUDImage.setParent(this);
			m_HUDImage.LoadImg("hud_main", true, false);
			m_HUDImage.setSize(EMBLEM_SIZE, EMBLEM_SIZE);
			m_HUDImage.setPos(coords[0] + BAR_W, coords[1] - (7 * BAR_SCALE));

			//Charge system
			for (int i = 0; i < 2; i++)
			{
				int Multiplier = (i == 0) ? -1 : 1;
				float OffsetW = CHARGE_SPACER_W + (i == 0) ? CHARGE_W : 0;
				m_Charge[i] = new CStatusBar(this, XRES(304) + OffsetW * Multiplier, YRES(408), CHARGE_W, CHARGE_H);
				m_Charge[i]->SetBGColorRGB(COLOR(128, 128, 128, 100));
				//m_Charge[i]->m_fBorder = false;
				m_Charge[i]->setVisible(false);
				m_ChargeLbl[i] = new MSLabel(this, "0/0", XRES(304) + OffsetW * Multiplier, YRES(408), CHARGE_W, CHARGE_H, MSLabel::a_center);
				m_ChargeLbl[i]->setVisible(false);
			}
		}

		//MiB NOV2007a - Moar Charge Colors!
		void Update()
		{
			//Update flasks
			for (int i = 0; i < 4; i++)
				m_Bar[i]->Update();

			bool bShowHealth = ShowHealth();

			m_HUDImage.setVisible(bShowHealth);

			for (int i = 0; i < 2; i++)
			{
				m_Charge[i]->setVisible(false);
				m_ChargeLbl[i]->setVisible(false);
			}

			//second attempt to fix phantom charge bar
			std::vector<CGenericItem*> *vHandsItems = new std::vector<CGenericItem*>;

			//get all hands items
			for (int i = 0; i < player.Gear.size(); i++)
			{
				CGenericItem* Item = player.Gear[i];
				if (Item->m_Location != ITEMPOS_HANDS)
					continue;

				vHandsItems->push_back(Item);
			}

			for (int i = 0; i < vHandsItems->size(); i++)
			{
				CGenericItem* Item = vHandsItems->at(i);

				//if we have more than one item in our hand ignore playerhands.
				if (vHandsItems->size() > 1 && Item->m_Hand == HAND_PLAYERHANDS)
				{
					continue;
				}

				int Bar = Item->m_Hand < 2 ? Item->m_Hand : 1;
				CStatusBar &ChargeBar = *m_Charge[Bar];
				MSLabel &ChargeLbl = *m_ChargeLbl[Bar];

				if (Item->Attack_IsCharging() && (vCurChargeAmt = Item->Attack_Charge()) > 0)
				{
					ChargeBar.setVisible(bShowHealth);
					bool notDone = true;

					int vChargeLevel = 1;
					int vChargeR = 0;
					int vChargeG = 0;
					int vChargeB = 0;

					while (notDone)
					{
						vChargeLevelAmt = GET_CHARGE_FROM_TIME(vChargeLevel);

						if (vCurChargeAmt <= vChargeLevelAmt)
						{
							notDone = false;
							ChargeBar.SetFGColorRGB(COLOR(vChargeR, vChargeG, vChargeB, 128));

							if (vChargeLevel != 1)
							{
								vCurChargeAmt -= GET_CHARGE_FROM_TIME(vChargeLevel - 1);
								vCurChargeAmt /= (GET_CHARGE_FROM_TIME(vChargeLevel) - GET_CHARGE_FROM_TIME(vChargeLevel - 1));
							}
						}

						vChargeR += 100;
						if (vChargeR > 255)
						{
							vChargeR -= 255;
							vChargeG += 100;
							if (vChargeG > 255)
							{
								vChargeG -= 255;
								vChargeB += 100;
								if (vChargeB > 255)
									vChargeB -= 255;
							}
						}

						vChargeLevel++;
					}
					
					vCurChargeLevel = (int)((vChargeLevel - 1) + vCurChargeAmt);
					vDisplayChargeLevel = vCurChargeLevel - 1;

					if(vCurChargeLevel > mCurChargeLevel)
						PlayHUDSound(gEngfuncs.pfnGetCvarString("ms_chargebar_sound"), gEngfuncs.pfnGetCvarFloat("ms_chargebar_volume"));

					mCurChargeLevel = vCurChargeLevel;

					if (vDisplayChargeLevel)
						ChargeLbl.setText(msstring() + vDisplayChargeLevel);
					else
						ChargeLbl.setText(" ");

					ChargeLbl.setVisible(bShowHealth);
					ChargeBar.Set(vCurChargeAmt * 100);
				}
			}
			delete vHandsItems;
		}
	};
}