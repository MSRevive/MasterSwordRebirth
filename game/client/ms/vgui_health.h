//
//  This should only be included by vgui_HUD.cpp
//

static COLOR Color_Text_LowHealth(250, 0, 0, 10),
	Color_Charge_Lvl1(128, 128, 128, 128), Color_Charge_Lvl2(255, 100, 100, 128),
	Color_Charge_Lvl3(100, 230, 30, 128),
	Color_Charge_BG(128, 128, 128, 100);
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

		msstring_ref ImageName = !Type ? "healthflask" : "manaflask";
		m_Image.setParent(this);
		m_Image.LoadImg(ImageName, false, false);
		m_Image.setFgColor(255, 255, 255, 255);
		m_Image.setSize(getWide(), getTall());
		m_Label = new MSLabel(this, "0/0", 0, getWide(), getWide(), YRES(12), MSLabel::a_center);
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
			m_Label->SetFGColorRGB(Color_Text_White);
		else
			m_Label->SetFGColorRGB(Color_Text_LowHealth);

		setVisible(ShowHealth());
	}
};

class VGUI_Health : public Panel, public IHUD_Interface
{
public:
	class VGUI_Flask *m_Flask[2];

	//Stamina ---------------------------
	CStatusBar *m_pStamina;

	//Weight ----------------------------
	CStatusBar *m_pWeight;
	CStatusBar *m_Charge[2];

	//Main HUD Image
	VGUI_Image3D m_HUDImage;

