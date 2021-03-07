// Cross-platform thread local storage memory allocation class.  Built for short-lived, in-thread memory allocations.
// (C) 2013 CubicleSoft.  All Rights Reserved.

#include "sync_tls.h"
#include "../templates/fast_find_replace.h"
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <new>

namespace CubicleSoft
{
	namespace Sync
	{
#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
		// Windows.
		TLS::TLS()
		{
			MxTlsIndex = ::TlsAlloc();
		}

		TLS::~TLS()
		{
			::TlsFree(MxTlsIndex);
		}

		bool TLS::SetMainPtr(StaticVector<Queue<char>> *MainPtr)
		{
			return (::TlsSetValue(MxTlsIndex, MainPtr) != 0);
		}

		StaticVector<Queue<char>> *TLS::GetMainPtr()
		{
			return (StaticVector<Queue<char>> *)::TlsGetValue(MxTlsIndex);
		}
#else
		// POSIX pthreads.
		TLS::TLS()
		{
			pthread_key_create(&MxKey, NULL);
		}

		TLS::~TLS()
		{
			pthread_key_delete(MxKey);
		}

		bool TLS::SetMainPtr(StaticVector<Queue<char>> *MainPtr)
		{
			return (pthread_setspecific(MxKey, MainPtr) == 0);
		}

		StaticVector<Queue<char>> *TLS::GetMainPtr()
		{
			return (StaticVector<Queue<char>> *)pthread_getspecific(MxKey);
		}
#endif

		// All platforms.
		bool TLS::ThreadInit(size_t MaxCacheBits)
		{
			StaticVector<Queue<char>> *MainPtr = GetMainPtr();
			if (MainPtr != NULL)  return true;

			return SetMainPtr(new StaticVector<Queue<char>>(MaxCacheBits));
		}

		void *TLS::malloc(size_t Size, size_t Align)
		{
			if (Size == 0)  return NULL;

			StaticVector<Queue<char>> *MainPtr = GetMainPtr();
			if (MainPtr == NULL)  return NULL;

			void *Data;
			if (!Align)  Align = (Size > 4 ? 8 : (Size > 2 ? 4 : 2));
			Size += Align - 1;
			size_t Pos = NormalizeBitPosition(Size);
			Size += 2;
			if (MainPtr->GetSize() <= Pos)  Data = ::malloc(Size);
			else
			{
				QueueNode<char> *Node = (*MainPtr)[Pos].Shift();
				if (Node == NULL)
				{
					if (Size < sizeof(QueueNode<char>))  Size = sizeof(QueueNode<char>);
					Data = ::malloc(Size);
					if (Data == NULL)  return NULL;

					Node = (QueueNode<char> *)Data;
				}

				Data = ((std::uint8_t *)Node);
			}

			Data = ((std::uint8_t *)Data) + 2;

			size_t x = (size_t)((std::uintptr_t)Data % Align);
			if (x)
			{
				x = Align - x;

				Data = ((std::uint8_t *)Data) + x;
			}

			// Store the offset.
			if (x < 128)  ((std::uint8_t *)Data)[-2] = (std::uint8_t)x;
			else
			{
				// Align storage for a 32-bit integer.
				Align = x;
				x = 2 + 4 + (size_t)((std::uintptr_t)(((std::uint8_t *)Data) - 2) % 4);
				((std::uint8_t *)Data)[-2] = (std::uint8_t)(128 + x);
				((std::uint32_t *)(((std::uint8_t *)Data) - x))[0] = (std::uint32_t)Align;
			}

			// Store the bit position.
			((std::uint8_t *)Data)[-1] = (std::uint8_t)Pos;

			return Data;
		}

