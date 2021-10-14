#ifndef STACKSTRING_H
#define STACKSTRING_H

#define clrmem(a) memset(&a, 0, sizeof(a));

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef _WIN32
extern "C" char *strlwr(char *str);
#endif

//Deuplicated from msdebug.h
#ifdef NOT_HLDLL
#define msnew new
#elif DEV_BUILD
void *operator new(size_t size, const char *pszSourceFile, int LineNum);
void operator delete(void *ptr, const char *pszSourceFile, int LineNum);
#define msnew new (__FILE__, __LINE__)
#else
#define msnew new
#endif

//mslist - List of items of type itemtype_y
//-Dogg
template <class itemtype_y>
class mslist
{
private:
	itemtype_y *m_First;
	size_t m_Items;
	size_t m_ItemsAllocated;

public:
	mslist()
	{
		m_Items = 0;
		m_ItemsAllocated = 0;
		m_First = (itemtype_y *)0;
	}
	mslist(bool NoInit) {} //Special case - no initialization
	~mslist()
	{
		clear();
	}
	inline itemtype_y &push_back(const itemtype_y &Item) { return add(Item); }
	itemtype_y &add(const itemtype_y &Item)
	{
		reserve(m_Items + 1);

		int idx = m_Items;
		m_First[idx] = Item;

		m_Items++;

		return m_First[idx];
	}
	itemtype_y &add_blank() //Add an entry that's completely zero'd out
	{
		reserve(m_Items + 1);

		int idx = m_Items;
		memset(&m_First[idx], 0, sizeof(itemtype_y));

		m_Items++;

		return m_First[idx];
	}
	itemtype_y &operator[](const int idx) const
	{
		return m_First[idx];
	}
	void erase(const size_t idx)
	{
		if (idx + 1 < m_Items)
			memmove(&m_First[idx], &m_First[idx + 1], (m_Items - (idx + 1)) * sizeof(itemtype_y));

		m_Items--;
	}
	void clear()
	{
		unalloc();
		m_Items = m_ItemsAllocated = 0;
		m_First = (itemtype_y *)0;
	}
	void clearitems() //No slow dealloc.  Just empty out the items
	{
		m_Items = 0;
	}
	size_t size() const
	{
		return m_Items;
	}
	void reserve()
	{
		if (!m_ItemsAllocated)
			m_ItemsAllocated++;
		else
			m_ItemsAllocated *= 2;
		itemtype_y *pNewItems = ::msnew itemtype_y[m_ItemsAllocated];
		for (unsigned int i = 0; i < m_Items; i++)
			pNewItems[i] = m_First[i];
		unalloc();
		m_First = pNewItems;
	}
	void unalloc()
	{
		if (m_First)
			delete[] m_First;
		m_First = (itemtype_y *)0;
	}
	void reserve(size_t Items)
	{
		while (m_ItemsAllocated < Items)
			reserve();
	}
	void reserve_once(size_t ReserveItems, size_t Items) //Special case - only use if you know what you're doing
	{
		unalloc();
		m_ItemsAllocated = ReserveItems;
		m_First = msnew itemtype_y[m_ItemsAllocated];
		m_Items = Items;
	}
	mslist &operator=(const mslist &OtherList)
	{
		if (size() != OtherList.size())
		{
			clear();
			for (unsigned int i = 0; i < OtherList.size(); i++)
				add(OtherList[i]);
		}
		else //Don't re-allocate if I don't have to
			for (unsigned int i = 0; i < OtherList.size(); i++)
				m_First[i] = OtherList[i];

		return *this;
	}
	itemtype_y *FirstItem()
	{
		return m_First;
	}
};

#define vector mslist

//mstring - A fast string with features... no dynamic allocation
//-Dogg
#define MSSTRING_SIZE 256
#define msstring_error ((size_t)-1)

typedef const char *msstring_ref;

