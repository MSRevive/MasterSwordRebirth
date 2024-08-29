struct title_t
{
	msstring Name;		   //Title name
	int MinLevel;		   //Each required skill must be higher than this
	mslist<int> SkillsReq; //If these skills are the highest player skills, then use this title
};

class CTitleManager
{
public:
	static void AddTitle(title_t &Title);
	static void DeleteAllTitles();
	static title_t *GetPlayerTitle(CBasePlayer *pPlayer);
	static mslist<title_t> Titles;
	static title_t DefaultTitle;
};
