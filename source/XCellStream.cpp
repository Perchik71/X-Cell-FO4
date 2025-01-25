// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <memory>

#include "XCellStream.h"
#include "XCellStringUtils.h"

namespace XCell
{
	// CustomStream

	CustomStream::CustomStream() :
		OnReadBuf(nullptr), OnWriteBuf(nullptr), _abort(false)
	{}

	CustomStream::CustomStream(const CustomStream& Rhs) :
		OnReadBuf(nullptr), OnWriteBuf(nullptr), _abort(false)
	{
		*this = Rhs;
	}

	CustomStream& CustomStream::operator=(const CustomStream& Rhs)
	{
		auto SafePos = Rhs.GetPosition();
		Assign(const_cast<CustomStream&>(Rhs));
		Rhs.SetPosition(SafePos);
		return *this;
	}

	void CustomStream::DoReadBuf(void* Buf, int64_t Size) const noexcept
	{
		if (OnReadBuf)
			OnReadBuf(Buf, Size);
	}

	void CustomStream::DoWriteBuf(const void* Buf, int64_t Size) const noexcept
	{
		if (OnWriteBuf)
			OnWriteBuf(Buf, Size);
	}

	int64_t CustomStream::CopyFrom(CustomStream& Steam, int64_t Size)
	{
		if (Steam.IsEmpty())
			return 0;

		IScopedCriticalSection Locker(&_lock);

		constexpr int32_t BufSize = 64 * 1024;
		auto Buffer = std::make_unique<char[]>(BufSize);
		if (!Buffer)
		{
			_FATALERROR("Out of memory");
			return 0;
		}

		int64_t AllReadBytes = Steam.GetPosition();
		int64_t AllBytes = Size + AllReadBytes;

		if (!Size)
		{
			AllBytes = Steam.GetSize();
			Steam.SetPosition(0);
		}

		int64_t ReadBytes = 0;
		int64_t ReadSize = BufSize;

		_abort = false;

		do
		{
			ReadSize = (AllBytes - AllReadBytes) > BufSize ? BufSize : AllBytes - AllReadBytes;
			ReadBytes = Steam.ReadBuf(Buffer.get(), (int32_t)ReadSize);
			if (!ReadBytes)
				goto __Leave;

			WriteBuf(Buffer.get(), (int32_t)ReadBytes);
			AllReadBytes += ReadBytes;

			if (_abort)
				goto __Leave;
		} while (AllBytes > ReadBytes);

	__Leave:
		return AllReadBytes;
	}

	int64_t CustomStream::GetPosition() const noexcept
	{
		return Seek(0, StreamOffset::kStreamCurrent);
	}

	void CustomStream::SetPosition(int64_t Position) const noexcept
	{
		Seek(Position, StreamOffset::kStreamBegin);
	}

	int64_t CustomStream::GetSize() const noexcept
	{
		auto Safe = Seek(0, StreamOffset::kStreamCurrent);
		auto Size = Seek(0, StreamOffset::kStreamEnd);
		Seek(Safe, StreamOffset::kStreamBegin);

		return Size;
	}

	// FileStream

	FileStream::FileStream() :
		_handle(INVALID_HANDLE_VALUE)
	{}

	FileStream::FileStream(const char* FileName, FileStreamMode Mode, bool Cache) :
		_handle(INVALID_HANDLE_VALUE)
	{
		Open(FileName, Mode, Cache);
	}

	FileStream::FileStream(const wchar_t* FileName, FileStreamMode Mode, bool Cache) :
		_handle(INVALID_HANDLE_VALUE)
	{
		Open(FileName, Mode, Cache);
	}

	FileStream::FileStream(const string& FileName, FileStreamMode Mode, bool Cache) :
		_handle(INVALID_HANDLE_VALUE)
	{
		Open(FileName.c_str(), Mode, Cache);
	}

	FileStream::~FileStream()
	{
		Close();
	}

