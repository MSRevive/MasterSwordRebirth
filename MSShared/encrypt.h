typedef unsigned char byte;

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

class CEncryptBase
{
protected:
	byte *m_pData;
	size_t m_DataSize;

public:
	CEncryptBase() { m_pData = NULL; }
	CEncryptBase(const byte pData[], const size_t Size) { SetData(pData, Size); }
	~CEncryptBase()
	{
		if (m_pData)
			delete m_pData;
	}

	void SetData(const byte pData[], const size_t Size);
	void GetData(byte *pData) const;
	byte *GetData() const;
	size_t GetDataSize() const { return m_DataSize; }

	virtual void Encrypt() {}
	virtual bool Decrypt() { return false; }
};

class CEncryptData1 : public CEncryptBase
{
public:
	CEncryptData1() : CEncryptBase(){};
	CEncryptData1(const byte pData[], const size_t Size) : CEncryptBase(pData, Size) {}

	void Encrypt();
	bool Decrypt();
	void RotateBytes(const int Offset, const size_t Length);
};

class CEncryptData2 : public CEncryptBase
{
public:
	CEncryptData2() : CEncryptBase(){};
	CEncryptData2(const byte pData[], const size_t Size) : CEncryptBase(pData, Size) {}

	void Encrypt();
	bool Decrypt();
};
