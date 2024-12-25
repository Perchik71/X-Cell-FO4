#pragma once

#include "f4se_common/Utilities.h"
#include "f4se/GameTypes.h"
#include "f4se/GameAPI_OG.h"
#include "f4se/GameUtilities.h"

// 08
class BSReadWriteLock_OG
{
	enum
	{
		kFastSpinThreshold = 10000,
		kLockWrite = 0x80000000,
		kLockCountMask = 0xFFFFFFF
	};

	volatile SInt32	threadID;	// 00
	volatile SInt32	lockValue;	// 04

public:
	BSReadWriteLock_OG() : threadID(0), lockValue(0) {}

	DEFINE_MEMBER_FN_0(LockForRead, void, 0x01B10930);
	DEFINE_MEMBER_FN_0(LockForWrite, void, 0x01B109B0);

	DEFINE_MEMBER_FN_0(UnlockRead, void, 0x01B10BF0);
	DEFINE_MEMBER_FN_0(UnlockWrite, void, 0x01B10C00);
};
STATIC_ASSERT(sizeof(BSReadWriteLock_OG) == 0x8);

class BSReadLocker_OG
{
public:
	BSReadLocker_OG(BSReadWriteLock_OG* lock) { m_lock = lock; m_lock->LockForRead(); }
	~BSReadLocker_OG() { m_lock->UnlockRead(); }

protected:
	BSReadWriteLock_OG* m_lock;
};

class BSWriteLocker_OG
{
public:
	BSWriteLocker_OG(BSReadWriteLock_OG* lock) { m_lock = lock; m_lock->LockForWrite(); }
	~BSWriteLocker_OG() { m_lock->UnlockWrite(); }

protected:
	BSReadWriteLock_OG* m_lock;
};

// 80808
class StringCache_OG
{
public:
	// 18+
	struct Entry
	{
		enum
		{
			kState_RefcountMask =	0x3FFF,
			kState_External =		0x4000,
			kState_Wide =			0x8000
		};

		Entry	* next;				// 00
		UInt32	state;				// 08 - refcount, hash, flags
		UInt32	length;				// 0C
		Entry	* externData;		// 10
		char	data[0];			// 18

		bool IsWide()
		{
			Entry * iter = this;

			while(iter->state & kState_External)
				iter = iter->externData;

			if((iter->state & kState_Wide) == kState_Wide)
				return true;

			return false;
		}

		template<typename T>
		T * Get()
		{
			Entry * iter = this;

			while(iter->state & kState_External)
				iter = iter->externData;

			return (T*)iter->data;
		}
	};

	struct Ref
	{
		Entry	* data;

		MEMBER_FN_PREFIX(Ref);
		// D3703E13297FD78BE317E0223C90DAB9021465DD+6F
		DEFINE_MEMBER_FN(ctor, Ref *, 0x01B41D40, const char * buf);
		// 34CA732E6B3C7BCD20DEFC8B3711427E5285FF82+AA
		DEFINE_MEMBER_FN(ctor_w, Ref *, 0x01B42B60, const wchar_t * buf);
		// 489C5F60950D108691FCB6CB0026101275BE474A+79
		DEFINE_MEMBER_FN(Set, Ref *, 0x01B41E70, const char * buf);
		DEFINE_MEMBER_FN(Set_w, Ref *, 0x01B443C0, const wchar_t * buf);

		DEFINE_MEMBER_FN(Release, void, 0x01B42FD0);

		Ref();
		Ref(const char * buf);
		Ref(const wchar_t * buf);
		
		void Release();

		bool operator==(const char * lhs) const;
		bool operator==(const Ref& lhs) const { return data == lhs.data; }
		bool operator<(const Ref& lhs) const { return data < lhs.data; }

		const char * c_str() const { return operator const char *(); }
		operator const char *() const { return data ? data->Get<char>() : nullptr; }

		const wchar_t * wc_str() { return operator const wchar_t *(); }
		operator const wchar_t *() { return data ? data->Get<wchar_t>() : nullptr; }
	};

	// 10
	struct Lock
	{
		UInt32	unk00;	// 00 - set to 80000000 when locked
		UInt32	pad04;	// 04
		UInt64	pad08;	// 08
	};

	Entry	* lut[0x10000];	// 00000
	Lock	lock[0x80];		// 80000
	UInt8	isInit;			// 80800
};

typedef StringCache_OG::Ref BSFixedString_OG;
typedef StringCache_OG::Ref BSFixedStringW_OG;

class BSAutoFixedString_OG : public BSFixedString_OG
{
public:
	BSAutoFixedString_OG() : BSFixedString_OG() { }
	BSAutoFixedString_OG(const char * buf) : BSFixedString_OG(buf) { }

	~BSAutoFixedString_OG()
	{
		Release();
	}
};