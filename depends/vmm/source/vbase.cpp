// Copyright © 2023 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/lgpl-3.0.html

#include "vbase.h"
#include "valloc.h"
#include "vassert.h"
#include <intrin.h>
#include <string>

namespace voltek
{
	namespace core
	{
		bool initialize_success = false;
		// Поддержка процессором AVX2
		bool avx2_supported = false;
		// Поддержка процессором SSE4.1
		bool sse41_supported = false;
		// Поддержка Hyper
		bool hyper_threads = false;
		// Кол-во логический ядер или просто ядер, если нет Hyper
		unsigned char logical_cores = 0;

		void get_cpu(std::string& output)
		{
			try
			{
				int CPUInfo[4] = { -1 };
				__cpuid(CPUInfo, 0x80000000);
				unsigned int nExIds = CPUInfo[0];

				char CPUBrandString[0x40] = { 0 };
				for (unsigned int i = 0x80000000; i <= nExIds; ++i)
				{
					__cpuid(CPUInfo, i);
					if (i == 0x80000002)
					{
						memcpy(CPUBrandString,
							CPUInfo,
							sizeof(CPUInfo));
					}
					else if (i == 0x80000003)
					{
						memcpy(CPUBrandString + 16,
							CPUInfo,
							sizeof(CPUInfo));
					}
					else if (i == 0x80000004)
					{
						memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
					}
				}

				output = _strlwr(CPUBrandString);
			}
			catch (...) { return; }
		}

		void initialize()
		{
			if (initialize_success) return;

			int info[4];
			__cpuid(info, 7);

			initialize_success = true;
			avx2_supported = (info[1] & (1 << 5)) != 0;

			__cpuid(info, 1);
			sse41_supported = (info[2] & (1 << 19)) != 0;
			hyper_threads = (info[3] & (1 << 28)) != 0;
			logical_cores = (info[1] >> 16) & 0xff;

			if (avx2_supported)
			{
				if (!hyper_threads) 
					// Отключение использования AVX2 на процессорах не поддерживающих Hyper 
					avx2_supported = false;
				else
				{
					std::string name;
					get_cpu(name);

					// Отключение использования AVX2 на процессорах Intel и других
					if (name.find_first_of("amd ") == std::string::npos)
						avx2_supported = false;
				}
			}
		}

		void* base::operator new (size_t size)
		{
			_vassert(size > 0);
			void* ptr = _internal::aligned_malloc(size, 0x10);
			_vassert(ptr != nullptr);
			return ptr;
		}

		void base::operator delete (void* ptr)
		{
			_vassert(ptr != nullptr);
			_internal::aligned_free(ptr);
		}
	}
}