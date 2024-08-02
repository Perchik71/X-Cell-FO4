// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

namespace xc
{
	void _assert(bool comp, const char* file_name, int line, const char* fmt, ...);
}

#define _xc_assert(Cond)					xc::_assert(Cond, __FILE__, __LINE__, #Cond)
#define _xc_assert_msg_fmt(Cond, Msg, ...)	xc::_assert(Cond, __FILE__, __LINE__, "%s\n\n" Msg, #Cond, ##__VA_ARGS__)
#define _xc_assert_msg(Cond, Msg)			_xc_assert_msg_fmt(Cond, Msg)