/***
*
*	Copyright (c) 1999, 2000 Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
//=========================================================
// nodes.h
//=========================================================

#ifndef NODES_H
#define NODES_H

class CFile;

//=========================================================
// DEFINE
//=========================================================

constexpr unsigned int MAX_STACK_NODES = 100;
constexpr unsigned int MAX_NODE_HULLS = 4;
constexpr unsigned int CACHE_SIZE = 128;
constexpr unsigned int NUM_RANGES = 256;
constexpr unsigned MAX_NODE_INITIAL_LINKS = 128;
constexpr unsigned MAX_NODES = 1024;
constexpr float HULL_STEP_SIZE = 16; // how far the test hull moves on each step
constexpr float NODE_HEIGHT = 8;	  // how high to lift nodes off the ground after we drop them all (make stair/ramp mapping easier)
constexpr unsigned int NUMBER_OF_PRIMES = 177;
constexpr int NO_NODE = -1;
constexpr int UNNUMBERED_NODE = -1;
constexpr int ENTRY_STATE_EMPTY = -1;

constexpr unsigned int Primes[NUMBER_OF_PRIMES] =
{ 1, 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67,
 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151,
 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239,
 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337,
 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433,
 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541,
 547, 557, 563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641,
 643, 647, 653, 659, 661, 673, 677, 683, 691, 701, 709, 719, 727, 733, 739, 743,
 751, 757, 761, 769, 773, 787, 797, 809, 811, 821, 823, 827, 829, 839, 853, 857,
 859, 863, 877, 881, 883, 887, 907, 911, 919, 929, 937, 941, 947, 953, 967, 971,
 977, 983, 991, 997, 1009, 1013, 1019, 1021, 1031, 1033, 1039, 0 };

struct tagNodePair
{
	short iSrc;
	short iDest;
};

enum bits_node_type_e {

	bits_NODE_LAND = (1 << 0),	 // Land node, so nudge if necessary.
	bits_NODE_AIR = (1 << 1),	 // Air node, don't nudge.
	bits_NODE_WATER = (1 << 2), // Water node, don't nudge.
	bits_NODE_GROUP_REALM = (bits_NODE_LAND | bits_NODE_AIR | bits_NODE_WATER)

};
//=========================================================
// Instance of a node.
//=========================================================
class CNode
{
public:
	Vector m_vecOrigin;		// location of this node in space
	Vector m_vecOriginPeek; // location of this node (LAND nodes are NODE_HEIGHT higher).
	byte m_Region[3];		// Which of 256 regions do each of the coordinate belong?
	int m_afNodeInfo;		// bits that tell us more about this location

	unsigned int m_cNumLinks;  // how many links this node has
	unsigned int m_iFirstLink; // index of this node's first link in the link pool.

	// Where to start looking in the compressed routing table (offset into m_pRouteInfo).
	// (4 hull sizes -- smallest to largest + fly/swim), and secondly, door capability.
	//
	int m_pNextBestNode[MAX_NODE_HULLS][2];

	// Used in finding the shortest path. m_fClosestSoFar is -1 if not visited.
	// Then it is the distance to the source. If another path uses this node
	// and has a closer distance, then m_iPreviousNode is also updated.
	//
	float m_flClosestSoFar; // Used in finding the shortest path.
	int m_iPreviousNode;

	short m_sHintType;	   // there is something interesting in the world at this node's position
	short m_sHintActivity; // there is something interesting in the world at this node's position
	float m_flHintYaw;	   // monster on this node should face this yaw to face the hint.
};

//=========================================================
// CLink - A link between 2 nodes
//=========================================================

enum bits_clink_e {
	bits_LINK_SMALL_HULL = (1 << 0), // headcrab box can fit through this connection
	bits_LINK_HUMAN_HULL = (1 << 1), // player box can fit through this connection
	bits_LINK_LARGE_HULL = (1 << 2), // big box can fit through this connection
	bits_LINK_FLY_HULL = (1 << 3),	  // a flying big box can fit through this connection
	bits_LINK_DISABLED = (1 << 4)	  // link is not valid when the set
};

enum node_hull_e {

	NODE_SMALL_HULL = 0,
	NODE_HUMAN_HULL = 1,
	NODE_LARGE_HULL = 2,
	NODE_FLY_HULL = 3

};
class CLink
{
public:
	int m_iSrcNode;	 // the node that 'owns' this link ( keeps us from having to make reverse lookups )
	int m_iDestNode; // the node on the other end of the link.

	entvars_t *m_pLinkEnt; // the entity that blocks this connection (doors, etc)

	// m_szLinkEntModelname is not necessarily NULL terminated (so we can store it in a more alignment-friendly 4 bytes)
	char m_szLinkEntModelname[4]; // the unique name of the brush model that blocks the connection (this is kept for save/restore)

	int m_afLinkInfo; // information about this link
	float m_flWeight; // length of the link line segment
};

typedef struct
{
	int m_SortedBy[3];
	int m_CheckedEvent;
} DIST_INFO;

typedef struct
{
	Vector v;
	short n; // Nearest node or -1 if no node found.
} CACHE_ENTRY;

//=========================================================
// CGraph
//=========================================================
#define GRAPH_VERSION (int)16 // !!!increment this whever graph/node/link classes change, to obsolesce older disk files.
class CGraph
{
public:
	// the graph has two flags, and should not be accessed unless both flags are TRUE!
	BOOL m_fGraphPresent;	  // is the graph in memory?
	BOOL m_fGraphPointersSet; // are the entity pointers for the graph all set?
	BOOL m_fRoutingComplete;  // are the optimal routes computed, yet?

	CNode *m_pNodes;	// pointer to the memory block that contains all node info
	CLink *m_pLinkPool; // big list of all node connections
	char *m_pRouteInfo; // compressed routing information the nodes use.

	unsigned int m_cNodes;	  // total number of nodes
	unsigned int m_cLinks;	  // total number of links
	unsigned int m_nRouteInfo; // size of m_pRouteInfo in bytes.

	// Tables for making nearest node lookup faster. SortedBy provided nodes in a
	// order of a particular coordinate. Instead of doing a binary search, RangeStart
	// and RangeEnd let you get to the part of SortedBy that you are interested in.
	//
	// Once you have a point of interest, the only way you'll find a closer point is
	// if at least one of the coordinates is closer than the ones you have now. So we
	// search each range. After the search is exhausted, we know we have the closest
	// node.
	//

	DIST_INFO *m_di; // This is m_cNodes long, but the entries don't correspond to CNode entries.
	unsigned int m_RangeStart[3][NUM_RANGES];
	unsigned int m_RangeEnd[3][NUM_RANGES];
	float m_flShortest;
	int m_iNearest;
	int m_minX, m_minY, m_minZ, m_maxX, m_maxY, m_maxZ;
	int m_minBoxX, m_minBoxY, m_minBoxZ, m_maxBoxX, m_maxBoxY, m_maxBoxZ;
	int m_CheckedCounter;
	float m_RegionMin[3], m_RegionMax[3]; // The range of nodes.
	CACHE_ENTRY m_Cache[CACHE_SIZE];

	int m_HashPrimes[16];
	short *m_pHashLinks;
	unsigned int m_nHashLinks;

	// kinda sleazy. In order to allow variety in active idles for monster groups in a room with more than one node,
	// we keep track of the last node we searched from and store it here. Subsequent searches by other monsters will pick
	// up where the last search stopped.
	unsigned int m_iLastActiveIdleSearch;

	// another such system used to track the search for cover nodes, helps greatly with two monsters trying to get to the same node.
	int m_iLastCoverSearch;

	// functions to create the graph
	int LinkVisibleNodes(CLink *pLinkPool, CFile &file, int *piBadNode);
	int RejectInlineLinks(CLink *pLinkPool, CFile &file);
	int FindShortestPath(int *piPath, int iStart, int iDest, int iHull, int afCapMask);
	int FindNearestNode(const Vector &vecOrigin, CBaseEntity *pEntity);
	int FindNearestNode(const Vector &vecOrigin, int afNodeTypes);
	//int		FindNearestLink ( const Vector &vecTestPoint, int *piNearestLink, BOOL *pfAlongLine );
	float PathLength(int iStart, int iDest, int iHull, int afCapMask);
	int NextNodeInRoute(int iCurrentNode, int iDest, int iHull, int iCap);

	enum NODEQUERY
	{
		NODEGRAPH_DYNAMIC,
		NODEGRAPH_STATIC
	};
	// A static query means we're asking about the possiblity of handling this entity at ANY time
	// A dynamic query means we're asking about it RIGHT NOW.  So we should query the current state
	int HandleLinkEnt(int iNode, entvars_t *pevLinkEnt, int afCapMask, NODEQUERY queryType);
	entvars_t *LinkEntForLink(CLink *pLink, CNode *pNode);
	void ShowNodeConnections(int iNode);
	void InitGraph(void);
	int AllocNodes(void);

	int CheckNODFile(const char *szMapName);
	int FLoadGraph(const char *szMapName);
	int FSaveGraph(const char *szMapName);
	int FSetGraphPointers(void);
	void CheckNode(Vector vecOrigin, int iNode);

	void BuildRegionTables(void);
	void ComputeStaticRoutingTables(void);
	void TestRoutingTables(void);

	void HashInsert(int iSrcNode, int iDestNode, int iKey);
	void HashSearch(int iSrcNode, int iDestNode, int &iKey);
	void HashChoosePrimes(int TableSize);
	void BuildLinkLookups(void);

	void SortNodes(void);

	int HullIndex(const CBaseEntity *pEntity); // what hull the monster uses
	int NodeType(const CBaseEntity *pEntity);  // what node type the monster uses
	inline int CapIndex(int afCapMask)
	{
		if (afCapMask & (bits_CAP_OPEN_DOORS | bits_CAP_AUTO_DOORS | bits_CAP_USE))
			return 1;
		return 0;
	}

	inline CNode &Node(int i)
	{
#ifdef _DEBUG
		if (!m_pNodes || i < 0 || i > m_cNodes)
			ALERT(at_error, "Bad Node!\n");
#endif
		return m_pNodes[i];
	}

	inline CLink &Link(int i)
	{
#ifdef _DEBUG
		if (!m_pLinkPool || i < 0 || i > m_cLinks)
			ALERT(at_error, "Bad link!\n");
#endif
		return m_pLinkPool[i];
	}

	inline CLink &NodeLink(int iNode, int iLink)
	{
		return Link(Node(iNode).m_iFirstLink + iLink);
	}

	inline CLink &NodeLink(const CNode &node, int iLink)
	{
		return Link(node.m_iFirstLink + iLink);
	}

	inline int INodeLink(int iNode, int iLink)
	{
		return NodeLink(iNode, iLink).m_iDestNode;
	}

#if 0
	inline CNode &SourceNode( int iNode, int iLink )
	{
		return Node( NodeLink( iNode, iLink ).m_iSrcNode );
	}

	inline CNode &DestNode( int iNode, int iLink )
	{
		return Node( NodeLink( iNode, iLink ).m_iDestNode );
	}

	inline	CNode *PNodeLink ( int iNode, int iLink ) 
	{
		return &DestNode( iNode, iLink );
	}
#endif
};

//=========================================================
// Nodes start out as ents in the level. The node graph
// is built, then these ents are discarded.
//=========================================================
class CNodeEnt : public CBaseEntity
{
	void Spawn(void);
	void KeyValue(KeyValueData *pkvd);
	virtual int ObjectCaps(void) { return CBaseEntity ::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	short m_sHintType;
	short m_sHintActivity;
};

//=========================================================
// CStack - last in, first out.
//=========================================================
class CStack
{
public:
	CStack(void);
	void Push(int value);
	int Pop(void);
	int Top(void);
	int Empty(void) { return m_level == 0; }
	int Size(void) { return m_level; }
	void CopyToArray(int *piArray);

private:
	int m_stack[MAX_STACK_NODES];
	int m_level;
};

//=========================================================
// CQueue - first in, first out.
//=========================================================
class CQueue
{
public:
	CQueue(void); // constructor
	inline int Full(void) { return (m_cSize == MAX_STACK_NODES); }
	inline int Empty(void) { return (m_cSize == 0); }
	//inline int Tail ( void ) { return ( m_queue[ m_tail ] ); }
	inline int Size(void) { return (m_cSize); }
	void Insert(int, float);
	int Remove(float &);

private:
	int m_cSize;
	struct tag_QUEUE_NODE
	{
		int Id;
		float Priority;
	} m_queue[MAX_STACK_NODES];
	int m_head;
	int m_tail;
};

//=========================================================
// CQueuePriority - Priority queue (smallest item out first).
//
//=========================================================
class CQueuePriority
{
public:
	CQueuePriority(void); // constructor
	inline int Full(void) { return (m_cSize == MAX_STACK_NODES); }
	inline int Empty(void) { return (m_cSize == 0); }
	//inline int Tail ( float & ) { return ( m_queue[ m_tail ].Id ); }
	inline int Size(void) { return (m_cSize); }
	void Insert(int, float);
	int Remove(float &);

private:
	int m_cSize;
	struct tag_HEAP_NODE
	{
		int Id;
		float Priority;
	} m_heap[MAX_STACK_NODES];
	void Heap_SiftDown(int);
	void Heap_SiftUp(void);
};

//=========================================================
// hints - these MUST coincide with the HINTS listed under
// info_node in the FGD file!
//=========================================================
enum hint_node_e
{
	HINT_NONE = 0,
	HINT_WORLD_DOOR,
	HINT_WORLD_WINDOW,
	HINT_WORLD_BUTTON,
	HINT_WORLD_MACHINERY,
	HINT_WORLD_LEDGE,
	HINT_WORLD_LIGHT_SOURCE,
	HINT_WORLD_HEAT_SOURCE,
	HINT_WORLD_BLINKING_LIGHT,
	HINT_WORLD_BRIGHT_COLORS,
	HINT_WORLD_HUMAN_BLOOD,
	HINT_WORLD_ALIEN_BLOOD,

	HINT_TACTICAL_EXIT = 100,
	HINT_TACTICAL_VANTAGE,
	HINT_TACTICAL_AMBUSH,

	HINT_STUKA_PERCH = 300,
	HINT_STUKA_LANDING,
};

extern CGraph WorldGraph;

#endif //NODES_H