class msstring
{
public:
	msstring();
	msstring(const msstring_ref a);
	msstring(const msstring_ref a, size_t length);
	msstring(const msstring &a);
	msstring &operator=(const msstring_ref a);
	msstring &operator=(int a);
	msstring &operator=(const msstring &a);
	msstring &operator+=(const msstring_ref a);
	msstring &operator+=(int a);
	msstring operator+(const msstring_ref a);
	msstring operator+(msstring &a);
	msstring operator+(int a);
	bool operator==(char *a) const;
	bool operator==(const char *a) const;
	bool operator!=(char *a) const { return !operator==(a); }
	bool operator!=(const char *a) const { return !operator==(a); }
	operator char *();
	operator void *() { return operator char *(); }
	char *c_str();
	size_t len() const;
	void append(const msstring_ref a);
	void append(const msstring_ref a, size_t length);
	size_t find(const msstring_ref a, size_t start = 0) const;				 //Returns position of the string "a"
	msstring_ref find_str(const msstring_ref a, size_t start = 0) const;	 //Returns a substring starting at "find(a,start)". Returns full string if "a" not found
	size_t findchar(const msstring_ref a, size_t start = 0) const;			 //Returns position of the first char within "a"
	msstring_ref findchar_str(const msstring_ref a, size_t start = 0) const; //Returns a substring starting at "findchar(a,start)". Returns full string if text didn't contain any chars from "a"
	bool contains(const msstring_ref a) const;								 //Reutrns true if substring "a" is contained within the main string
	bool starts_with(const msstring_ref a) const;							 //Reutrns true if the main string starts with "a"
	bool ends_with(const msstring_ref a) const;								 //MIB FEB2008a returns true if last character is "a"
	msstring substr(size_t start, size_t length);
	msstring substr(size_t start);
	msstring thru_substr(const msstring_ref a, size_t start = 0) const; //Returns a substring spanning from "start" to "find(a,start)". Returns full string if "find(a,start)" not found
	msstring thru_char(const msstring_ref a, size_t start = 0) const;	//Returns a substring spanning from "start" to "findchar(a,start)". Returns full string if "findchar(a,start)" not found
	msstring skip(const msstring_ref a) const;							//Returns a substring starting at the first char that isn't within "a"

	// These all need to be primes!
	static const int PRIME_A = 54059;
	static const int PRIME_B = 76963;
	static const int PRIME_INIT = 31;
	int hashCode()
	{
		int h = PRIME_INIT;
		for (unsigned int i = 0; i < len(); i++)
		{
			h = (h * PRIME_A) ^ (data[i] * PRIME_B);
		}
		return h < 0 ? -h : h;
	}
	msstring toString() { return *this; }

protected:
	char data[MSSTRING_SIZE];
	//char *data;
};

/*class str256 : public msstring
{
public:
	str256( ) { init( ); }
	str256( const msstring_ref a ) { init( ); msstring::operator = ( a ); }
	str256( const msstring_ref a, size_t length ) { init( ); append( a, length ); }
	str256( const msstring &a ) { init( ); msstring::operator = ( a ); }
	
	void init( ) { data = m_Data; }
	char m_Data[256];
};
#define msstring str256*/

typedef mslist<msstring> msstringlist;

bool TokenizeString(msstring_ref pszString, msstringlist &Tokens, msstring_ref Separator);
inline bool TokenizeString(msstring_ref pszString, msstringlist &Tokens) { return TokenizeString(pszString, Tokens, ";"); }
void ReplaceChar(char *pString, char org, char dest);

//It's an int, a float, and a string
class msvariant
{
public:
	msstring m_String;
	int m_Int;
	float m_Float;
	enum type
	{
		INT,
		FLOAT,
		STRING
	} m_Type;

	msvariant();
	msvariant(int a) { operator=(a); }
	msvariant(float a) { operator=(a); }
	msvariant(msstring_ref a) { operator=(a); }
	msvariant &operator=(msstring_ref a)
	{
		SetFromString(a);
		m_Type = STRING;
		return *this;
	}
	msvariant &operator=(int a)
	{
		SetFromInt(a);
		m_Type = INT;
		return *this;
	}
	msvariant &operator=(float a)
	{
		SetFromFloat(a);
		m_Type = STRING;
		return *this;
	}

	void SetFromString(msstring_ref a);
	void SetFromInt(int a);
	void SetFromFloat(float a);
	void SetType(type Type) { m_Type = Type; }

	operator int() { return m_Int; }
	operator float() { return m_Float; }
	operator msstring_ref() { return m_String.c_str(); }
};

