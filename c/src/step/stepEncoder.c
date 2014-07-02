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

/*
 * 包含头文件
 */

#include "common.h"
#include "errlib.h"
#include "epsTypes.h"
#include "stepCodecUtil.h"

#include "stepCodec.h"

/*
 * 全局定义
 */

/*
 * STEP消息类型对应关系表
 */
static const char* STEP_MSGTYPE_MAP[] =
{
    STEP_MSGTYPE_LOGON_VALUE,
    STEP_MSGTYPE_LOGOUT_VALUE,
    STEP_MSGTYPE_HEARTBEAT_VALUE,
    STEP_MSGTYPE_MD_REQUEST_VALUE,
    STEP_MSGTYPE_MD_SNAPSHOT_VALUE,
    STEP_MSGTYPE_TRADING_STATUS_VALUE,
};


/*
 * 内部函数申明
 */

static ResCodeT EncodeLogonDataRecord(LogonRecordT* pRecord, StepDirectionT direction, 
        char* buf, int32 bufSize, int32* pEncodeSize);
static ResCodeT EncodeLogoutRecord(LogoutRecordT* pRecord, StepDirectionT direction, 
        char* buf, int32 bufSize, int32* pEncodeSize);
static ResCodeT EncodeMDRequestRecord(MDRequestRecordT* pRecord, StepDirectionT direction, 
        char* buf, int32 bufSize, int32* pEncodeSize);
static ResCodeT EncodeMDSnapshotFullRefreshRecord(MDSnapshotFullRefreshRecordT* pRecord, 
        StepDirectionT direction, char* buf, int32 bufSize, int32* pEncodeSize);
static ResCodeT EncodeStepMessageBody(StepMessageT* pMsg, StepDirectionT direction,
        char* buf, int32 bufSize, int32* pEncodeSize);


/*
 * 函数实现
 */

