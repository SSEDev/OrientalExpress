/*
 * Express Project Studio, Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    stepEncoder.c
 *
 * STEP解码器实现文件 
 *
 * @version $Id
 * @since   2013/11/12
 * @author  Jin Chengxun
 *    
 */

/*
 MODIFICATION HISTORY:
 <pre>
 ================================================================================
 DD-MMM-YYYY INIT.    SIR    Modification Description
 ----------- -------- ------ ----------------------------------------------------
 12-11-2013  CXJIN           创建
 ================================================================================
  </pre>
*/

/*
 * 包含头文件
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "errlib.h"
#include "epsTypes.h"
#include "stepCodecUtil.h"

#include "stepCodec.h"

/*
 * 全局定义
 */

/*
 * STEP消息头解析占位宏
 */
#define DECODE_STEP_MSG_HEADER_STUB \
    case STEP_SENDER_COMP_ID_TAG:\
    {\
        STEP_EXTRACT_STRING_VALUE(field, \
            pMsg->senderCompID, (int32)sizeof(pMsg->senderCompID));\
        break;\
    }\
    case STEP_TARGET_COMP_ID_TAG:\
    {\
        STEP_EXTRACT_STRING_VALUE(field, \
                pMsg->targetCompID, (int32)sizeof(pMsg->targetCompID));\
        break;\
    }\
    case STEP_MSG_SEQ_NUM_TAG:\
    {\
        STEP_EXTRACT_INT_VALUE(field, uint64, pMsg->msgSeqNum);\
        break;\
    }\
    case STEP_SENDING_TIME_TAG:\
    {\
        STEP_EXTRACT_STRING_VALUE(field, \
                pMsg->sendingTime, (int32)sizeof(pMsg->sendingTime));\
        break;\
    }\
    case STEP_MSG_ENCODING_TAG:\
    {\
        STEP_EXTRACT_STRING_VALUE(field, \
                pMsg->msgEncoding, (int32)sizeof(pMsg->msgEncoding));\
        break;\
    }


/*
 * 内部函数申明
 */
 
static ResCodeT DecodeStepMessageBody(const char* buf, int32 bufSize, 
        StepMessageT* pMsg);
static ResCodeT DecodeLogonDataRecord(const char* buf, int32 bufSize, 
        StepMessageT* pMsg);
static ResCodeT DecodeLogoutRecord(const char* buf, int32 bufSize, 
        StepMessageT* pMsg);
static ResCodeT DecodeHeartBeatRecord(const char* buf, int32 bufSize, 
        StepMessageT* pMsg);
static ResCodeT DecodeMDRequestRecord(const char* buf, int32 bufSize, 
        StepMessageT* pMsg);
static ResCodeT DecodeMDSnapshotFullRefreshRecord(const char* buf, int32 bufSize, 
        StepMessageT* pMsg);

/*
 * 函数实现
 */

