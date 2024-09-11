#define MAX_VOTE_PLAYERS 32
enum option_e
{
	OPTIONSC_VOTEKICK,
	OPTIONSC_VOTETIME,
	OPTIONSC_PARTY
};

class CPanel_Options : public CTransparentPanel
{
private:
	class CInitVotePanel *m_pInitVote;
	class CCastVotePanel *m_pCastVote;
	class CPartyPanel *m_pPartyPanel;
	class CPartyLeavePanel *m_pPartyLeavePanel;
	option_e m_OpenScreen;

public:
	CPanel_Options(Panel *myParent);

	virtual bool SlotInput(int iSlot);
	virtual void Open(option_e OptionScreen);
	virtual void Reset(void);
	virtual void Initialize(void);
};
