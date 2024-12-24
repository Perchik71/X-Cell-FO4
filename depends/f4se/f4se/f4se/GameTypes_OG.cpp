#include "f4se/GameTypes_OG.h"

StringCache_OG::Ref::Ref()
{
	CALL_MEMBER_FN(this, ctor)("");
}

StringCache_OG::Ref::Ref(const char * buf)
{
	CALL_MEMBER_FN(this, ctor)(buf);
}

StringCache_OG::Ref::Ref(const wchar_t * buf)
{
	CALL_MEMBER_FN(this, ctor_w)(buf);
}

void StringCache_OG::Ref::Release()
{
	CALL_MEMBER_FN(this, Release)();
}

bool StringCache_OG::Ref::operator==(const char * lhs) const
{
	Ref tmp(lhs);
	bool res = data == tmp.data;
	CALL_MEMBER_FN(&tmp, Release)();
	return res;
}