/**
 * 编码STEP消息
 *
 * @param   pMsg            in  - STEP消息
 * @param   direction       in  - 消息传输方向
 * @param   buf             out - 编码缓冲区
 * @param   bufSize         in  - 编码缓冲区长度
 * @param   pEncodeSize     out - 编码长度
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT EncodeStepMessage(StepMessageT* pMsg, StepDirectionT direction,
        char* buf, int32 bufSize, int32* pEncodeSize)
{
    TRY
    {
        char body[STEP_MSG_MAX_LEN * 2];
        int32 bodySize = 0, encodeSize = 0;

        /* 编码消息体 */
        THROW_ERROR(EncodeStepMessageBody(pMsg, direction, body, sizeof(body), &bodySize));

        /* 编码消息头 */
        THROW_ERROR(AddStringField(STEP_BEGIN_STRING_TAG, STEP_BEGIN_STRING_VALUE, buf, 
                bufSize, &encodeSize));

        THROW_ERROR(AddUint32Field(STEP_BODY_LENGTH_TAG, bodySize, buf, bufSize, 
                &encodeSize));

        /* 预留了BODY和CHECKSUM字段的位置 */
        if (encodeSize + bodySize + STEP_CHECKSUM_FIELD_LEN > bufSize)
        {   
            THROW_ERROR(ERCD_STEP_BUFFER_OVERFLOW);
        }

        memcpy(buf+encodeSize, body, bodySize);
        
        encodeSize += bodySize;

        /* 编码消息尾 */
        char checksum[STEP_CHECKSUM_LEN+1];
        THROW_ERROR(CalcChecksum(buf, encodeSize, checksum));
        THROW_ERROR(AddStringField(STEP_CHECKSUM_TAG, checksum, buf, bufSize, 
                &encodeSize));
       
        *pEncodeSize = encodeSize;
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
 * 编码登陆消息
 *
 * @param   pRecord         in  - 登陆信息
 * @param   direction       in  - 消息方向
 * @param   buf             in  - 编码缓冲区
 *                          out - 编码后的缓冲区
 * @param   bufSize         in  - 编码缓冲区长度
 * @param   pEncodeSize     in  - 编码前缓冲区已编码长度
 *                          out - 编码后缓冲区已编码长度
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT EncodeLogonDataRecord(LogonRecordT* pRecord, StepDirectionT direction, 
        char* buf, int32 bufSize, int32* pEncodeSize)
{
    TRY
    {
        int32 recordSize = 0;

        char* bufBegin    = buf + *pEncodeSize;
        int32 bufLeftSize = bufSize - *pEncodeSize;

        THROW_ERROR(AddInt8Field(STEP_ENCRYPT_METHOD_TAG, pRecord->encryptMethod, 
                bufBegin, bufLeftSize, &recordSize));

        THROW_ERROR(AddUint16Field(STEP_HEARTBT_INT_TAG, pRecord->heartBtInt, 
                bufBegin, bufLeftSize, &recordSize));

        THROW_ERROR(AddStringField(STEP_USERNAME_TAG, pRecord->username, 
                bufBegin, bufLeftSize, &recordSize));

        if(STEP_DIRECTION_REQ == direction)
        {
            THROW_ERROR(AddStringField(STEP_PASSWORD_TAG, pRecord->password, 
                    bufBegin, bufLeftSize, &recordSize));
        }

        *pEncodeSize += recordSize;
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
 * 编码登出消息
 *
 * @param   pRecord         in  - 登出信息
 * @param   direction       in  - 消息方向
 * @param   buf             in  - 编码缓冲区
 *                          out - 编码后的缓冲区
 * @param   bufSize         in  - 编码缓冲区长度
 * @param   pEncodeSize     in  - 编码前缓冲区已编码长度
 *                          out - 编码后缓冲区已编码长度
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT EncodeLogoutRecord(LogoutRecordT* pRecord, StepDirectionT direction, 
        char* buf, int32 bufSize, int32* pEncodeSize)
{
    TRY
    {
        int32 recordSize = 0;

        char* bufBegin    = buf + *pEncodeSize;
        int32 bufLeftSize = bufSize - *pEncodeSize;

        THROW_ERROR(AddStringField(STEP_TEXT_TAG, pRecord->text, 
                bufBegin, bufLeftSize, &recordSize));

        *pEncodeSize += recordSize;
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
 * 编码行情订阅消息
 *
 * @param   pRecord         in  - 行情订阅信息
 * @param   direction       in  - 消息方向
 * @param   buf             in  - 编码缓冲区
 *                          out - 编码后的缓冲区
 * @param   bufSize         in  - 编码缓冲区长度
 * @param   pEncodeSize     in  - 编码前缓冲区已编码长度
 *                          out - 编码后缓冲区已编码长度
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT EncodeMDRequestRecord(MDRequestRecordT* pRecord, StepDirectionT direction, 
        char* buf, int32 bufSize, int32* pEncodeSize)
{
    TRY
    {
        int32 recordSize = 0;

        char* bufBegin    = buf + *pEncodeSize;
        int32 bufLeftSize = bufSize - *pEncodeSize;

        THROW_ERROR(AddStringField(STEP_SECURITY_TYPE_TAG, pRecord->securityType, 
                bufBegin, bufLeftSize, &recordSize));

        *pEncodeSize += recordSize;
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
 * 编码行情快照
 *
 * @param   pRecord         in     -  行情快照信息
 * @param   direction       in  - 消息方向
 * @param   buf             in  - 编码缓冲区
 *                          out - 编码后的缓冲区
 * @param   bufSize         in  - 编码缓冲区长度
 * @param   pEncodeSize     in  - 编码前缓冲区已编码长度
 *                          out - 编码后缓冲区已编码长度
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
static ResCodeT EncodeMDSnapshotFullRefreshRecord(MDSnapshotFullRefreshRecordT* pRecord, 
        StepDirectionT direction, char* buf, int32 bufSize, int32* pEncodeSize)
{
    TRY
    {
        int32 recordSize = 0;

        char* bufBegin    = buf + *pEncodeSize;
        int32 bufLeftSize = bufSize - *pEncodeSize;
        
        THROW_ERROR(AddStringField(STEP_SECURITY_TYPE_TAG, pRecord->securityType, 
                bufBegin, bufLeftSize, &recordSize));

        THROW_ERROR(AddInt16Field(STEP_TRADE_SES_MODE_TAG, pRecord->tradSesMode, 
                bufBegin, bufLeftSize, &recordSize));

        THROW_ERROR(AddUint32Field(STEP_APPL_ID_TAG, pRecord->applID, 
                 bufBegin, bufLeftSize, &recordSize));

        THROW_ERROR(AddUint64Field(STEP_APPL_SEQ_NUM_TAG, pRecord->applSeqNum, 
                 bufBegin, bufLeftSize, &recordSize));

        THROW_ERROR(AddStringField(STEP_TRADE_DATE_TAG, pRecord->tradeDate, 
                 bufBegin, bufLeftSize, &recordSize));

        if (pRecord->lastUpdateTime[0] != 0x00)
        {
            THROW_ERROR(AddStringField(STEP_LAST_UPDATETIME_TAG, pRecord->lastUpdateTime, 
                     bufBegin, bufLeftSize, &recordSize));
        }
        THROW_ERROR(AddStringField(STEP_MD_UPDATETYPE_TAG, pRecord->mdUpdateType, 
                 bufBegin, bufLeftSize, &recordSize));

        THROW_ERROR(AddUint32Field(STEP_MD_COUNT_TAG, pRecord->mdCount, 
                 bufBegin, bufLeftSize, &recordSize));

        THROW_ERROR(AddUint32Field(STEP_RAWDATA_LENGTH_TAG, pRecord->mdDataLen, 
                 bufBegin, bufLeftSize, &recordSize));

        THROW_ERROR(AddBinaryField(STEP_RAWDATA_TAG, pRecord->mdData, pRecord->mdDataLen, 
                 bufBegin, bufLeftSize, &recordSize));
 
        *pEncodeSize += recordSize;
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
 * 编码STEP消息体
 *
 * @param   pMsg            in  -  STEP消息
 * @param   direction       in  - 消息方向
 * @param   buf             in  - 编码缓冲区
 *                          out - 编码后的缓冲区
 * @param   bufSize         in  - 编码缓冲区长度
 * @param   pEncodeSize     in  - 编码前缓冲区已编码长度
 *                          out - 编码后缓冲区已编码长度
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
static ResCodeT EncodeStepMessageBody(StepMessageT* pMsg, StepDirectionT direction,
        char* buf, int32 bufSize, int32* pEncodeSize)
{
    TRY
    {
        if (pMsg->msgType <= STEP_MSGTYPE_INVALID ||
            pMsg->msgType >= STEP_MSGTYPE_COUNT)
        {
            THROW_ERROR(ERCD_STEP_INVALID_MSGTYPE, pMsg->msgType);
        }
     
        int encodeSize = 0;
        
        THROW_ERROR(AddStringField(STEP_MSG_TYPE_TAG, STEP_MSGTYPE_MAP[pMsg->msgType], 
                (char*)buf, bufSize, &encodeSize));

        THROW_ERROR(AddStringField(STEP_SENDER_COMP_ID_TAG, pMsg->senderCompID, 
                (char*)buf, bufSize, &encodeSize));

        THROW_ERROR(AddStringField(STEP_TARGET_COMP_ID_TAG, pMsg->targetCompID, 
                (char*)buf, bufSize, &encodeSize));

        THROW_ERROR(AddUint64Field(STEP_MSG_SEQ_NUM_TAG, pMsg->msgSeqNum, 
                (char*)buf, bufSize, &encodeSize));

        THROW_ERROR(AddStringField(STEP_SENDING_TIME_TAG, pMsg->sendingTime, 
                (char*)buf, bufSize, &encodeSize));

        THROW_ERROR(AddStringField(STEP_MSG_ENCODING_TAG, pMsg->msgEncoding, 
                (char*)buf, bufSize, &encodeSize));

        switch(pMsg->msgType)
        {
            case STEP_MSGTYPE_LOGON:
            {
                THROW_ERROR(EncodeLogonDataRecord((LogonRecordT*)pMsg->body,
                        direction, (char*)buf, bufSize, &encodeSize));
                break;
            }
            case STEP_MSGTYPE_LOGOUT:
            {
                THROW_ERROR(EncodeLogoutRecord((LogoutRecordT*)pMsg->body,
                        direction, (char*)buf, bufSize, &encodeSize));
                break;
            }
            case STEP_MSGTYPE_HEARTBEAT:
            {
                break;
            }
            case STEP_MSGTYPE_MD_REQUEST:
            {
                THROW_ERROR(EncodeMDRequestRecord((MDRequestRecordT*)pMsg->body,
                        direction, (char*)buf, bufSize, &encodeSize));
                break;
            }
            case STEP_MSGTYPE_MD_SNAPSHOT:
            {
                THROW_ERROR(EncodeMDSnapshotFullRefreshRecord((MDSnapshotFullRefreshRecordT*)pMsg->body, 
                        direction, (char*)buf, bufSize, &encodeSize));
                break;
            }
            case STEP_MSGTYPE_TRADING_STATUS:
            {
                break;
            }
            
            default:
                THROW_ERROR(ERCD_STEP_INVALID_MSGTYPE, pMsg->msgType);
                break;
        }

        *pEncodeSize = encodeSize;
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}
