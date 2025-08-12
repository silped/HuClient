#include "HuCommon.h"

DEFINE_LOG_CATEGORY( HuLogCommon );

bool HuCommonLog( const bool Cond, const bool Result, const TCHAR* const CondStr, const TCHAR* const Function, const int32 Line )
{
    if ( Cond == Result )
    {
        const FString TimeStr = FDateTime::Now().ToString( TEXT( "%Y-%m-%d %H:%M:%S" ) );
        UE_LOG( HuLogCommon, Error, TEXT( "[%s] %s(%d) : %s" ), *TimeStr, Function, Line, CondStr );

        if ( GEngine )
        {
            GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Red, FString::Printf( TEXT( "%s(%d) : %s" ), Function, Line, CondStr ) );
        }
    }

    return Cond;
}

void HuCommonLog( const TCHAR* const Function, const int32 Line )
{
    const FString TimeStr = FDateTime::Now().ToString( TEXT( "%Y-%m-%d %H:%M:%S" ) );
    UE_LOG( HuLogCommon, Log, TEXT( "[%s] %s(%d) : Success" ), *TimeStr, Function, Line );

    if ( GEngine )
    {
        GEngine->AddOnScreenDebugMessage( -1, 3.0f, FColor::Green, FString::Printf( TEXT( "%s(%d) : Success" ), Function, Line ) );
    }
}