//MiB NOV2014_19 - individualized trigger cooldown [begin]
// TODO: Remove this and update references to use mshash instead
template <class itemtype_key, class itemtype_val>
class mshashentry
{
private:
	itemtype_key mKey;
	itemtype_val mVal;

public:
	mshashentry(){};
	mshashentry(const itemtype_key rKey, const itemtype_val rVal)
	{
		mKey = rKey;
		mVal = rVal;
	}

	mshashentry &operator=(mshashentry a)
	{
		mKey = a.mKey;
		mVal = a.mVal;
		return *this;
	}

	const itemtype_key GetKey(void)
	{
		return mKey;
	}

	const itemtype_val GetVal(void)
	{
		return mVal;
	}

	void SetVal(const itemtype_val rVal)
	{
		mVal = rVal;
	}
};
//MiB NOV2014_19 - individualized trigger cooldown [end]

// MiB 30NOV_2014 List of 1000 primes
#define MAX_HASH_INT 1000
static const int HashPrimes[MAX_HASH_INT] =
	{
		11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89,
		97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173,
		179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263,
		269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 353, 359,
		367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439, 443, 449, 457,
		461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541, 547, 557, 563, 569,
		571, 577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641, 643, 647, 653, 659,
		661, 673, 677, 683, 691, 701, 709, 719, 727, 733, 739, 743, 751, 757, 761, 769,
		773, 787, 797, 809, 811, 821, 823, 827, 829, 839, 853, 857, 859, 863, 877, 881,
		883, 887, 907, 911, 919, 929, 937, 941, 947, 953, 967, 971, 977, 983, 991, 997,
		1009, 1013, 1019, 1021, 1031, 1033, 1039, 1049, 1051, 1061, 1063, 1069, 1087,
		1091, 1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153, 1163, 1171, 1181,
		1187, 1193, 1201, 1213, 1217, 1223, 1229, 1231, 1237, 1249, 1259, 1277, 1279,
		1283, 1289, 1291, 1297, 1301, 1303, 1307, 1319, 1321, 1327, 1361, 1367, 1373,
		1381, 1399, 1409, 1423, 1427, 1429, 1433, 1439, 1447, 1451, 1453, 1459, 1471,
		1481, 1483, 1487, 1489, 1493, 1499, 1511, 1523, 1531, 1543, 1549, 1553, 1559,
		1567, 1571, 1579, 1583, 1597, 1601, 1607, 1609, 1613, 1619, 1621, 1627, 1637,
		1657, 1663, 1667, 1669, 1693, 1697, 1699, 1709, 1721, 1723, 1733, 1741, 1747,
		1753, 1759, 1777, 1783, 1787, 1789, 1801, 1811, 1823, 1831, 1847, 1861, 1867,
		1871, 1873, 1877, 1879, 1889, 1901, 1907, 1913, 1931, 1933, 1949, 1951, 1973,
		1979, 1987, 1993, 1997, 1999, 2003, 2011, 2017, 2027, 2029, 2039, 2053, 2063,
		2069, 2081, 2083, 2087, 2089, 2099, 2111, 2113, 2129, 2131, 2137, 2141, 2143,
		2153, 2161, 2179, 2203, 2207, 2213, 2221, 2237, 2239, 2243, 2251, 2267, 2269,
		2273, 2281, 2287, 2293, 2297, 2309, 2311, 2333, 2339, 2341, 2347, 2351, 2357,
		2371, 2377, 2381, 2383, 2389, 2393, 2399, 2411, 2417, 2423, 2437, 2441, 2447,
		2459, 2467, 2473, 2477, 2503, 2521, 2531, 2539, 2543, 2549, 2551, 2557, 2579,
		2591, 2593, 2609, 2617, 2621, 2633, 2647, 2657, 2659, 2663, 2671, 2677, 2683,
		2687, 2689, 2693, 2699, 2707, 2711, 2713, 2719, 2729, 2731, 2741, 2749, 2753,
		2767, 2777, 2789, 2791, 2797, 2801, 2803, 2819, 2833, 2837, 2843, 2851, 2857,
		2861, 2879, 2887, 2897, 2903, 2909, 2917, 2927, 2939, 2953, 2957, 2963, 2969,
		2971, 2999, 3001, 3011, 3019, 3023, 3037, 3041, 3049, 3061, 3067, 3079, 3083,
		3089, 3109, 3119, 3121, 3137, 3163, 3167, 3169, 3181, 3187, 3191, 3203, 3209,
		3217, 3221, 3229, 3251, 3253, 3257, 3259, 3271, 3299, 3301, 3307, 3313, 3319,
		3323, 3329, 3331, 3343, 3347, 3359, 3361, 3371, 3373, 3389, 3391, 3407, 3413,
		3433, 3449, 3457, 3461, 3463, 3467, 3469, 3491, 3499, 3511, 3517, 3527, 3529,
		3533, 3539, 3541, 3547, 3557, 3559, 3571, 3581, 3583, 3593, 3607, 3613, 3617,
		3623, 3631, 3637, 3643, 3659, 3671, 3673, 3677, 3691, 3697, 3701, 3709, 3719,
		3727, 3733, 3739, 3761, 3767, 3769, 3779, 3793, 3797, 3803, 3821, 3823, 3833,
		3847, 3851, 3853, 3863, 3877, 3881, 3889, 3907, 3911, 3917, 3919, 3923, 3929,
		3931, 3943, 3947, 3967, 3989, 4001, 4003, 4007, 4013, 4019, 4021, 4027, 4049,
		4051, 4057, 4073, 4079, 4091, 4093, 4099, 4111, 4127, 4129, 4133, 4139, 4153,
		4157, 4159, 4177, 4201, 4211, 4217, 4219, 4229, 4231, 4241, 4243, 4253, 4259,
		4261, 4271, 4273, 4283, 4289, 4297, 4327, 4337, 4339, 4349, 4357, 4363, 4373,
		4391, 4397, 4409, 4421, 4423, 4441, 4447, 4451, 4457, 4463, 4481, 4483, 4493,
		4507, 4513, 4517, 4519, 4523, 4547, 4549, 4561, 4567, 4583, 4591, 4597, 4603,
		4621, 4637, 4639, 4643, 4649, 4651, 4657, 4663, 4673, 4679, 4691, 4703, 4721,
		4723, 4729, 4733, 4751, 4759, 4783, 4787, 4789, 4793, 4799, 4801, 4813, 4817,
		4831, 4861, 4871, 4877, 4889, 4903, 4909, 4919, 4931, 4933, 4937, 4943, 4951,
		4957, 4967, 4969, 4973, 4987, 4993, 4999, 5003, 5009, 5011, 5021, 5023, 5039,
		5051, 5059, 5077, 5081, 5087, 5099, 5101, 5107, 5113, 5119, 5147, 5153, 5167,
		5171, 5179, 5189, 5197, 5209, 5227, 5231, 5233, 5237, 5261, 5273, 5279, 5281,
		5297, 5303, 5309, 5323, 5333, 5347, 5351, 5381, 5387, 5393, 5399, 5407, 5413,
		5417, 5419, 5431, 5437, 5441, 5443, 5449, 5471, 5477, 5479, 5483, 5501, 5503,
		5507, 5519, 5521, 5527, 5531, 5557, 5563, 5569, 5573, 5581, 5591, 5623, 5639,
		5641, 5647, 5651, 5653, 5657, 5659, 5669, 5683, 5689, 5693, 5701, 5711, 5717,
		5737, 5741, 5743, 5749, 5779, 5783, 5791, 5801, 5807, 5813, 5821, 5827, 5839,
		5843, 5849, 5851, 5857, 5861, 5867, 5869, 5879, 5881, 5897, 5903, 5923, 5927,
		5939, 5953, 5981, 5987, 6007, 6011, 6029, 6037, 6043, 6047, 6053, 6067, 6073,
		6079, 6089, 6091, 6101, 6113, 6121, 6131, 6133, 6143, 6151, 6163, 6173, 6197,
		6199, 6203, 6211, 6217, 6221, 6229, 6247, 6257, 6263, 6269, 6271, 6277, 6287,
		6299, 6301, 6311, 6317, 6323, 6329, 6337, 6343, 6353, 6359, 6361, 6367, 6373,
		6379, 6389, 6397, 6421, 6427, 6449, 6451, 6469, 6473, 6481, 6491, 6521, 6529,
		6547, 6551, 6553, 6563, 6569, 6571, 6577, 6581, 6599, 6607, 6619, 6637, 6653,
		6659, 6661, 6673, 6679, 6689, 6691, 6701, 6703, 6709, 6719, 6733, 6737, 6761,
		6763, 6779, 6781, 6791, 6793, 6803, 6823, 6827, 6829, 6833, 6841, 6857, 6863,
		6869, 6871, 6883, 6899, 6907, 6911, 6917, 6947, 6949, 6959, 6961, 6967, 6971,
		6977, 6983, 6991, 6997, 7001, 7013, 7019, 7027, 7039, 7043, 7057, 7069, 7079,
		7103, 7109, 7121, 7127, 7129, 7151, 7159, 7177, 7187, 7193, 7207, 7211, 7213,
		7219, 7229, 7237, 7243, 7247, 7253, 7283, 7297, 7307, 7309, 7321, 7331, 7333,
		7349, 7351, 7369, 7393, 7411, 7417, 7433, 7451, 7457, 7459, 7477, 7481, 7487,
		7489, 7499, 7507, 7517, 7523, 7529, 7537, 7541, 7547, 7549, 7559, 7561, 7573,
		7577, 7583, 7589, 7591, 7603, 7607, 7621, 7639, 7643, 7649, 7669, 7673, 7681,
		7687, 7691, 7699, 7703, 7717, 7723, 7727, 7741, 7753, 7757, 7759, 7789, 7793,
		7817, 7823, 7829, 7841, 7853, 7867, 7873, 7877, 7879, 7883, 7901, 7907, 7919,
		7927, 7933, 7937, 7949};

