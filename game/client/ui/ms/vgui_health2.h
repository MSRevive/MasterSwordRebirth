//
//  This should only be included by vgui_HUD.cpp
//

#include <vector>

static COLOR HighColor(0, 255, 0, 128), MedColor(255, 255, 0, 128), LowColor(255, 0, 0, 128);

#define FLASK_W XRES(64 * 0.625) //I want 64x104 in 1024x768 res, and smaller in lower res
#define FLASK_H YRES(104 * 0.625)
#define FLASK_SPACER XRES(10)

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

		m_Image.setParent(this);
		
		switch(Type) {
			case 0:
				m_Image.LoadImg("hud/healthflask", false, false);
				break;
			case 1:
				m_Image.LoadImg("hud/manaflask", false, false);
				break;
		}
		
		m_Image.setFgColor(255, 255, 255, 255);
		m_Image.setSize(getWide(), getTall());
		m_Label = new MSLabel(this, "0/0", 0, getTall()/1.5, getWide(), YRES(8), MSLabel::a_center);
	}

	void Update()
	{
		float Amt = !m_Type ? player.m_HP : player.m_MP;
		float MaxAmt = !m_Type ? player.MaxHP() : player.MaxMP();
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
			m_Label->SetFGColorRGB(COLOR(255, 255, 255, 10));
		else
			m_Label->SetFGColorRGB(COLOR(250, 0, 0, 10)); //low health text colour

		setVisible(ShowHealth());
	}
};

class VGUI_Health2 : public Panel, public IHUD_Interface
{
protected:
	float vChargeLevelAmt = 0;
	float vCurChargeAmt = 0;
	float vDisplayChargeLevel = 0;
	int mCurChargeLevel = 1;
	int vCurChargeLevel = 0;

public:
	class VGUI_Flask *m_Flask[2];

	//Stamina ---------------------------
	CStatusBar *m_pStamina;

	//Weight ----------------------------
	CStatusBar *m_pWeight;
	CStatusBar *m_Charge[2];
	MSLabel *m_ChargeLbl[2];

	//Main HUD Image
	VGUI_Image3D m_HUDImage;

	VGUI_Health2(Panel *pParent) : Panel(0, 0, ScreenWidth, ScreenHeight)
	{
		setParent(pParent);
		SetBGColorRGB(Color_Transparent);

		m_HUDImage.setParent(this);
		m_HUDImage.LoadImg("hud_main2", true, false);
		//m_HUDImage.setFgColor( 255, 255, 255, 255 );
		m_HUDImage.setSize(getWide(), getTall());

//Health and mana flasks
#define FLASK_START_X XRES(30)
#define FLASK_START_Y YRES(480) - YRES(30) - FLASK_H
#define MANA_FLASK_X FLASK_START_X + FLASK_W + FLASK_SPACER
		m_Flask[0] = new VGUI_Flask(this, 0, FLASK_START_X, FLASK_START_Y);
		m_Flask[1] = new VGUI_Flask(this, 1, MANA_FLASK_X, FLASK_START_Y);

		//Stamina and weight bars

#define STAMINA_X FLASK_START_X
#define STAMINA_Y YRES(453)
#define STAMINA_SIZE_X FLASK_W + FLASK_SPACER + FLASK_W
#define STAMINA_SIZE_Y YRES(12)

		m_pStamina = new CStatusBar(this, STAMINA_X, STAMINA_Y, STAMINA_SIZE_X, STAMINA_SIZE_Y);
		m_pStamina->m_fBorder = false;
		//m_pStamina->SetBGColorRGB( BorderColor );
		//m_pStamina->SetBGColorRGB( Color_Transparent );

#define STAMINA_LBL_SIZE_Y YRES(10)

		MSLabel *pLabel = new MSLabel(m_pStamina, Localized("#STAMINA"), 0, (STAMINA_SIZE_Y / 2.0f) - (STAMINA_LBL_SIZE_Y / 2.0f), STAMINA_SIZE_X, STAMINA_LBL_SIZE_Y, MSLabel::a_center);
		pLabel->SetFGColorRGB(Color_Text_White);

#define WEIGHT_SIZE_Y YRES(10)

		COLOR WeightColor(250, 150, 0, 100);

		m_pWeight = new CStatusBar(this, STAMINA_X, STAMINA_Y + STAMINA_SIZE_Y, STAMINA_SIZE_X, WEIGHT_SIZE_Y);
		m_pWeight->m_fBorder = false;
		m_pWeight->SetFGColorRGB(WeightColor);

#define WEIGHT_LBL_SIZE_Y WEIGHT_SIZE_Y

		pLabel = new MSLabel(m_pWeight, Localized("#WEIGHT"), 0, (WEIGHT_SIZE_Y / 2.0f) - (WEIGHT_LBL_SIZE_Y / 2.0f), STAMINA_SIZE_X, WEIGHT_LBL_SIZE_Y, MSLabel::a_center);
		//pLabel->setFgColor( 255, 255, 255, 64 );
		pLabel->SetFGColorRGB(Color_Text_White);

//Charge system
#define CHARGE_W XRES(30)
#define CHARGE_H YRES(6)
#define CHARGE_SPACER_W XRES(2)
		//#define CHARGE_X XRES(320) - CHARGE_W/2

		for (int i = 0; i < 2; i++)
		{
			int Multiplier = (i == 0) ? -1 : 1;
			float OffsetW = CHARGE_SPACER_W + (i == 0) ? CHARGE_W : 0;
			m_Charge[i] = new CStatusBar(this, XRES(320) + OffsetW * Multiplier, STAMINA_Y, CHARGE_W, CHARGE_H);
			m_Charge[i]->SetBGColorRGB(COLOR(128, 128, 128, 100));
			//m_Charge[i]->m_fBorder = false;
			m_Charge[i]->setVisible(false);
			m_ChargeLbl[i] = new MSLabel(this, "0/0", XRES(320) + OffsetW * Multiplier, STAMINA_Y, CHARGE_W, CHARGE_H, MSLabel::a_center);
			m_ChargeLbl[i]->setVisible(false);
		}
	}

	//MiB NOV2007a - Moar Charge Colors!
	void Update()
	{
		//Update Health & Mana flasks
		for (int i = 0; i < 2; i++)
			m_Flask[i]->Update();

		bool bShowHealth = ShowHealth();

		m_pStamina->setVisible(bShowHealth);
		m_pWeight->setVisible(bShowHealth);
		m_HUDImage.setVisible(bShowHealth);

		//Update stamina, weight
		float flStaminaPercent = player.Stamina / player.MaxStamina();
		m_pStamina->Set(flStaminaPercent * 100.0f);
		if (flStaminaPercent < 0.15)
			m_pStamina->SetFGColorRGB(LowColor);
		else if (flStaminaPercent <= 0.85f)
			m_pStamina->SetFGColorRGB(MedColor);
		else
			m_pStamina->SetFGColorRGB(HighColor);

		m_pWeight->Set(player.Weight(), player.Volume());

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