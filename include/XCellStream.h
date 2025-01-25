// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include <ICriticalSection.h>

namespace XCell
{
	enum StreamOffset
	{
		kStreamBegin = 0,
		kStreamCurrent,
		kStreamEnd
	};

	typedef void(*StreamReadEvent)(void* Buf, int64_t Size);
	typedef void(*StreamWriteEvent)(const void* Buf, int64_t Size);

	class CustomStream
	{
		bool _abort;
	protected:
		ICriticalSection _lock;

		void DoReadBuf(void* Buf, int64_t Size) const noexcept;
		void DoWriteBuf(const void* Buf, int64_t Size) const noexcept;
	public:
		CustomStream();
		virtual ~CustomStream() = default;
		CustomStream(const CustomStream& Rhs);
		CustomStream& operator=(const CustomStream& Rhs);

		StreamReadEvent OnReadBuf;
		StreamWriteEvent OnWriteBuf;

		virtual int32_t ReadBuf(void* Buf, int32_t Size) const = 0;
		virtual int32_t WriteBuf(const void* Buf, int32_t Size) = 0;
		virtual int64_t Seek(int64_t Offset, StreamOffset Flag) const = 0;

		virtual int64_t CopyFrom(CustomStream& Steam, int64_t Size = 0);
		inline void Assign(CustomStream& Steam) { CopyFrom(Steam); }
		inline void Abort() noexcept { _abort = true; }

		[[nodiscard]] virtual int64_t GetPosition() const noexcept;
		virtual void SetPosition(int64_t Position) const noexcept;

		[[nodiscard]] virtual int64_t GetSize() const noexcept;

		[[nodiscard]] inline bool IsEmpty() const { return GetSize() == 0; }
		[[nodiscard]] inline bool Eof() const { return GetSize() == GetPosition(); }

		XCProperty(GetPosition, SetPosition) int64_t Position;
		XCPropertyReadOnly(GetSize) int64_t Size;
	};

	enum class FileStreamMode
	{
		kStreamCreate = 0,
		kStreamOpenRead,
		kStreamOpenReadWrite
	};

	class FileStream : public CustomStream
	{
		void* _handle;
		string _FileName;
	public:
		FileStream();
		explicit FileStream(const char* FileName, FileStreamMode Mode, bool Cache = true);
		explicit FileStream(const wchar_t* FileName, FileStreamMode Mode, bool Cache = true);
		FileStream(const string& FileName, FileStreamMode Mode, bool Cache = true);
		virtual ~FileStream();

		FileStream(const FileStream& Rhs) = delete;
		FileStream& operator=(const FileStream& Rhs) = delete;

		virtual int32_t ReadBuf(void* Buf, int32_t Size) const;
		virtual int32_t WriteBuf(const void* Buf, int32_t Size);
		virtual int64_t Seek(int64_t Offset, StreamOffset Flag) const;

		virtual bool Open(const char* FileName, FileStreamMode Mode, bool Cache = true);
		virtual bool Open(const wchar_t* FileName, FileStreamMode Mode, bool Cache = true);
		virtual bool Open(const string& FileName, FileStreamMode Mode, bool Cache = true);
		[[nodiscard]] virtual bool IsOpen() const noexcept;
		virtual void Close();
		virtual void Flush() const noexcept;

		[[nodiscard]] virtual string GetFileName() const noexcept;

		XCPropertyReadOnly(GetFileName) string FileName;
	};

	enum TextFileEncode : uint8_t
	{
		kTextEncode_ANSI = 0,
		kTextEncode_UTF8,
		kTextEncode_UTF16,
	};

	class TextFileStream : public FileStream
	{
	public:
		virtual int32_t ReadString(char* Str, int32_t Length) const;
		virtual int32_t ReadStringLine(char** Str) const;
		virtual int32_t ReadStringLine(string& Str, TextFileEncode SourceEncode = kTextEncode_ANSI) const;
		virtual int32_t ReadStdStringLine(std::string& Str) const;
		virtual int32_t WriteString(const char* Str);
		virtual int32_t WriteString(const char* Str, int32_t Length);
		virtual int32_t WriteString(const string& Str, TextFileEncode SourceEncode = kTextEncode_ANSI);
		virtual int32_t WriteFormatString(const char* FormatStr, ...);
		virtual int32_t WriteFormatString(const char* FormatStr, va_list ap);
	};

	class FileStreamInterfaceMethods
	{
	public:
		bool LoadFromFile(const char* FileName);
		bool LoadFromFile(const wchar_t* FileName);
		bool SaveToFile(const char* FileName) const;
		bool SaveToFile(const wchar_t* FileName) const;
		virtual bool LoadFromStream(CustomStream& Steam) = 0;
		virtual bool SaveToStream(CustomStream& Steam) const = 0;
	};

	class TextFileStreamInterfaceMethods
	{
	public:
		virtual bool LoadFromTextFile(const char* FileName);
		virtual bool LoadFromTextFile(const wchar_t* FileName);
		virtual bool SaveToTextFile(const char* FileName) const;
		virtual bool SaveToTextFile(const wchar_t* FileName) const;
		virtual bool LoadFromTextStream(CustomStream& Steam) = 0;
		virtual bool SaveToTextStream(CustomStream& Steam) const = 0;
	};

	class MemoryStream : public CustomStream, public FileStreamInterfaceMethods
	{
		void* _data;
		int64_t _pos;
		int64_t _size;
	protected:
		void* Allocate(int64_t NewSize);
		void Deallocate();
	public:
		MemoryStream();
		MemoryStream(const void* Buf, int32_t Size);
		virtual ~MemoryStream();
		MemoryStream(const MemoryStream& Rhs);
		MemoryStream& operator=(const MemoryStream& Rhs);

		virtual int32_t ReadBuf(void* Buf, int32_t Size) const;
		virtual int32_t WriteBuf(const void* Buf, int32_t Size);
		virtual int64_t Seek(int64_t Offset, StreamOffset Flag) const;
		virtual void SetSize(int64_t NewSize);

		virtual void Clear();
		inline void* Memory() const noexcept { return _data; }

		virtual bool LoadFromStream(CustomStream& Steam);
		virtual bool SaveToStream(CustomStream& Steam) const;
	};
}