// MiB 30NOV_2014
// NOTE: If you want a data type to work as a key, it must have a function:
//		 int hashCode( );
//		 msstring toString( );
template <typename K, typename V>
class mshash
{
	/*
		TODO:
		-Other things I can't think of now
	*/
public:
	// Structure containing key and value
	struct entry_t
	{
		entry_t(K k, V v)
		{
			key = k;
			val = v;
		}
		entry_t(){};
		V &GetVal() { return val; }
		K &GetKey() { return key; }
		void SetVal(V v) { val = v; }
		entry_t &operator=(const entry_t &other)
		{
			key = other.key;
			val = other.val;
			return *this;
		}

	private:
		K key;
		V val;
	};

	// Structure containing a non-dynamic array of 10 entries
	struct bucket_t
	{
		bucket_t() { sz = 0; }
		bool put(K k, V v)
		{
			int l = locate(k);
			if (l > -1)
			{
				b[l].SetVal(v);
				return true;
			}

			if (sz >= MAX)
				return false;
			b[sz++] = entry_t(k, v);
			return true;
		}

		bool erase(K k)
		{
			int l = locate(k);
			if (l > -1)
			{
				--sz;
				if (l < MAX - 1)
					memcpy(&(b[l]), &(b[l + 1]), sizeof(b[l]) * (MAX - sz - 1));
				return true;
			}
			else
				return false;
		}
		int getSize() { return sz; }
		entry_t &get(int i)
		{
			bool b;
			return get(i, b);
		}
		entry_t &get(int i, bool &bFound)
		{
			if (i >= sz)
			{
				bFound = false;
				static entry_t bad;
				return bad;
			}
			bFound = true;
			return b[i];
		}
		entry_t &operator[](int i) { return get(i); }