	int32_t FileStream::ReadBuf(void* Buf, int32_t Size) const
	{
		IScopedCriticalSection Locker(&((const_cast<FileStream*>(this))->_lock));

		int32_t readbytes = 0;
		if (!ReadFile((HANDLE)_handle, Buf, Size, (LPDWORD)&readbytes, nullptr))
		{
			const_cast<FileStream*>(this)->Abort();
			return -1;
		}

		DoReadBuf(Buf, readbytes);
		return readbytes;
	}

	int32_t FileStream::WriteBuf(const void* Buf, int32_t Size)
	{
		IScopedCriticalSection Locker(&_lock);

		int32_t writebytes = 0;
		if (!WriteFile((HANDLE)_handle, Buf, Size, (LPDWORD)&writebytes, nullptr))
		{
			Abort();
			return -1;
		}

		DoWriteBuf(Buf, writebytes);
		return writebytes;
	}

	int64_t FileStream::Seek(int64_t Offset, StreamOffset Flag) const
	{
		IScopedCriticalSection Locker(&((const_cast<FileStream*>(this))->_lock));

		int64_t pos = 0;
		LARGE_INTEGER li{};
		li.QuadPart = (LONGLONG)Offset;

		if (!SetFilePointerEx((HANDLE)_handle, li, (PLARGE_INTEGER)&pos, (DWORD)Flag))
		{
			const_cast<FileStream*>(this)->Abort();
			return -1;
		}

		return pos;
	}

	bool FileStream::Open(const char* FileName, FileStreamMode Mode, bool Cache)
	{
		if (_handle != INVALID_HANDLE_VALUE)
			return false;

		_handle = (void*)CreateFileA(FileName,
			(Mode == FileStreamMode::kStreamOpenRead) ? GENERIC_READ : (GENERIC_READ | GENERIC_WRITE),
			(Mode == FileStreamMode::kStreamOpenRead) ? FILE_SHARE_READ : 0,
			nullptr,
			(Mode == FileStreamMode::kStreamCreate) ? CREATE_ALWAYS : OPEN_ALWAYS,
			((Cache) ? FILE_FLAG_SEQUENTIAL_SCAN : FILE_FLAG_NO_BUFFERING) | FILE_ATTRIBUTE_NORMAL,
			nullptr);

		bool bRet = _handle != INVALID_HANDLE_VALUE;
		if (bRet)
			_FileName = FileName;
		else
			_ERROR("Couldn't open file: \"%s\"", FileName);

		return bRet;
	}

	bool FileStream::Open(const wchar_t* FileName, FileStreamMode Mode, bool Cache)
	{
		if (_handle != INVALID_HANDLE_VALUE)
			return false;

		_handle = (void*)CreateFileW(FileName,
			(Mode == FileStreamMode::kStreamOpenRead) ? GENERIC_READ : (GENERIC_READ | GENERIC_WRITE),
			(Mode == FileStreamMode::kStreamOpenRead) ? FILE_SHARE_READ : 0,
			nullptr,
			(Mode == FileStreamMode::kStreamCreate) ? CREATE_ALWAYS : OPEN_ALWAYS,
			((Cache) ? FILE_FLAG_SEQUENTIAL_SCAN : FILE_FLAG_NO_BUFFERING) | FILE_ATTRIBUTE_NORMAL,
			nullptr);

		bool bRet = _handle != INVALID_HANDLE_VALUE;
		if (bRet)
			_FileName = Utils::WideToAnsi(FileName);
		else
			_ERROR("Couldn't open file: \"%s\"", Utils::WideToAnsi(FileName).c_str());

		return bRet;
	}

	bool FileStream::Open(const string& FileName, FileStreamMode Mode, bool Cache)
	{
		return Open(FileName.c_str(), Mode, Cache);
	}

	bool FileStream::IsOpen() const noexcept
	{
		return _handle != INVALID_HANDLE_VALUE;
	}

	void FileStream::Close()
	{
		IScopedCriticalSection Locker(&_lock);

		if (IsOpen())
		{
			FlushFileBuffers((HANDLE)_handle);
			CloseHandle((HANDLE)_handle);
			_handle = INVALID_HANDLE_VALUE;
			_FileName.clear();
		}
	}