/**
 * 解码STEP消息
 *
 * @param   buf             in  - 解码缓冲区
 * @param   bufSize         in  - 解码缓冲区长度
 * @param   pMsg            out - STEP消息
 * @param   pDecodeSize     out - 解码长度
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT DecodeStepMessage(const char* buf, int32 bufSize, 
        StepMessageT* pMsg, int32* pDecodeSize)
{
    TRY
    {
        int32 bufOffset = 0;
        StepFieldT field;
    
        if (bufSize < STEP_MSG_MIN_LEN)
        {
            THROW_RESCODE(ERCD_STEP_STREAM_NOT_ENOUGH);
        }

        /* 解析字段8(BEGIN_STRING) */
        THROW_ERROR(GetTextField(buf, bufSize, &field, &bufOffset));

        if (field.tag != STEP_BEGIN_STRING_TAG)
        {
            THROW_ERROR(ERCD_STEP_INVALID_MSGFORMAT, 
                "The first field tag isn't 8(BeginString)");
        }
        if (field.valueSize != (sizeof(STEP_BEGIN_STRING_VALUE)-1) ||
            strncmp(STEP_BEGIN_STRING_VALUE, field.value, field.valueSize) != 0)
        {
            THROW_ERROR(ERCD_STEP_INVALID_FLDVALUE, 
                field.tag, field.valueSize, field.value);
        }
        
        /* 解析字段9(BODY_LENGTH) */
        THROW_ERROR(GetTextField(buf, bufSize, &field, &bufOffset));
        if (field.tag != STEP_BODY_LENGTH_TAG)
        {
            THROW_ERROR(ERCD_STEP_INVALID_MSGFORMAT, 
                "The second field tag isn't 9(BodyLength)");
        }

        int32 bodyLen = 0;
        STEP_EXTRACT_INT_VALUE(field, int32, bodyLen);
        if (bodyLen > STEP_MSG_BODY_MAX_LEN)
        {
            THROW_ERROR(ERCD_STEP_INVALID_FLDVALUE, 
                field.tag, field.valueSize, field.value, "BodyLength overflow");
        }
        
        int32 msgLen = bufOffset + bodyLen + STEP_CHECKSUM_FIELD_LEN;
        if (bufSize < msgLen)
        {
            THROW_RESCODE(ERCD_STEP_STREAM_NOT_ENOUGH);
        }

        /* 解析字段10(CHECKSUM)，检查校验和 */
        int32 checksumOffset = bufOffset + bodyLen;
        THROW_ERROR(GetTextField(buf, bufSize, &field, &checksumOffset));
        if (field.tag != STEP_CHECKSUM_TAG)
        {
           THROW_ERROR(ERCD_STEP_INVALID_MSGFORMAT, 
                "The last field tag isn't 10(Checksum)");
        }
        if (field.valueSize != STEP_CHECKSUM_LEN)
        {
            THROW_ERROR(ERCD_STEP_INVALID_FLDVALUE, 
                field.tag, field.valueSize, field.value, "Invalid Checksum length");
        }
        STEP_CHECK_NUMBERONLY_TEXT(field);

        char calcChecksum[STEP_CHECKSUM_LEN+1] = {0};
        THROW_ERROR(CalcChecksum(buf, bufOffset+bodyLen, calcChecksum));
        if(memcmp(calcChecksum, field.value, STEP_CHECKSUM_LEN) != 0)
        {
            THROW_ERROR(ERCD_STEP_CHECKSUM_FAILED, field.value, calcChecksum);
        }

        /* 解析消息体 */
        THROW_ERROR(DecodeStepMessageBody(buf+bufOffset, bodyLen, pMsg));

        *pDecodeSize = msgLen;
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/**
 * 解码STEP消息体
 *
 * @param   buf             in  - 解码缓冲区
 * @param   bufSize         in  - 解码缓冲区长度
 * @param   pMsg            out - STEP消息
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
static ResCodeT DecodeStepMessageBody(const char* buf, int32 bufSize, StepMessageT* pMsg)
{
    TRY
    {
        /* STEP消息初始化模板 */
        static const StepMessageT STEP_MSG_TEMPLATE = 
        {
            STEP_MSGTYPE_INVALID,       /* msgType */
            STEP_INVALID_STRING_VALUE,  /* senderCompID */
            STEP_INVALID_STRING_VALUE,  /* targetCompID */
            STEP_INVALID_UINT_VALUE,    /* msgSeqNum */
            STEP_INVALID_STRING_VALUE,  /* sendingTime */
            STEP_MSG_ENCODING_VALUE,    /* msgEncoding */
            {0}                         /* body */
        };

        StepFieldT field;
        int bufOffset = 0;
 
        /* 解析字段35(MsgType) */
        THROW_ERROR(GetTextField(buf, bufSize, &field, &bufOffset));

        if (field.tag != STEP_MSG_TYPE_TAG)
        {
            THROW_ERROR(ERCD_STEP_INVALID_MSGFORMAT, 
                "The third field tag isn't 35(MsgType)");
        }

        char msgType[STEP_MSGTYPE_MAX_LEN+1] = {0};
        STEP_EXTRACT_STRING_VALUE(field, msgType, (int)sizeof(msgType));

        memcpy(pMsg, &STEP_MSG_TEMPLATE, sizeof(StepMessageT));
        
        if (strncmp(STEP_MSGTYPE_LOGON_VALUE, msgType, sizeof(msgType)) == 0)
        {
            THROW_ERROR(DecodeLogonDataRecord((char*)buf + bufOffset, 
                    bufSize - bufOffset, pMsg));
        }
        else if (strncmp(STEP_MSGTYPE_LOGOUT_VALUE, msgType, sizeof(msgType)) == 0)
        {
            THROW_ERROR(DecodeLogoutRecord((char*)buf + bufOffset, 
                    bufSize - bufOffset, pMsg));
        }
        else if (strncmp(STEP_MSGTYPE_HEARTBEAT_VALUE, msgType, sizeof(msgType)) == 0)
        {
            THROW_ERROR(DecodeHeartBeatRecord((char*)buf + bufOffset, 
                    bufSize - bufOffset, pMsg));
        }
        else if (strncmp(STEP_MSGTYPE_MD_REQUEST_VALUE, msgType, sizeof(msgType)) == 0)
        {
            THROW_ERROR(DecodeMDRequestRecord((char*)buf + bufOffset, 
                    bufSize - bufOffset, pMsg));
        }
        else if (strncmp(STEP_MSGTYPE_MD_SNAPSHOT_VALUE, msgType, sizeof(msgType)) == 0)
        {
            THROW_ERROR(DecodeMDSnapshotFullRefreshRecord((char*)buf + bufOffset, 
                bufSize - bufOffset, pMsg));
        }
        else if (strncmp(STEP_MSGTYPE_TRADING_STATUS_VALUE, msgType, sizeof(msgType)) == 0)
        {
        }
        else
        {
            THROW_ERROR(ERCD_STEP_INVALID_MSGTYPE, msgType);
        }
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/*
 * 解码STEP登陆消息
 *
 * @param   buf             in  - 解码缓冲区
 * @param   bufSize         in  - 解码缓冲区长度
 * @param   pMsg            out - STEP消息
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
static ResCodeT DecodeLogonDataRecord(const char* buf, int32 bufSize, StepMessageT* pMsg)
{
    TRY
    {
        /* 登陆请求记录初始化模板 */
        static const LogonRecordT STEP_LOGON_RECORD_TEMPLATE = 
        {
            STEP_INVALID_INT_VALUE,     /* encryptMethod */
            STEP_INVALID_UINT_VALUE,    /* heartBtInt */
            STEP_INVALID_STRING_VALUE,  /* username */
            STEP_INVALID_STRING_VALUE,  /* password */
        };

        pMsg->msgType = STEP_MSGTYPE_LOGON;

        StepFieldT field;
        int32 bufOffset = 0;
        LogonRecordT* pRecord = (LogonRecordT*)pMsg->body;
        memcpy(pRecord, &STEP_LOGON_RECORD_TEMPLATE, sizeof(LogonRecordT));
        
        while(bufOffset < bufSize)
        {
            THROW_ERROR(GetTextField(buf, bufSize, &field, &bufOffset));
   
            switch (field.tag)
            {
                DECODE_STEP_MSG_HEADER_STUB

                case STEP_ENCRYPT_METHOD_TAG:
                {
                    STEP_EXTRACT_CHAR_VALUE(field, int8, pRecord->encryptMethod);
                    break;
                }
                case STEP_HEARTBT_INT_TAG:
                {
                    STEP_EXTRACT_INT_VALUE(field, uint16, pRecord->heartBtInt);
                    break;
                }
                case STEP_USERNAME_TAG:
                {
                    STEP_EXTRACT_STRING_VALUE(field, pRecord->username, 
                            (int32)sizeof(pRecord->username));
                    break;
                }
                case STEP_PASSWORD_TAG:
                {
                    STEP_EXTRACT_STRING_VALUE(field, pRecord->password, 
                            (int32)sizeof(pRecord->password));
                    break;
                }
                default:
                    THROW_ERROR(ERCD_STEP_UNEXPECTED_TAG, field.tag);
                    break;
            }
        }
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/*
 * 解码STEP登出消息
 *
 * @param   buf             in  - 解码缓冲区
 * @param   bufSize         in  - 解码缓冲区长度
 * @param   pMsg            out - STEP消息
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
static ResCodeT DecodeLogoutRecord(const char* buf, int32 bufSize, StepMessageT* pMsg)
{
    TRY
    {
        /* 登出请求记录初始化模板 */
        static const LogoutRecordT STEP_LOGOUT_RECORD_TEMPLATE = 
        {
            STEP_INVALID_STRING_VALUE, /* text */
        };

        pMsg->msgType = STEP_MSGTYPE_LOGOUT;

        StepFieldT field;
        int32 bufOffset = 0;
        LogoutRecordT* pRecord = (LogoutRecordT*)pMsg->body;
        memcpy(pRecord, &STEP_LOGOUT_RECORD_TEMPLATE, sizeof(LogoutRecordT));
        
        while(bufOffset < bufSize)
        {
            THROW_ERROR(GetTextField(buf, bufSize, &field, &bufOffset));

            switch(field.tag)
            {
                DECODE_STEP_MSG_HEADER_STUB

                case STEP_TEXT_TAG:
                {
                    STEP_EXTRACT_STRING_VALUE(field, pRecord->text, 
                            (int32)sizeof(pRecord->text));
                    break;
                }
                default:
                    THROW_ERROR(ERCD_STEP_UNEXPECTED_TAG, field.tag);
                    break;
            }
        }
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/*
 * 解码STEP心跳消息
 *
 * @param   buf             in  - 解码缓冲区
 * @param   bufSize         in  - 解码缓冲区长度
 * @param   pMsg            out - STEP消息
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
static ResCodeT DecodeHeartBeatRecord(const char* buf, int32 bufSize, StepMessageT* pMsg)
{
    TRY
    {
        pMsg->msgType = STEP_MSGTYPE_HEARTBEAT;

        StepFieldT field;
        int32 bufOffset = 0;
        
        while(bufOffset < bufSize)
        {
            THROW_ERROR(GetTextField(buf, bufSize, &field, &bufOffset));

            switch(field.tag)
            {
                DECODE_STEP_MSG_HEADER_STUB

                default:
                    THROW_ERROR(ERCD_STEP_UNEXPECTED_TAG, field.tag);
                    break;
            }
        }
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/*
 * 解码STEP行情订阅消息
 *
 * @param   buf             in  - 解码缓冲区
 * @param   bufSize         in  - 解码缓冲区长度
 * @param   pMsg            out - STEP消息
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
static ResCodeT DecodeMDRequestRecord(const char* buf, int32 bufSize, StepMessageT* pMsg)
{
    TRY
    {
        /* 行情请求记录初始化模板 */
        static const MDRequestRecordT STEP_MD_REQUEST_RECORD_TEMPLATE = 
        {
            STEP_INVALID_STRING_VALUE, /* securityType */
        };

        pMsg->msgType = STEP_MSGTYPE_MD_REQUEST;

        StepFieldT field;
        int32 bufOffset = 0;
        MDRequestRecordT* pRecord = (MDRequestRecordT*)pMsg->body;
        memcpy(pRecord, &STEP_MD_REQUEST_RECORD_TEMPLATE, sizeof(MDRequestRecordT));
        
        while(bufOffset < bufSize)
        {
            THROW_ERROR(GetTextField(buf, bufSize, &field, &bufOffset));

            switch(field.tag)
            {
                DECODE_STEP_MSG_HEADER_STUB

                case STEP_SECURITY_TYPE_TAG:
                {
                    STEP_EXTRACT_STRING_VALUE(field, pRecord->securityType, 
                            (int32)sizeof(pRecord->securityType));
                    break;
                }
                default:
                    THROW_ERROR(ERCD_STEP_UNEXPECTED_TAG, field.tag);
                    break;
            }
        }
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/*
 * 解码STEP行情快照消息
 *
 * @param   buf             in  - 解码缓冲区
 * @param   bufSize         in  - 解码缓冲区长度
 * @param   pMsg            out - STEP消息
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
static ResCodeT DecodeMDSnapshotFullRefreshRecord(const char* buf, 
        int32 bufSize, StepMessageT* pMsg)
{
    TRY
    {
        /* 全幅行情消息初始化模板 */
        static const MDSnapshotFullRefreshRecordT STEP_MDSNAPSHORT_RECORD_TEMPLATE = 
        {
            STEP_INVALID_STRING_VALUE,  /* securityType */
            STEP_INVALID_INT_VALUE,     /* tradSesMode */
            STEP_INVALID_UINT_VALUE,    /* applID */
            STEP_INVALID_UINT_VALUE,    /* applSeqNum */
            STEP_INVALID_STRING_VALUE,  /* tradeDate */
            STEP_INVALID_STRING_VALUE,  /* lastUpdateTime */
            STEP_INVALID_STRING_VALUE,  /* mdUpdateType */
            STEP_INVALID_UINT_VALUE,    /* mdCount */
            STEP_INVALID_UINT_VALUE,    /* mdDataLen */
            STEP_INVALID_STRING_VALUE   /* mdData */
        };

        pMsg->msgType = STEP_MSGTYPE_MD_SNAPSHOT;

        StepFieldT field;
        int32 bufOffset = 0;
        MDSnapshotFullRefreshRecordT* pRecord = (MDSnapshotFullRefreshRecordT*)pMsg->body;
        memcpy(pRecord, &STEP_MDSNAPSHORT_RECORD_TEMPLATE, sizeof(MDSnapshotFullRefreshRecordT));
        
        while(bufOffset < bufSize)
        {
            THROW_ERROR(GetTextField(buf, bufSize, &field, &bufOffset));
      
            switch(field.tag)
            {
                DECODE_STEP_MSG_HEADER_STUB
                
                case STEP_SECURITY_TYPE_TAG:
                {
                    STEP_EXTRACT_STRING_VALUE(field, pRecord->securityType, 
                            (int32)sizeof(pRecord->securityType));
                    break;
                }
                case STEP_TRADE_SES_MODE_TAG:
                {
                    STEP_EXTRACT_INT_VALUE(field, int16, pRecord->tradSesMode);
                    break;
                }
                case STEP_APPL_ID_TAG:
                {
                    STEP_EXTRACT_INT_VALUE(field, uint32, pRecord->applID);
                    break;
                }
                case STEP_APPL_SEQ_NUM_TAG:
                {
                    STEP_EXTRACT_INT_VALUE(field, uint64, pRecord->applSeqNum);
                    break;
                }
                case STEP_TRADE_DATE_TAG:
                {
                    STEP_EXTRACT_STRING_VALUE(field, pRecord->tradeDate, 
                            (int32)sizeof(pRecord->tradeDate));
                    break;
                }
                case STEP_LAST_UPDATETIME_TAG:
                {
                    STEP_EXTRACT_STRING_VALUE(field, pRecord->lastUpdateTime, 
                            (int32)sizeof(pRecord->lastUpdateTime));
                    break;
                }
                case STEP_MD_UPDATETYPE_TAG:
                {
                    STEP_EXTRACT_STRING_VALUE(field, pRecord->mdUpdateType, 
                            (int32)sizeof(pRecord->mdUpdateType));
                    break;
                }
                case STEP_MD_COUNT_TAG:
                {
                    STEP_EXTRACT_INT_VALUE(field, uint32, pRecord->mdCount);
                    break;
                }
                case STEP_RAWDATA_LENGTH_TAG:
                {
                    STEP_EXTRACT_INT_VALUE(field, uint32, pRecord->mdDataLen);

                    /* Tag(96, RawData)必须紧跟Tag(95, RawDataLength) */
                    THROW_ERROR(GetBinaryField(buf, bufSize, pRecord->mdDataLen, 
                            &field, &bufOffset));

                    if(STEP_RAWDATA_TAG != field.tag)
                    {
                        THROW_ERROR(ERCD_STEP_INVALID_MSGFORMAT, 
                            "Tag 96(RawData) must follow Tag 95(RawDataLength)");
                    }
                    STEP_EXTRACT_BINARY_VALUE(field, pRecord->mdData, 
                            (int32)sizeof(pRecord->mdData));
                    break;
                }
                default:
                    THROW_ERROR(ERCD_STEP_UNEXPECTED_TAG, field.tag);
                    break;
            }
        }
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}
