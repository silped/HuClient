#pragma once

#include "CoreMinimal.h"
#include "HuCommon.h"
#include <yas/object.hpp>
#include <yas/serialize.hpp>
#include <yas/std_types.hpp>

#define HU_USE_SERIAL( type, ... ) \
    static constexpr hu::MessageId kTypeId { k##type }; \
    YAS_DEFINE_STRUCT_SERIALIZE( #type, __VA_ARGS__ )

namespace hu {

template <auto kOption>
class Serializer final
{
public:
    template <SerialType T>
    static bool Write(
        const T& obj,
        Buffer&  buffer
    )
    {
        try
        {
            const auto buffer_size = yas::saved_size<kOption>( obj );
            if ( buffer_size <= 0 )
                return false;

            buffer.clear();
            buffer.reserve( buffer_size );

            yas::save<kFlag>( yas::vector_ostream<Byte>( buffer ), obj );
            return true;
        }
        catch ( const std::exception& )
        {
            return false;
        }
    }

    template <SerialType T>
    static bool Read(
        const Buffer& buffer,
        T&            obj
    )
    {
        try
        {
            yas::load<kFlag>( buffer, obj );
            return true;
        }
        catch ( const std::exception& )
        {
            return false;
        }
    }

private:
    static constexpr uint32 kFlag = ( kOption | yas::mem );
};

using BinSerializer  = Serializer<yas::binary>;
using JsonSerializer = Serializer<yas::json>;

} // hu