		void *TLS::realloc(void *Data, size_t NewSize, size_t Align, bool Cache)
		{
			if (NewSize == 0)
			{
				if (Data != NULL)  free(Data, Cache);

				return NULL;
			}

			if (Data == NULL)  return malloc(NewSize, Align);

			// See if the new size fits within the current size and is aligned correctly.
			if (!Align)  Align = (NewSize > 4 ? 8 : (NewSize > 2 ? 4 : 2));
			size_t NewSize2 = NewSize + Align - 1;
			size_t NewPos = NormalizeBitPosition(NewSize2);
			size_t PrevPos = (size_t)((std::uint8_t *)Data)[-1];
			if (NewPos <= PrevPos && (std::uintptr_t)Data % Align == 0)  return Data;

			StaticVector<Queue<char>> *MainPtr = GetMainPtr();
			if (MainPtr == NULL)  return NULL;

			void *Data2;

			// Allocate data.
			Data2 = malloc(NewSize, Align);

			// Copy the data.
			memcpy(Data2, Data, (1 << PrevPos));

			// Free the previous object.
			free(Data, Cache);

			return Data2;
		}

		void *TLS::dup_malloc(void *Data, bool Cache)
		{
			if (Data == NULL)  return NULL;

			// Allocate the appropriate size buffer.
			size_t Size = (size_t)(1 << ((size_t)((std::uint8_t *)Data)[-1]));
			void *Data2 = ::malloc(Size);
			if (Data2 == NULL)  return NULL;

			// Copy the data.
			memcpy(Data2, Data, Size);

			// Free the previous object.
			free(Data, Cache);

			return Data2;
		}

		void TLS::free(void *Data, bool Cache)
		{
			if (Data == NULL)  return;

			StaticVector<Queue<char>> *MainPtr = GetMainPtr();
			if (MainPtr == NULL)  return;

			size_t Pos = (size_t)((std::uint8_t *)Data)[-1];

			size_t Offset = (size_t)((std::uint8_t *)Data)[-2];
			if (Offset >= 128)  Offset = (size_t)(((std::uint32_t *)(((std::uint8_t *)Data) - (Offset - 128)))[0]);
			Offset += 2;

			if (MainPtr->GetSize() <= Pos)  ::free(((std::uint8_t *)Data) - Offset);
			else
			{
				QueueNode<char> *Node = (QueueNode<char> *)(((std::uint8_t *)Data) - Offset);

				if (!Cache)  ::free(Node);
				else
				{
					// Placement new.  Instantiates QueueNode.
					Node = new(Node) QueueNode<char>;

					(*MainPtr)[Pos].Push(Node);
				}
			}
		}

		bool TLS::GetBucketInfo(size_t Num, size_t &Nodes, size_t &Size)
		{
			StaticVector<Queue<char>> *MainPtr = GetMainPtr();
			if (MainPtr == NULL)  return false;

			if (Num >= MainPtr->GetSize())  return false;
			else
			{
				Nodes = (*MainPtr)[Num].GetSize();
				Size = (size_t)(1 << Num);
				if (Size < sizeof(QueueNode<char>))  Size = sizeof(QueueNode<char>);
				Size *= Nodes;
			}

			return true;
		}

		bool TLS::ThreadEnd()
		{
			StaticVector<Queue<char>> *MainPtr = GetMainPtr();
			if (MainPtr == NULL)  return false;

			SetMainPtr(NULL);

			// Free all cached data.
			size_t y = MainPtr->GetSize();
			Queue<char> *RawData = MainPtr->RawData();
			QueueNode<char> *Node;
			for (size_t x = 0; x < y; x++)
			{
				while ((Node = RawData[x].Shift()) != NULL)
				{
					::free(Node);
				}
			}

			delete MainPtr;

			return true;
		}

		size_t TLS::NormalizeBitPosition(size_t &Size)
		{
			size_t Pos = 3;

			while (((size_t)1 << Pos) < Size)  Pos++;
			Size = ((size_t)1 << Pos);

			return Pos;
		}


		TLS::AutoFree::AutoFree(TLS *TLSPtr, void *Data)
		{
			MxTLS = TLSPtr;
			MxData = Data;
		}

		TLS::AutoFree::~AutoFree()
		{
			MxTLS->free(MxData);
		}


		TLS::MixedVar::MixedVar(TLS *TLSPtr) : MxMode(TMV_None), MxInt(0), MxDouble(0.0), MxStr(NULL), MxStrPos(0), MxTLS(TLSPtr)
		{
		}