	private:
		int locate(K k)
		{
			for (int i = 0; i < sz; i++)
			{
				if (b[i].GetKey() == k)
					return i;
			}
			return -1;
		}
		const static int MAX = 10;
		entry_t b[MAX];
		int sz;
	};

	mshash()
	{
		dropTable();
		Grow(HashPrimes[0]);
		numElements = 0;
	}
	mshash(int PRIME_NUMBER_TBL_SIZE)
	{
		dropTable();
		Grow(PRIME_NUMBER_TBL_SIZE);
		numElements = 0;
	}
	~mshash() { dropTable(); }

	void put(K k, V v)
	{
		while (!tablePut(k, v) && TBL_SIZE > -1)
			Grow();
		numElements++;
	}
	void erase(K k)
	{
		if (tableErase(k))
			--numElements;
	}

	bool hasKey(K k)
	{
		bool found;
		getFull(k, found);
		return found;
	}
	V &operator[](K k) { return get(k); }
	V &get(K k) const
	{
		bool found;
		return get(k, found);
	}
	V &get(K k, bool &bFound) const { return getFull(k, bFound).GetVal(); }
	entry_t &getFull(K k) const
	{
		bool found;
		return getFull(k, found);
	}
	entry_t &getFull(K k, bool &bFound) const
	{
		int h = hashCode(k);
		bFound = false;
		for (int i = 0; i < tbl[h].getSize(); i++)
		{
			if (tbl[h].get(i).GetKey() == k)
			{
				bFound = true;
				return tbl[h].get(i);
			}
		}
		static entry_t bad;
		return bad;
	}

