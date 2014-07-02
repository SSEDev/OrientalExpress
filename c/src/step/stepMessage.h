/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/*
 * @file    stepMessage.h
 *
 * STEP协议定义头文件
 *
 * @version $Id
 * @since   2013/10/19
 * @author  Jin Chengxun
 *    
 */

/*
 MODIFICATION HISTORY:
 <pre>
 ================================================================================
 DD-MMM-YYYY INIT.    SIR    Modification Description
 ----------- -------- ------ ----------------------------------------------------
 19-10-2013  CXJIN           创建
 ================================================================================
  </pre>
*/

#ifndef EPS_STEP_MESSAGE_H
#define EPS_STEP_MESSAGE_H

/**
 * 包含头文件
 */

#include "epsTypes.h"


#ifdef __cplusplus
extern "C" {
#endif

/*
 * STEP字段长度定义
 */
#define STEP_MSGTYPE_MAX_LEN                        6
#define STEP_COMPID_MAX_LEN                         20
#define STEP_TIMESTAMP_MAX_LEN                      8
#define STEP_MSGDATA_MAX_LEN                        8192
#define STEP_USERNAME_MAX_LEN                       10
#define STEP_PASSWORD_MAX_LEN                       10
#define STEP_DATE_LEN                               8
#define STEP_TIME_LEN                               8
#define STEP_CHECKSUM_MAX_LEN                       4
#define STEP_SECURITY_TYPE_LEN                      2
#define STEP_MD_UPDATETYPE_LEN                      3
#define STEP_MD_DATA_MAX_LEN                        4096
#define STEP_MSGENCODING_MAX_LEN                    8
#define STEP_CHECKSUM_LEN                           3
#define STEP_TEXT_MAX_LEN                           30
#define STEP_MSG_MAX_LEN                            4096

#define STEP_CHECKSUM_FIELD_LEN                     7
#define STEP_MSG_BODY_MAX_LEN                       4096
#define STEP_MSG_MAX_LEN                            4096        
#define STEP_MSG_MIN_LEN                            50
#define STEP_MD_MSG_WRAP_SIZE                       200         



/*
 * STEP消息类型值定义
 */
#define STEP_MSGTYPE_LOGON_VALUE                    "A"
#define STEP_MSGTYPE_LOGOUT_VALUE                   "5"
#define STEP_MSGTYPE_HEARTBEAT_VALUE                "0"
#define STEP_MSGTYPE_MD_REQUEST_VALUE               "V"
#define STEP_MSGTYPE_MD_SNAPSHOT_VALUE              "W"
#define STEP_MSGTYPE_TRADING_STATUS_VALUE           "h"


/*
 * STEP字段TAG定义
 */

#define STEP_BEGIN_STRING_TAG                       8
#define STEP_BODY_LENGTH_TAG                        9
#define STEP_CHECKSUM_TAG                           10
#define STEP_MSG_SEQ_NUM_TAG                        34
#define STEP_MSG_TYPE_TAG                           35
#define STEP_REF_SEQ_NUM_TAG                        45 
#define STEP_SENDER_COMP_ID_TAG                     49 
#define STEP_SENDING_TIME_TAG                       52 
#define STEP_TARGET_COMP_ID_TAG                     56 
#define STEP_TEXT_TAG                               58 
#define STEP_TRADE_DATE_TAG                         75 
#define STEP_RAWDATA_LENGTH_TAG                     95 
#define STEP_RAWDATA_TAG                            96
#define STEP_ENCRYPT_METHOD_TAG                     98
#define STEP_HEARTBT_INT_TAG                        108
#define STEP_SECURITY_TYPE_TAG                      167
#define STEP_MD_UPDATETYPE_TAG                      265
#define STEP_TRADE_SES_MODE_TAG                     339
#define STEP_MSG_ENCODING_TAG                       347
#define STEP_SESSION_REJECT_REASON_TAG              373
#define STEP_USERNAME_TAG                           553 
#define STEP_PASSWORD_TAG                           554
#define STEP_LAST_UPDATETIME_TAG                    779
#define STEP_APPL_ID_TAG                            1180
#define STEP_APPL_SEQ_NUM_TAG                       1181
#define STEP_MD_COUNT_TAG                           5468

/*
 * STEP字符常量定义
 */
 
#define STEP_BEGIN_STRING_VALUE                     "STEP.1.0.0"
#define STEP_SENDER_COMPID_VALUE                    "EzEI"
#define STEP_TARGET_COMPID_VALUE                    "EzSR"
#define STEP_MSG_ENCODING_VALUE                     "GBK"

#define STEP_SECURITY_TYPE_ALL_VALUE                "00"
#define STEP_SECURITY_TYPE_STK_VALUE                "01"
#define STEP_SECURITY_TYPE_DEV_VALUE                "02"

#define STEP_INVALID_STRING_VALUE                   ""
#define STEP_INVALID_INT_VALUE                      -1
#define STEP_INVALID_UINT_VALUE                     -1


/*
 * STEP消息类型枚举
 */
typedef enum StepMsgTypeTag
{
    STEP_MSGTYPE_INVALID          = -1, /* 无效消息类型 */             
    STEP_MSGTYPE_LOGON            = 0,  /* 登陆消息, 'A' */
    STEP_MSGTYPE_LOGOUT           = 1,  /* 登出消息, '5' */
    STEP_MSGTYPE_HEARTBEAT        = 2,  /* 心跳消息, '0' */
    STEP_MSGTYPE_MD_REQUEST       = 3,  /* 订阅消息, 'V' */
    STEP_MSGTYPE_MD_SNAPSHOT      = 4,  /* 全幅行情消息, 'W' */
    STEP_MSGTYPE_TRADING_STATUS   = 5,  /* 市场状态消息, 'h' */
    STEP_MSGTYPE_COUNT
} StepMsgTypeT;

/*
 * STEP消息加密枚举
 */
typedef enum StepEncryptMethodTag
{
    STEP_ENCRYPT_METHOD_NONE    = '0',  /* 无加密 */
} StepEncryptMethodT;


/*
 * STEP消息传输方向枚举
 */
typedef enum StepDirectionTag
{
    STEP_DIRECTION_REQ          = 0,     /* 请求，客户端 -> 服务端 */
    STEP_DIRECTION_RSP          = 1,     /* 应答，服务端 -> 客户端 */
    STEP_DIRECTION_DAT          = 2,     /* 数据，服务端 -> 客户端 */
} StepDirectionT;

/*
 * STEP消息结构
 */
typedef struct StepMessageTag
{
    StepMsgTypeT msgType;
    char    senderCompID[STEP_COMPID_MAX_LEN+1];
    char    targetCompID[STEP_COMPID_MAX_LEN+1];
    uint64  msgSeqNum;
    char    sendingTime[STEP_TIME_LEN+1];
    char    msgEncoding[STEP_MSGENCODING_MAX_LEN+1];
    char    body[STEP_MSGDATA_MAX_LEN+1];
} StepMessageT;

/*
 * 登陆消息体结构
 */
typedef struct LogonRecordTag
{
    char    encryptMethod;
    uint16  heartBtInt;
    char    username[STEP_USERNAME_MAX_LEN+1];
    char    password[STEP_PASSWORD_MAX_LEN+1];
} LogonRecordT;

/*
 * 登出消息体结构
 */
typedef struct LogoutRecordTag
{
    char    text[STEP_TEXT_MAX_LEN+1];
} LogoutRecordT;

/*
 * 行情订阅消息体结构
 */
typedef struct MDRequestRecordTag
{
    char    securityType[STEP_SECURITY_TYPE_LEN+1];
} MDRequestRecordT;

/*
 * 全幅行情消息体结构
 */
typedef struct MDSnapshotFullRefreshRecordTag
{
    char    securityType[STEP_SECURITY_TYPE_LEN+1];
    int16   tradSesMode;
    uint32  applID;
    uint64  applSeqNum;
    char    tradeDate[STEP_DATE_LEN+1];
    char    lastUpdateTime[STEP_TIME_LEN+1];
    char    mdUpdateType[STEP_MD_UPDATETYPE_LEN+1];
    uint32  mdCount;
    uint32  mdDataLen;
    char    mdData[STEP_MD_DATA_MAX_LEN+1];
} MDSnapshotFullRefreshRecordT;


#ifdef __cplusplus
}
#endif

#endif /* EPS_STEP_MESSAGE_H */
