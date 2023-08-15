//
//  This should only be included by vgui_HUD.cpp
//

#include <vector>

static COLOR Color_Text_LowHealth(250, 0, 0, 10),
	Color_Charge_Lvl1(128, 128, 128, 128), Color_Charge_Lvl2(255, 100, 100, 128),
	Color_Charge_Lvl3(100, 230, 30, 128),
	Color_Charge_BG(128, 128, 128, 100);
static COLOR HighColor(0, 255, 0, 128), MedColor(255, 255, 0, 128), LowColor(255, 0, 0, 128);

//Scales flasks down to only 40% wide of the screen if sprites are too big
#define FLASK_SCALE (1.0f - (((320 * 2.0f) - (ScreenWidth * 0.40f)) / ScreenWidth))

//Dimensions of the "Flask" sprites
#define FLASK_W 320 * FLASK_SCALE
#define FLASK_H 48 * FLASK_SCALE

class VGUI_Flask : public Panel
{
public:
	VGUI_Image3D m_Image;
	MSLabel *m_Label;
	int m_Type;
	float m_CurrentAmt;

	VGUI_Flask(Panel *pParent, int Type, int x, int y) : Panel(x, y, FLASK_W, FLASK_H)
	{
		setParent(pParent);
		setBgColor(0, 0, 0, 255);
		m_Type = Type;

		msstring_ref ImageName = "hud/healthbar";

		switch(Type) {
			case 0: ImageName = "hud/healthbar";
				break;
			case 1: ImageName = "hud/manabar";
				break;
			case 2: ImageName = "hud/weightbar";
				break;
			case 3: ImageName = "hud/stambar";
				break;
		}

		m_Image.setParent(this);
		m_Image.LoadImg(ImageName, false, false);
		m_Image.setFgColor(255, 255, 255, 255);
		m_Image.setSize(getWide(), getTall());
		m_Label = new MSLabel(this, "0/0", 0, getTall()/5, getWide(), YRES(8), MSLabel::a_center);
	}

	void Update()
	{
		float Amt, MaxAmt;

		switch (m_Type) {
			case 0: Amt = player.m_HP;
				MaxAmt = player.MaxHP();
				break;
			case 1: Amt = player.m_MP;
				MaxAmt = player.MaxMP();
				break;
			case 2: Amt = player.Weight();
				MaxAmt = player.Volume();
				break;
			case 3: Amt = player.Stamina;
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
			m_CurrentAmt += min(MaxChange, Amt - m_CurrentAmt);
		else if (m_CurrentAmt > Amt)
			m_CurrentAmt -= min(MaxChange, m_CurrentAmt - Amt);

		float frame = (m_CurrentAmt / MaxAmt) * LastFrame;
		if (frame > 0 && frame < 1)
			frame = 1; //Cap at 1, unless dead
		if (frame > LastFrame)
			frame = LastFrame;
		frame = max(frame, 0);

		m_Image.SetFrame(frame);

		m_Label->setText(UTIL_VarArgs("%i/%i ", (int)m_CurrentAmt, (int)MaxAmt)); //the space is intentional
		if (m_CurrentAmt > MaxAmt / 4.0f)
			m_Label->SetFGColorRGB(Color_Text_White);
		else
			m_Label->SetFGColorRGB(Color_Text_LowHealth);

		setVisible(ShowHealth());
	}
};

class VGUI_Health : public Panel, public IHUD_Interface
{
protected:
	float vChargeLevelAmt = 0;
	float vCurChargeAmt = 0;
	float vDisplayChargeLevel = 0;
	int mCurChargeLevel = 1;
	int vCurChargeLevel = 0;

public:
	class VGUI_Flask *m_Flask[4];

	CStatusBar *m_Charge[2];
	MSLabel *m_ChargeLbl[2];

	//Emblem
	//VGUI_Image3D m_HUDImage;

	//Main HUD Image
	VGUI_Health(Panel* pParent) : Panel(0, 0, ScreenWidth, ScreenHeight)
	{
		startdbg;
		dbg("Begin");
		setParent(pParent);
		SetBGColorRGB(Color_Transparent);

		//Point defines where status bars are positioned relative to and the max screen space its allowed to take before scaling
		float coords[2];

		coords[0] = 10; //x
		coords[1] = (ScreenHeight - (2 * FLASK_H)); //y, from the bottom of the screen, as high as the sprites are

		//	Status Bars
		
		//Health bar
		m_Flask[0] = new VGUI_Flask(this, 0, coords[0], coords[1]);


		//Mana bar
		m_Flask[1] = new VGUI_Flask(this, 1, coords[0] + FLASK_W, coords[1]);


		//Stamina bar
		m_Flask[2] = new VGUI_Flask(this, 2, coords[0], coords[1] + FLASK_H);


		//Weight bar
		m_Flask[3] = new VGUI_Flask(this, 3, coords[0] + FLASK_W, coords[1] + FLASK_H);

//Charge system
#define CHARGE_W XRES(30)
#define CHARGE_H YRES(6)
#define CHARGE_SPACER_W XRES(2)
		//#define CHARGE_X XRES(320) - CHARGE_W/2

		dbg("Setup m_Charge[0] & m_Charge[1]");
		for (int i = 0; i < 2; i++)
		{
			int Multiplier = (i == 0) ? -1 : 1;
			float OffsetW = CHARGE_SPACER_W + (i == 0) ? CHARGE_W : 0;
			m_Charge[i] = new CStatusBar(this, XRES(304) + OffsetW * Multiplier, YRES(408), CHARGE_W, CHARGE_H);
			m_Charge[i]->SetBGColorRGB(Color_Charge_BG);
			//m_Charge[i]->m_fBorder = false;
			m_Charge[i]->setVisible(false);
			m_ChargeLbl[i] = new MSLabel(this, "0/0", XRES(304) + OffsetW * Multiplier, YRES(408), CHARGE_W, CHARGE_H, MSLabel::a_center);
			m_ChargeLbl[i]->setVisible(false);
		}

		enddbg;
	}

	//MiB NOV2007a - Moar Charge Colors!
	void Update()
	{
		//Update flasks
		for (int i = 0; i < 4; i++)
			m_Flask[i]->Update();

		bool bShowHealth = ShowHealth();

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