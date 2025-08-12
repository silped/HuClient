#pragma once

#include "HuSerial.h"


namespace hu {

enum EMessageId : MessageId
{
    kVer = 2025 + 06 + 25,

    kAuthInfo,
    kUserInfo,

    kLoginReq,
    kLoginRes
};

enum EResultCode : ResultCode
{
    kResSuccess = 0,
    kResFail,
    kResFailToHandle,
    kResLogined,
    kResNotLogined
};

struct AuthInfo
{
    Buffer    key;
    ConnectId conn_id { 0 };

    HU_USE_SERIAL( AuthInfo, key, conn_id );
};

struct UserInfo
{
    UUId       id;
    TimeNumber login_time { 0 };
    TimeNumber logout_time { 0 };
    TimeNumber create_time { 0 };

    HU_USE_SERIAL( UserInfo, id, login_time, logout_time, create_time );
};

struct LoginReq
{
    UUId     user_id;
    AuthInfo auth;

    HU_USE_SERIAL( LoginReq, user_id, auth );
};

struct LoginRes
{
    ResultCode code { kResSuccess };
    UserInfo   user;
    AuthInfo   auth;

    HU_USE_SERIAL( LoginRes, code, user, auth );
};

} // hu