		TLS::MixedVar::~MixedVar()
		{
			if (MxStr != NULL)  MxTLS->free(MxStr);
		}

		// Copy constructor.
		TLS::MixedVar::MixedVar(const TLS::MixedVar &TempVar)
		{
			MxTLS = TempVar.MxTLS;

			if (TempVar.MxStr != NULL)  SetData(TempVar.MxStr, TempVar.MxStrPos);
			else
			{
				MxStr = NULL;
				MxStrPos = 0;
			}

			MxMode = TempVar.MxMode;
			MxInt = TempVar.MxInt;
			MxDouble = TempVar.MxDouble;
		}

		// Assignment operator.
		TLS::MixedVar &TLS::MixedVar::operator=(const TLS::MixedVar &TempVar)
		{
			if (this != &TempVar)
			{
				MxTLS = TempVar.MxTLS;

				if (TempVar.MxStr != NULL)  SetData(TempVar.MxStr, TempVar.MxStrPos);
				else
				{
					MxTLS->free(MxStr);

					MxStr = NULL;
					MxStrPos = 0;
				}

				MxMode = TempVar.MxMode;
				MxInt = TempVar.MxInt;
				MxDouble = TempVar.MxDouble;
			}

			return *this;
		}

		void TLS::MixedVar::SetData(const char *str, size_t size)
		{
			MxMode = TMV_Str;
			if (MxStr != NULL)  MxTLS->free(MxStr);
			MxStr = (char *)MxTLS->malloc(size + 1, 1);
			memcpy(MxStr, str, size);
			MxStrPos = size;
			MxStr[MxStrPos] = '\0';
		}

		void TLS::MixedVar::SetStr(const char *str)
		{
			SetData(str, strlen(str));
		}

		void TLS::MixedVar::PrependData(const char *str, size_t size)
		{
			char *str2 = (char *)MxTLS->malloc(size + MxStrPos + 1, 1);
			memcpy(str2, str, size);
			memcpy(str2 + size, MxStr, MxStrPos);
			MxTLS->free(MxStr);
			MxStr = str2;
			MxStrPos += size;
			MxStr[MxStrPos] = '\0';
		}

		void TLS::MixedVar::PrependStr(const char *str)
		{
			PrependData(str, strlen(str));
		}

		void TLS::MixedVar::PrependInt(const std::int64_t val, size_t radix)
		{
			char tempbuffer[44];
			if (IntToString(tempbuffer, sizeof(tempbuffer), val, radix))  PrependStr(tempbuffer);
		}

		void TLS::MixedVar::PrependUInt(const std::uint64_t val, size_t radix)
		{
			char tempbuffer[44];
			if (IntToString(tempbuffer, sizeof(tempbuffer), val, radix))  PrependStr(tempbuffer);
		}

		void TLS::MixedVar::PrependDouble(const double val, const size_t precision)
		{
			char tempbuffer[100];
#if (defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)) && defined(_MSC_VER) && _MSC_VER < 1900
			_snprintf_s(tempbuffer, sizeof(tempbuffer), _TRUNCATE, "%1.*g", precision, val);
			tempbuffer[sizeof(tempbuffer) - 1] = '\0';
#else
			snprintf(tempbuffer, sizeof(tempbuffer), "%1.*g", (int)precision, val);
#endif

			PrependStr(tempbuffer);
		}

		void TLS::MixedVar::AppendData(const char *str, size_t size)
		{
			MxStr = (char *)MxTLS->realloc(MxStr, MxStrPos + size + 1, 1);
			memcpy(MxStr + MxStrPos, str, size);
			MxStrPos += size;
			MxStr[MxStrPos] = '\0';
		}

		void TLS::MixedVar::AppendStr(const char *str)
		{
			AppendData(str, strlen(str));
		}

		void TLS::MixedVar::AppendInt(const std::int64_t val, size_t radix)
		{
			char tempbuffer[44];
			if (IntToString(tempbuffer, sizeof(tempbuffer), val, radix))  AppendStr(tempbuffer);
		}

		void TLS::MixedVar::AppendUInt(const std::uint64_t val, size_t radix)
		{
			char tempbuffer[44];
			if (IntToString(tempbuffer, sizeof(tempbuffer), val, radix))  AppendStr(tempbuffer);
		}