	void FileStream::Flush() const noexcept
	{
		if (IsOpen())
			FlushFileBuffers((HANDLE)_handle);
	}

	string FileStream::GetFileName() const noexcept
	{
		return _FileName;
	}

	// TextFileStream

	int32_t TextFileStream::ReadString(char* Str, int32_t Length) const
	{
		return ReadBuf(Str, Length);
	}

	int32_t TextFileStream::ReadStringLine(char** Str) const
	{
		if (!Str)
			return -1;

		constexpr static size_t MAX = 250;

		std::string sBuf;

		do
		{
			char szBuf[MAX + 1];
			auto ReadBytes = ReadBuf(szBuf, (int32_t)MAX);
			szBuf[MAX] = 0;

			if (ReadBytes <= 0)
				return ReadBytes;

			int32_t iL = 0;
			for (; iL < ReadBytes; iL++)
				if (szBuf[iL] == '\n')
				{
					Seek((int64_t)iL - ReadBytes, StreamOffset::kStreamCurrent);
					break;
				}
			sBuf.append(szBuf, (size_t)iL);
			if (MAX != iL) break;
		} while (!Eof());

		if (!sBuf.length())
			return 0;

		auto l = (int32_t)sBuf.length();
		*Str = (char*)malloc((size_t)l + 1);
		if (*Str)
		{
			memcpy(*Str, sBuf.c_str(), l);
			*Str[l] = 0;
			return l;
		}

		Seek(-((int64_t)l), StreamOffset::kStreamCurrent);
		return -1;
	}

	int32_t TextFileStream::ReadStringLine(string& Str, TextFileEncode SourceEncode) const
	{
		Str.clear();

		constexpr static size_t MAX = 250;

		if (SourceEncode == TextFileEncode::kTextEncode_UTF16)
		{
			do
			{
				wchar_t szBuf[MAX + 1];
				auto ReadBytes = ReadBuf(szBuf, (int32_t)(MAX * sizeof(wchar_t)));
				szBuf[MAX] = 0;

				if (ReadBytes <= 0)
					return ReadBytes;

				int32_t iL = 0;
				int32_t iMax = ReadBytes / sizeof(wchar_t);

				for (; iL < iMax; iL++)
					if (szBuf[iL] == L'\n')
					{
						Seek((int64_t)(((size_t)iL + 1) * sizeof(wchar_t)) - ReadBytes, StreamOffset::kStreamCurrent);
						break;
					}
				Str.append(Utils::WideToAnsi(szBuf), (size_t)iL);
				if (MAX != iL) break;
			} while (!Eof());
		}
		else
		{
			do
			{
				char szBuf[MAX + 1];
				auto ReadBytes = ReadBuf(szBuf, (int32_t)MAX);
				szBuf[MAX] = 0;

				if (ReadBytes <= 0)
					return ReadBytes;

				int32_t iL = 0;
				for (; iL < ReadBytes; iL++)
					if (szBuf[iL] == '\n')
					{
						Seek((int64_t)(iL + 1) - ReadBytes, StreamOffset::kStreamCurrent);
						break;
					}
				if (SourceEncode == TextFileEncode::kTextEncode_UTF8)
				{
					szBuf[iL] = 0;
					Str.append(Utils::Utf8ToAnsi(szBuf));
				}
				else
					Str.append(szBuf, (size_t)iL);
				if (MAX != iL) break;
			} while (!Eof());
		}

		return (int32_t)Str.length();
	}

	int32_t TextFileStream::ReadStdStringLine(std::string& Str) const
	{
		Str.clear();

		constexpr static size_t MAX = 250;

		do
		{
			char szBuf[MAX + 1];
			auto ReadBytes = ReadBuf(szBuf, (int32_t)MAX);
			szBuf[MAX] = 0;

			if (ReadBytes <= 0)
				return ReadBytes;

			int32_t iL = 0;
			for (; iL < ReadBytes; iL++)
				if (szBuf[iL] == '\n')
				{
					Seek((int64_t)(iL + 1) - ReadBytes, StreamOffset::kStreamCurrent);
					break;
				}
			Str.append(szBuf, (size_t)iL);
			if (MAX != iL) break;
		} while (!Eof());

		return (int32_t)Str.length();
	}