	VGUI_Health(Panel *pParent) : Panel(0, 0, ScreenWidth, ScreenHeight)
	{
		startdbg;
		dbg("Begin");
		setParent(pParent);
		SetBGColorRGB(Color_Transparent);

		dbg("Setup m_HUDImage");
		m_HUDImage.setParent(this);
		dbg("m_HUDImage.LoadImg");
		m_HUDImage.LoadImg("hud_main", true, false);
		//m_HUDImage.setFgColor( 255, 255, 255, 255 );
		dbg("m_HUDImage.setSize");
		m_HUDImage.setSize(getWide(), getTall());

//Health and mana flasks
#define FLASK_START_X XRES(30)
#define FLASK_START_Y YRES(480) - YRES(30) - FLASK_H
#define MANA_FLASK_X FLASK_START_X + FLASK_W + FLASK_SPACER
		dbg("Setup m_Flask[0]");
		m_Flask[0] = new VGUI_Flask(this, 0, FLASK_START_X, FLASK_START_Y);
		dbg("Setup m_Flask[1]");
		m_Flask[1] = new VGUI_Flask(this, 1, MANA_FLASK_X, FLASK_START_Y);

		//Stamina and weight bars

#define STAMINA_X FLASK_START_X
#define STAMINA_Y YRES(453)
#define STAMINA_SIZE_X FLASK_W + FLASK_SPACER + FLASK_W
#define STAMINA_SIZE_Y YRES(12)

		dbg("Setup m_pStamina");
		m_pStamina = new CStatusBar(this, STAMINA_X, STAMINA_Y, STAMINA_SIZE_X, STAMINA_SIZE_Y);
		m_pStamina->m_fBorder = false;
		//m_pStamina->SetBGColorRGB( BorderColor );
		//m_pStamina->SetBGColorRGB( Color_Transparent );

#define CHARGE_W XRES(30)
#define CHARGE_H YRES(6)
#define CHARGE_SPACER_W XRES(2)
		//#define CHARGE_X XRES(320) - CHARGE_W/2

		dbg("Setup m_Charge[0] & m_Charge[1]");
		for (int i = 0; i < 2; i++)
		{
			int Multiplier = (i == 0) ? -1 : 1;
			float OffsetW = CHARGE_SPACER_W + (i == 0) ? CHARGE_W : 0;
			m_Charge[i] = new CStatusBar(this, XRES(320) + OffsetW * Multiplier, STAMINA_Y, CHARGE_W, CHARGE_H);
			m_Charge[i]->SetBGColorRGB(Color_Charge_BG);
			//m_Charge[i]->m_fBorder = false;
			//m_Charge[i]->setVisible( false );
		}

#define STAMINA_LBL_SIZE_Y YRES(10)

		dbg("Setup Stamina Label");
		MSLabel *pLabel = new MSLabel(m_pStamina, Localized("#STAMINA"), 0, (STAMINA_SIZE_Y / 2.0f) - (STAMINA_LBL_SIZE_Y / 2.0f), STAMINA_SIZE_X, STAMINA_LBL_SIZE_Y, MSLabel::a_center);
		pLabel->SetFGColorRGB(Color_Text_White);

#define WEIGHT_SIZE_Y YRES(10)

		COLOR WeightColor(250, 150, 0, 100);

		dbg("Setup m_pWeight");
		m_pWeight = new CStatusBar(this, STAMINA_X, STAMINA_Y + STAMINA_SIZE_Y, STAMINA_SIZE_X, WEIGHT_SIZE_Y);
		m_pWeight->m_fBorder = false;
		m_pWeight->SetFGColorRGB(WeightColor);

#define WEIGHT_LBL_SIZE_Y WEIGHT_SIZE_Y

		dbg("Setup Weight Label");
		pLabel = new MSLabel(m_pWeight, Localized("#WEIGHT"), 0, (WEIGHT_SIZE_Y / 2.0f) - (WEIGHT_LBL_SIZE_Y / 2.0f), STAMINA_SIZE_X, WEIGHT_LBL_SIZE_Y, MSLabel::a_center);
		//pLabel->setFgColor( 255, 255, 255, 64 );
		pLabel->SetFGColorRGB(Color_Text_White);

		enddbg;
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
			m_Charge[i]->setVisible(false);

		for (int i = 0; i < player.Gear.size(); i++)
		{
			CGenericItem &Item = *player.Gear[i];
			if (Item.m_Location != ITEMPOS_HANDS)
				continue;

			if (Item.Attack_IsCharging())
			{
				int Bar = Item.m_Hand < 2 ? Item.m_Hand : 1;
				CStatusBar &ChargeBar = *m_Charge[Bar];

				float Amt = Item.Attack_Charge();
				if (Amt == 0)
					continue;

				ChargeBar.setVisible(bShowHealth);

				//Old system
				/* 
			float Level1Amt = GET_CHARGE_FROM_TIME( 1 ); 
			float Level2Amt = GET_CHARGE_FROM_TIME( 2 ); 
			float Level3Amt = GET_CHARGE_FROM_TIME( 3 ); 

			if( Amt <= Level1Amt ) 
			{ 
				ChargeBar.SetFGColorRGB( Color_Charge_Lvl1 ); 
				//ChargeBar.SetBGColorRGB( Color_Transparent ); 
			} 
			else if( Amt <= Level2Amt ) 
			{ 
				ChargeBar.SetFGColorRGB( Color_Charge_Lvl2 ); 
				//ChargeBar.SetBGColorRGB( Color_Charge_Lvl1 ); 
				Amt -= Level1Amt; 
				Amt /= (Level2Amt - Level1Amt); 
			} 
			else if( Amt <= Level3Amt ) 
			{ 
				ChargeBar.SetFGColorRGB( Color_Charge_Lvl3 ); 
				//ChargeBar.SetBGColorRGB( Color_Charge_Lvl1 ); 
				Amt -= Level2Amt; 
				Amt /= (Level3Amt - Level2Amt); 
			}*/
				//[/Old system]

				//[New System] (MiB NOV2007a)
				bool notDone = true;

				int Level = 1;
				int r = 0;
				int g = 0;
				int b = 0;
				int h = 128;
				while (notDone)
				{

					float LevelAmt = GET_CHARGE_FROM_TIME(Level);

					if (Amt <= LevelAmt)
					{
						notDone = false;
						ChargeBar.SetFGColorRGB(COLOR(r, g, b, h));

						if (Level != 1)
						{
							/* From 
						Amt -= Level1Amt; 
						Amt /= (Level2Amt - Level1Amt); 

						for Level2, and the same number functions in 3 

						1. Amt -= Previous level 
						2. Amt /= (This level - Previous level) 
						*/
							Amt -= GET_CHARGE_FROM_TIME(Level - 1);
							Amt /= (GET_CHARGE_FROM_TIME(Level) - GET_CHARGE_FROM_TIME(Level - 1));
						}
					}

					r += 100;
					if (r > 255)
					{
						r -= 255;
						g += 100;
						if (g > 255)
						{
							g -= 255;
							b += 100;
							if (b > 255)
								b -= 255;
						}
					}

					Level++;
				}
				//[/New System]

				ChargeBar.Set(Amt * 100);
			}
		}
	}
};