		void TLS::MixedVar::AppendDouble(const double val, const size_t precision)
		{
			char tempbuffer[100];
#if (defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)) && defined(_MSC_VER) && _MSC_VER < 1900
			_snprintf_s(tempbuffer, sizeof(tempbuffer), _TRUNCATE, "%1.*g", precision, val);
			tempbuffer[sizeof(tempbuffer) - 1] = '\0';
#else
			snprintf(tempbuffer, sizeof(tempbuffer), "%1.*g", (int)precision, val);
#endif

			AppendStr(tempbuffer);
		}

		void TLS::MixedVar::AppendChar(char chr)
		{
			MxStr = (char *)MxTLS->realloc(MxStr, MxStrPos + 2, 1);
			MxStr[MxStrPos++] = chr;
			MxStr[MxStrPos] = '\0';
		}

		void TLS::MixedVar::AppendMissingChar(char chr)
		{
			if (!MxStrPos || MxStr[MxStrPos - 1] != chr)  AppendChar(chr);
		}

		void TLS::MixedVar::SetSize(size_t pos)
		{
			if (MxStrPos < pos)  MxStr = (char *)MxTLS->realloc(MxStr, pos + 1, 1);

			MxStrPos = pos;
			MxStr[MxStrPos] = '\0';
		}

		size_t TLS::MixedVar::ReplaceData(const char *src, size_t srcsize, const char *dest, size_t destsize)
		{
			size_t Num;

			if (srcsize < destsize)
			{
				char *Result;
				size_t ResultSize;

				Num = FastReplaceAlloc<char, TLS>::ReplaceAll(Result, ResultSize, MxStr, MxStrPos, src, srcsize, dest, destsize, MxTLS);

				MxTLS->free(MxStr);
				MxStr = Result;
				MxStrPos = ResultSize;
			}
			else
			{
				Num = FastReplaceAlloc<char, TLS>::StaticReplaceAll(MxStr, MxStrPos, MxStr, MxStrPos, src, srcsize, dest, destsize, MxTLS);
			}

			MxStr[MxStrPos] = '\0';

			return Num;
		}

		size_t TLS::MixedVar::ReplaceStr(const char *src, const char *dest)
		{
			return ReplaceData(src, strlen(src), dest, strlen(dest));
		}

		// Swiped and slightly modified from Int::ToString().
		bool TLS::MixedVar::IntToString(char *Result, size_t Size, std::uint64_t Num, size_t Radix)
		{
			if (Size < 2)  return false;

			size_t x = Size, z;

			Result[--x] = '\0';
			if (!Num)  Result[--x] = '0';
			else
			{
				while (Num && x)
				{
					z = Num % Radix;
					Result[--x] = (char)(z > 9 ? z - 10 + 'A' : z + '0');
					Num /= Radix;
				}

				if (Num)  return false;
			}

			memmove(Result, Result + x, Size - x);

			return true;
		}

		bool TLS::MixedVar::IntToString(char *Result, size_t Size, std::int64_t Num, size_t Radix)
		{
			if (Num >= 0)  return IntToString(Result, Size, (std::uint64_t)Num, Radix);

			if (Size < 2)  return false;
			Result[0] = '-';

			return IntToString(Result + 1, Size - 1, (std::uint64_t)-Num, Radix);
		}


		// Wide character MixedVar.
		TLS::WCMixedVar::WCMixedVar(TLS *TLSPtr) : MxMode(TMV_None), MxInt(0), MxDouble(0.0), MxStr(NULL), MxStrPos(0), MxTLS(TLSPtr)
		{
		}

		TLS::WCMixedVar::~WCMixedVar()
		{
			if (MxStr != NULL)  MxTLS->free(MxStr);
		}

		// Copy constructor.
		TLS::WCMixedVar::WCMixedVar(const TLS::WCMixedVar &TempVar)
		{
			MxTLS = TempVar.MxTLS;

			MxStr = NULL;
			MxStrPos = 0;
			if (TempVar.MxStr != NULL)
			{
				SetSize(TempVar.MxStrPos);
				memcpy(MxStr, TempVar.MxStr, MxStrPos * sizeof(WCHAR));
			}

			MxMode = TempVar.MxMode;
			MxInt = TempVar.MxInt;
			MxDouble = TempVar.MxDouble;
		}