	int32_t TextFileStream::WriteString(const char* Str)
	{
		return WriteString(Str, (int32_t)strlen(Str));
	}

	int32_t TextFileStream::WriteString(const char* Str, int32_t Length)
	{
		return WriteBuf(Str, Length);
	}

	int32_t TextFileStream::WriteString(const string& Str, TextFileEncode SourceEncode)
	{
		switch (SourceEncode)
		{
		case kTextEncode_ANSI:
			return WriteBuf((const void*)Str.c_str(), (int32_t)Str.length());
		case kTextEncode_UTF8:
		{
			string temp = Utils::AnsiToUtf8(Str);
			return WriteBuf((const void*)temp.c_str(), (int32_t)temp.length());
		}
		case kTextEncode_UTF16:
		{
			wstring temp = Utils::AnsiToWide(Str);
			return WriteBuf((const void*)temp.c_str(), (int32_t)(temp.length() * sizeof(wchar_t)));
		}
		default:
			return -1;
		}
	}

	int32_t TextFileStream::WriteFormatString(const char* FormatStr, ...)
	{
		va_list ap;
		va_start(ap, FormatStr);
		int32_t r = WriteFormatString(FormatStr, ap);
		va_end(ap);
		return r;
	}

	int32_t TextFileStream::WriteFormatString(const char* FormatStr, va_list ap)
	{
		int32_t len = _vscprintf(FormatStr, ap);
		if (len <= 0) return 0;
		auto buf = std::make_unique<char[]>((size_t)len + 1);
		if (!buf) return 0;
		vsprintf(buf.get(), FormatStr, ap);
		return WriteString(buf.get(), len);
	}

	// FileStreamInterfaceMethods

	bool FileStreamInterfaceMethods::LoadFromFile(const char* FileName)
	{
		FileStream fstm;
		if (!fstm.Open(FileName, FileStreamMode::kStreamOpenRead))
			return false;

		bool ret = LoadFromStream(fstm);
		fstm.Close();

		return ret;
	}

	bool FileStreamInterfaceMethods::LoadFromFile(const wchar_t* FileName)
	{
		FileStream fstm;
		if (!fstm.Open(FileName, FileStreamMode::kStreamOpenRead))
			return false;

		bool ret = LoadFromStream(fstm);
		fstm.Close();

		return ret;
	}

	bool FileStreamInterfaceMethods::SaveToFile(const char* FileName) const
	{
		FileStream fstm;
		if (!fstm.Open(FileName, FileStreamMode::kStreamCreate))
			return false;

		bool ret = SaveToStream(fstm);
		fstm.Close();

		return ret;
	}

	bool FileStreamInterfaceMethods::SaveToFile(const wchar_t* FileName) const
	{
		FileStream fstm;
		if (!fstm.Open(FileName, FileStreamMode::kStreamCreate))
			return false;

		bool ret = SaveToStream(fstm);
		fstm.Close();

		return ret;
	}

	// TextFileStreamInterfaceMethods

	bool TextFileStreamInterfaceMethods::LoadFromTextFile(const char* FileName)
	{
		TextFileStream fstm;
		if (!fstm.Open(FileName, FileStreamMode::kStreamOpenRead))
			return false;

		bool ret = LoadFromTextStream(fstm);
		fstm.Close();

		return ret;
	}

	bool TextFileStreamInterfaceMethods::LoadFromTextFile(const wchar_t* FileName)
	{
		TextFileStream fstm;
		if (!fstm.Open(FileName, FileStreamMode::kStreamOpenRead))
			return false;

		bool ret = LoadFromTextStream(fstm);
		fstm.Close();

		return ret;
	}

	bool TextFileStreamInterfaceMethods::SaveToTextFile(const char* FileName) const
	{
		TextFileStream fstm;
		if (!fstm.Open(FileName, FileStreamMode::kStreamCreate))
			return false;

		bool ret = SaveToTextStream(fstm);
		fstm.Close();

		return ret;
	}

