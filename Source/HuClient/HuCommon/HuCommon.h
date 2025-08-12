#pragma once

#include "CoreMinimal.h"
#include <vector>
#include <string>
#include <type_traits>

namespace hu {

using AChar   = char;
using AString = std::string;

using WChar   = wchar_t;
using WString = std::wstring;

using Char   = WChar;
using String = WString;

using Byte   = uint8;
using Buffer = std::vector<Byte>;

using TimeNumber = uint64;

using ConnectId  = uint32;
using MessageId  = uint32;
using ResultCode = uint32;

using UUId = std::string;

template <typename T>
concept SerialType = std::is_aggregate_v<T>;

} // hu

using HuMessageSize = uint32;
using HuMessageId = uint32;
using HuByte = uint8;
using HuBuffer = TArray<HuByte>;

DECLARE_LOG_CATEGORY_EXTERN( HuLogCommon, Log, All );

bool HuCommonLog( const bool Cond, const bool Result, const TCHAR* const CondStr, const TCHAR* const Function, const int32 Line );

void HuCommonLog( const TCHAR* const Function, const int32 Line );

#define HU_STATIC_ASSERT( cond ) static_assert( cond, #cond )
#define HU_CHECK_TRUE( Cond ) HuCommonLog( Cond, true, TEXT( #Cond ), TEXT(__FUNCTION__), __LINE__ )
#define HU_CHECK_FALSE( Cond ) HuCommonLog( Cond, false, TEXT( #Cond ), TEXT(__FUNCTION__), __LINE__ )
#define HU_LOG_SUCCESS() HuCommonLog( TEXT(__FUNCTION__), __LINE__ )