		// Assignment operator.
		TLS::WCMixedVar &TLS::WCMixedVar::operator=(const TLS::WCMixedVar &TempVar)
		{
			if (this != &TempVar)
			{
				MxTLS = TempVar.MxTLS;

				if (TempVar.MxStr != NULL)
				{
					SetSize(TempVar.MxStrPos);
					memcpy(MxStr, TempVar.MxStr, MxStrPos * sizeof(WCHAR));
				}
				else
				{
					MxTLS->free(MxStr);

					MxStr = NULL;
					MxStrPos = 0;
				}

				MxMode = TempVar.MxMode;
				MxInt = TempVar.MxInt;
				MxDouble = TempVar.MxDouble;
			}

			return *this;
		}

		void TLS::WCMixedVar::SetStr(const WCHAR *str)
		{
			MxMode = TMV_Str;
			size_t y = wcslen(str) + 1;
			size_t y2 = y * sizeof(WCHAR);

			if (MxStr != NULL)  MxTLS->free(MxStr);
			MxStr = (WCHAR *)MxTLS->malloc(y2, sizeof(WCHAR));
			memcpy(MxStr, str, y2);
			MxStrPos = y - 1;
		}

		// Doesn't do anything fancy beyond expanding characters to fill the space of a wide character.
		void TLS::WCMixedVar::SetStr(const char *str)
		{
			MxMode = TMV_Str;
			size_t y = strlen(str) + 1;
			size_t y2 = y * sizeof(WCHAR);

			if (MxStr != NULL)  MxTLS->free(MxStr);
			MxStr = (WCHAR *)MxTLS->malloc(y2, sizeof(WCHAR));
			for (size_t x = 0; x < y; x++)  MxStr[x] = (WCHAR)*str++;
			MxStrPos = y - 1;
		}

		void TLS::WCMixedVar::PrependStr(const WCHAR *str)
		{
			size_t y = wcslen(str);

			WCHAR *str2 = (WCHAR *)MxTLS->malloc((y + MxStrPos + 1) * sizeof(WCHAR), sizeof(WCHAR));
			memcpy(str2, str, y * sizeof(WCHAR));
			memcpy(str2 + y, MxStr, (MxStrPos + 1) * sizeof(WCHAR));
			MxTLS->free(MxStr);

			MxStr = str2;
			MxStrPos += y;
		}

		// Doesn't do anything fancy beyond expanding characters to fill the space of a wide character.
		void TLS::WCMixedVar::PrependStr(const char *str)
		{
			size_t y = strlen(str);

			WCHAR *str2 = (WCHAR *)MxTLS->malloc((y + MxStrPos + 1) * sizeof(WCHAR), sizeof(WCHAR));
			for (size_t x = 0; x < y; x++)  str2[x] = (WCHAR)*str++;
			memcpy(str2 + y, MxStr, (MxStrPos + 1) * sizeof(WCHAR));
			MxTLS->free(MxStr);

			MxStr = str2;
			MxStrPos += y;
		}

		void TLS::WCMixedVar::PrependInt(const std::int64_t val, size_t radix)
		{
			char tempbuffer[44];
			if (TLS::MixedVar::IntToString(tempbuffer, sizeof(tempbuffer), val, radix))  PrependStr(tempbuffer);
		}

		void TLS::WCMixedVar::PrependUInt(const std::uint64_t val, size_t radix)
		{
			char tempbuffer[44];
			if (TLS::MixedVar::IntToString(tempbuffer, sizeof(tempbuffer), val, radix))  PrependStr(tempbuffer);
		}

		void TLS::WCMixedVar::PrependDouble(const double val, const size_t precision)
		{
			char tempbuffer[100];
#if (defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)) && defined(_MSC_VER) && _MSC_VER < 1900
			_snprintf_s(tempbuffer, sizeof(tempbuffer), _TRUNCATE, "%1.*g", precision, val);
			tempbuffer[sizeof(tempbuffer) - 1] = '\0';
#else
			snprintf(tempbuffer, sizeof(tempbuffer), "%1.*g", (int)precision, val);
#endif

			PrependStr(tempbuffer);
		}

