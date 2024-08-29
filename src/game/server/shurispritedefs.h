//Shuriken FEB2008a Sprite crap
#define INV_SPRITE_NUM 77
#define INV_SPRITE "allitems"

const char *SpriteArray[INV_SPRITE_NUM] = {
	"apple",
	"armor1",
	"armor2",
	"armor3",
	"axe",
	"backsheath",
	"battleaxe",
	"boarskin",
	"book",
	"bpot",
	"breastplate",
	"broadarrow",
	"chainmail",
	"crafted",
	"crestedana",
	"dragonsword",
	"dreadscythe",
	"ebook",
	"expbolt",
	"firearrow",
	"firemagic",
	"firemana",
	"gauntlets",
	"gold",
	"gpot",
	"greataxe",
	"hammer",
	"helm1",
	"helm2",
	"helm3",
	"hugger",
	"hvybackpack",
	"iceblade",
	"ironshield",
	"katana",
	"key",
	"leather",
	"letter",
	"log",
	"longbow",
	"longsword",
	"lostblade",
	"mace",
	"machete",
	"maul",
	"merldagger",
	"mhealth",
	"orcbow",
	"orionbow",
	"orionsword",
	"package",
	"platehelm",
	"plateleggins",
	"quiver",
	"ratpelt",
	"rdagger",
	"ring",
	"ringmail",
	"runeaxe",
	"runeshield",
	"rustyhammer",
	"scythe",
	"sheath1",
	"sheath2",
	"shortsword",
	"silverarrow",
	"skullblade",
	"smallaxe",
	"torch",
	"treebow",
	"warhammer",
	"watermagic",
	"watermana",
	"wclub1",
	"weaponsmithaxe",
	"woodenarrow",
	"xbow"};

//Shuriken FEB2008a to check old sprite names and get their frame
int SpriteIsInArray(const char *SpriteName)
{
	for (int i = 0; i < INV_SPRITE_NUM; i++)
	{
		if (strcmp(SpriteName, SpriteArray[i]) == 0)
		{
			return i;
		}
	}
	return -1;
}