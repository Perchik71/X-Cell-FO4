// Copyright � 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/lgpl-3.0.html

#pragma once

#include "vbits.h"

namespace voltek
{
	namespace core
	{
		// ����� ������, ������� �����������������, ��� ����������� ����, 
		// ����� ������ �������� ������ ������ ��� ��������.
		// ����: ��������� 2%... ���������.
		class mapper : public base
		{
		public:
			// ������ �� ���������.
			inline static constexpr size_t DEFAULT_SIZE = 256 * 1024 * 1024;
			// ������ ������ ����� �� ���������.
			inline static constexpr size_t DEFAULT_BLOCKSIZE = DEFAULT_SIZE / 65536;
			// ����������� �� ���������.
			mapper();
			// �����������.
			// � �������� ��������� ����������� �������� ������ � ��.
			// � ������ ������ ����� � ������.
			mapper(size_t size, size_t blocksize);
			// ����������� �����.
			mapper(const bits& ob);
			// ����������.
			virtual ~mapper();
			// �������� ����������
			mapper& operator=(const mapper& ob);
			// ������� ���������� ���� ������ ������ ���� � ��� ������.
			inline bool empty() const { return !_size; }
			// ������������ ������.
			inline void clear() { resize(0, 0); }
			// ���������� ������ ���������� ������.
			inline size_t size() const { return _size; }
			// ���������� ������ ��������� ������.
			inline size_t free_size() const { return _freesize; }
			// ���������� ������ ������ �����.
			inline size_t block_size() const { return _blocksize; }
			// ���������� ���-�� ����� ���������� ������.
			inline size_t count_blocks() const { return _size / _blocksize; }
			// ���������� ���-�� ��������� ������.
			inline size_t free_blocks() const { return _freesize / _blocksize; }
			// ���������� ��������� �� ������, ���������.
			inline const char* c_data() const { return _mem; }
			// ���������� ��������� �� ������, �� ���������.
			inline char* data() { return _mem; }
			// ���������� ��������� �� ������ ���������� �����.
			void* block_alloc();
			// ����������� ������ ����� �� ��������� �� ������.
			bool block_free(const void* ptr);
		private:
			// �������� ������ ������.
			// � �������� ��������� ����������� �������� ������ � ������.
			// ���� ������� 0, �� ����� ����������� ������.
			// � ������ ������ ����� � ������.
			void resize(size_t size, size_t blocksize);
			// ���������� ������ ���� ��������� ����������� ������. 
			bool is_valid_ptr(const void* ptr) const;
		private:
			// ������.
			char* _mem;
			// ���������� ������.
			size_t _size;
			// ��������.
			size_t _freesize;
			// ������ ������ �����.
			size_t _blocksize;
			// ������� ����� ��� �������� ������.
			bits* _mask;
		};
	}
}