		void TLS::WCMixedVar::AppendStr(const WCHAR *str)
		{
			size_t y = wcslen(str);

			MxStr = (WCHAR *)MxTLS->realloc(MxStr, (MxStrPos + y + 1) * sizeof(WCHAR), sizeof(WCHAR));
			memcpy(MxStr + MxStrPos, str, (y + 1) * sizeof(WCHAR));
			MxStrPos += y;
		}

		// Doesn't do anything fancy beyond expanding characters to fill the space of a wide character.
		void TLS::WCMixedVar::AppendStr(const char *str)
		{
			size_t y = strlen(str);

			MxStr = (WCHAR *)MxTLS->realloc(MxStr, (MxStrPos + y + 1) * sizeof(WCHAR), sizeof(WCHAR));
			while (*str)  MxStr[MxStrPos++] = (WCHAR)*str++;
			MxStr[MxStrPos] = L'\0';
		}

		void TLS::WCMixedVar::AppendInt(const std::int64_t val, size_t radix)
		{
			char tempbuffer[44];
			if (TLS::MixedVar::IntToString(tempbuffer, sizeof(tempbuffer), val, radix))  AppendStr(tempbuffer);
		}

		void TLS::WCMixedVar::AppendUInt(const std::uint64_t val, size_t radix)
		{
			char tempbuffer[44];
			if (TLS::MixedVar::IntToString(tempbuffer, sizeof(tempbuffer), val, radix))  AppendStr(tempbuffer);
		}

		void TLS::WCMixedVar::AppendDouble(const double val, const size_t precision)
		{
			char tempbuffer[100];
#if (defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)) && defined(_MSC_VER) && _MSC_VER < 1900
			_snprintf_s(tempbuffer, sizeof(tempbuffer), _TRUNCATE, "%1.*g", precision, val);
			tempbuffer[sizeof(tempbuffer) - 1] = '\0';
#else
			snprintf(tempbuffer, sizeof(tempbuffer), "%1.*g", (int)precision, val);
#endif

			AppendStr(tempbuffer);
		}

		void TLS::WCMixedVar::AppendChar(WCHAR chr)
		{
			MxStr = (WCHAR *)MxTLS->realloc(MxStr, (MxStrPos + 2) * sizeof(WCHAR), sizeof(WCHAR));
			MxStr[MxStrPos++] = chr;
			MxStr[MxStrPos] = L'\0';
		}

		void TLS::WCMixedVar::AppendMissingChar(WCHAR chr)
		{
			if (!MxStrPos || MxStr[MxStrPos - 1] != chr)  AppendChar(chr);
		}

		void TLS::WCMixedVar::SetSize(size_t pos)
		{
			if (MxStrPos < pos)  MxStr = (WCHAR *)MxTLS->realloc(MxStr, pos + 1, sizeof(WCHAR));

			MxStrPos = pos;
			MxStr[MxStrPos] = L'\0';
		}

		size_t TLS::WCMixedVar::ReplaceStr(const WCHAR *src, const WCHAR *dest)
		{
			size_t srcsize = wcslen(src);
			size_t destsize = wcslen(dest);
			size_t Num;

			if (srcsize < destsize)
			{
				WCHAR *Result;
				size_t ResultSize;

				Num = FastReplaceAlloc<WCHAR, TLS>::ReplaceAll(Result, ResultSize, MxStr, MxStrPos, src, srcsize, dest, destsize, MxTLS);

				MxTLS->free(MxStr);
				MxStr = Result;
				MxStrPos = ResultSize;
			}
			else
			{
				Num = FastReplaceAlloc<WCHAR, TLS>::StaticReplaceAll(MxStr, MxStrPos, MxStr, MxStrPos, src, srcsize, dest, destsize, MxTLS);
			}

			MxStr[MxStrPos] = L'\0';

			return Num;
		}
	}
}