	bool TextFileStreamInterfaceMethods::SaveToTextFile(const wchar_t* FileName) const
	{
		TextFileStream fstm;
		if (!fstm.Open(FileName, FileStreamMode::kStreamCreate))
			return false;

		bool ret = SaveToTextStream(fstm);
		fstm.Close();

		return ret;
	}

	// MemoryStream

	void* MemoryStream::Allocate(int64_t NewSize)
	{
		void* ret = (!_data) ? malloc(NewSize) : realloc(_data, NewSize);
		if (ret)
		{
			_size = NewSize;
			_data = ret;

			if (_size < _pos)
				_pos = _size;
		}
		else
		{
			_pos = 0;
			_size = 0;
			_data = nullptr;
		}

		return ret;
	}

	void MemoryStream::Deallocate()
	{
		if (_data)
		{
			free(_data);

			_data = nullptr;
			_pos = 0;
			_size = 0;
		}
	}

	MemoryStream::MemoryStream() :
		_data(nullptr), _pos(0), _size(0)
	{}

	MemoryStream::MemoryStream(const void* Buf, int32_t Size) :
		_data(nullptr), _pos(0), _size(0)
	{
		WriteBuf(Buf, Size);
	}

	MemoryStream::~MemoryStream()
	{
		Clear();
	}

	MemoryStream::MemoryStream(const MemoryStream& Rhs)
	{
		*this = Rhs;
	}

	MemoryStream& MemoryStream::operator=(const MemoryStream& Rhs)
	{
		auto SafePos = Rhs.GetPosition();
		Assign(const_cast<MemoryStream&>(Rhs));
		Rhs.SetPosition(SafePos);
		return *this;
	}

	int32_t MemoryStream::ReadBuf(void* Buf, int32_t Size) const
	{
		IScopedCriticalSection Locker(&((const_cast<MemoryStream*>(this))->_lock));

		if (_pos >= _size || !_data)
			return 0;

		int32_t readbytes = Size;
		if (((int64_t)readbytes) > (_size - _pos))
			readbytes = (int32_t)(_size - _pos);

		memcpy(Buf, ((char*)_data) + _pos, readbytes);
		*(const_cast<int64_t*>(&_pos)) += (int64_t)readbytes;

		return readbytes;
	}

	int32_t MemoryStream::WriteBuf(const void* Buf, int32_t Size)
	{
		IScopedCriticalSection Locker(&_lock);

		if (!_data || ((_size - _pos) < (int64_t)Size))
		{
			if (!Allocate(_pos + Size))
			{
				Abort();
				return 0;
			}
		}

		if (_data)
		{
			memcpy(((char*)_data) + _pos, Buf, Size);
			_pos += (int64_t)Size;
		}
		else
			return 0;

		return Size;
	}

	int64_t MemoryStream::Seek(int64_t Offset, StreamOffset Flag) const
	{
		IScopedCriticalSection Locker(&((const_cast<MemoryStream*>(this))->_lock));

		int64_t ret = 0;

		switch (Flag)
		{
		case StreamOffset::kStreamBegin:
			ret = Offset;
			break;
		case StreamOffset::kStreamCurrent:
			ret = _pos + Offset;
			break;
		case StreamOffset::kStreamEnd:
			ret = _size + Offset;
			break;
		}

		*(const_cast<int64_t*>(&_pos)) = ret;
		return ret;
	}

	void MemoryStream::SetSize(int64_t NewSize)
	{
		Allocate(NewSize);
	}

	void MemoryStream::Clear()
	{
		Deallocate();
	}

	bool MemoryStream::LoadFromStream(CustomStream& Steam)
	{
		auto ret = CopyFrom(Steam);
		return ret == Steam.GetSize();
	}

	bool MemoryStream::SaveToStream(CustomStream& Steam) const
	{
		auto no_const_self = const_cast<MemoryStream*>(this);

		auto safe = no_const_self->GetPosition();
		no_const_self->SetPosition(0);

		auto ret = Steam.CopyFrom(*no_const_self);

		no_const_self->SetPosition(safe);
		return ret == GetSize();
	}
}