	bucket_t *getBucketList() const { return tbl; }
	int getTableSize() const { return TBL_SIZE; }
	bool empty() const { return !numElements; }

	void diagnostic()
	{
		Print("Hash Information:\n");
		int mtBuckets = 0;
		int flstBucket = 0;
		for (int i = 0; i < TBL_SIZE; i++)
		{
			Print("\tBucket %d:\n", i);
			if (!tbl[i].getSize())
			{
				++mtBuckets;
			}
			else if (tbl[i].getSize() > tbl[flstBucket].getSize())
			{
				flstBucket = i;
			}

			for (int j = 0; j < tbl[i].getSize(); j++)
			{
				msstring key = tbl[i].get(j).GetKey();
				Print("\t\tSlot %d: Key: %s\n\t\t\t%s\n", j, key.c_str(), tbl[i].get(j).GetVal().toString().c_str() // If you're getting an error here, your Value class needs a .toString() function
				);
			}
		}
		Print("Total Elements: %d\n", numElements);
		Print("Number of Buckets: %d\n", TBL_SIZE);
		Print("Average Used Bucket Size: %.2f\n", numElements / ((float)TBL_SIZE - mtBuckets));
		Print("Fullest Bucket: %d (Bucket %d)\n", tbl[flstBucket].getSize(), flstBucket);
		Print("Number of Empty Buckets: %d\n", mtBuckets);
	}

private:
	// Data
	bucket_t *tbl;
	int TBL_SIZE;
	int numElements;

	// Members
	bool Grow() { return Grow(nextPrime()); }
	bool Grow(int size)
	{
		if (size == -1)
			return false;

		bucket_t *new_tbl = msnew bucket_t[size];

		if (tbl)
		{
			for (int i = 0; i < TBL_SIZE; i++)
			{
				bucket_t bucket = tbl[i];
				for (int j = 0; j < bucket.getSize(); j++)
				{
					if (!tablePut(bucket[j].GetKey(), bucket[j].GetVal(), new_tbl, size))
					{
						delete[] new_tbl;
						return Grow(nextPrime(size));
					}
				}
			}
			delete[] tbl;
		}

		TBL_SIZE = size;
		tbl = new_tbl;

		return true;
	}

	bool tableErase(K k) { return tableErase(k, tbl, TBL_SIZE); }
	bool tableErase(K k, bucket_t *t, int sz)
	{
		int h = hashCode(k, sz);
		return t[h].erase(k);
	}
	bool tablePut(K k, V v) { return tablePut(k, v, tbl, TBL_SIZE); }
	bool tablePut(K k, V v, bucket_t *t, int sz)
	{
		int h = hashCode(k, sz);
		return t[h].put(k, v);
	}
	void dropTable()
	{
		if (tbl)
			delete[] tbl;
		tbl = (bucket_t *)0;
	}
	int hashCode(K k, int sz) const { return k.hashCode() % sz; } // If you're getting an error here, your key class needs a hashCode function to use mshash
	int hashCode(K k) const { return hashCode(k, TBL_SIZE); }
	int nextPrime(void) const { return nextPrime(TBL_SIZE); }
	static int nextPrime(int prime)
	{
		for (int i = 0; i < (MAX_HASH_INT - 1); i++)
		{
			if (HashPrimes[i] == prime)
				return HashPrimes[i + 1];
			if (HashPrimes[i] > prime)
				return HashPrimes[i];
		}
		return -1;
	}
};

#endif STACKSTRING_H