

#ifndef VOTE_H
#define VOTE_H

struct vote_t
{
	bool fActive;		//If a current vote is active
	msstring Type,		//Vote type
		Info;			//Vote parameters
	short TargetPlayer, //Player the vote will affect
		SourcePlayer;	//Player that initiated the vote
	msstring TargetPlayerName,
		TargetPlayerWonID;
	float EndTime;
	unsigned long VoteTally;
	msstring Title, Desc;
	static msstringlist VotesTypes;		   //All The vote types
	static msstringlist VotesTypesAllowed; //The vote types that are allowed
};



#endif //VOTE_H