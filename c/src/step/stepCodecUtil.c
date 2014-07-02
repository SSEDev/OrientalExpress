/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    stepCodecUtil.c
 *
 * STEP协议编解码工具库实现文件
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


/*
 * 全局申明
 */
const char STEP_DELIMITER  = 0x01;      /* 分隔符 <SOH> */


/*
 * 内部函数申明
 */

/*
 * 查找分隔符
 */
static int32 FindDelimiter(const char* buf, int32 bufSize, char delimiter);

/*
 * 函数实现
 */

/*
 * 编码int8数据
 *
 * @param   tag             in  -  标记
 * @param   value           in  -  值
 * @param   buf             out -  编码缓冲区
 * @param   bufSize         in  -  编码缓冲区长度
 * @param   pOffset         in  -  编码前缓冲区偏移
 *                          out -  编码后缓冲区偏移
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT AddInt8Field(int32 tag, int8 value, char* buf, int32 bufSize, 
        int32* pOffset)
{
    TRY
    {
        char* bufBegin    = buf + *pOffset;
        int32 bufLeftSize = bufSize - *pOffset;
        int len = snprintf(bufBegin, bufLeftSize, "%d=%c%c", tag, value, STEP_DELIMITER);
        if (bufLeftSize < len + 1)
        {
            THROW_ERROR(ERCD_STEP_BUFFER_OVERFLOW);
        }

        *pOffset += len; 
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
 * 编码int16数据
 *
 * @param   tag             in  -  标记
 * @param   value           in  -  值
 * @param   buf             out -  编码缓冲区
 * @param   bufSize         in  -  编码缓冲区长度
 * @param   pOffset         in  -  编码前缓冲区偏移
 *                          out -  编码后缓冲区偏移
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT AddInt16Field(int32 tag, int16 value, char* buf, int32 bufSize, 
        int32* pOffset)
{
    TRY
    {
        char* bufBegin    = buf + *pOffset;
        int32 bufLeftSize = bufSize - *pOffset;
        int len = snprintf(bufBegin, bufLeftSize, "%d=%d%c", tag, value, STEP_DELIMITER);
        if (bufLeftSize < len + 1)
        {
            THROW_ERROR(ERCD_STEP_BUFFER_OVERFLOW);
        }

        *pOffset += len; 
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
 * 编码int32数据
 *
 * @param   tag             in  -  标记
 * @param   value           in  -  值
 * @param   buf             out -  编码缓冲区
 * @param   bufSize         in  -  编码缓冲区长度
 * @param   pOffset         in  -  编码前缓冲区偏移
 *                          out -  编码后缓冲区偏移
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT AddInt32Field(int32 tag, int32 value, char* buf, int32 bufSize, 
        int32* pOffset)
{
    TRY
    {
        char* bufBegin    = buf + *pOffset;
        int32 bufLeftSize = bufSize - *pOffset;
        int len = snprintf(bufBegin, bufLeftSize, "%d=%d%c", tag, value, STEP_DELIMITER);
        if (bufLeftSize < len + 1)
        {
            THROW_ERROR(ERCD_STEP_BUFFER_OVERFLOW);
        }

        *pOffset += len; 
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
 * 编码int64数据
 *
 * @param   tag             in  -  标记
 * @param   value           in  -  值
 * @param   buf             out -  编码缓冲区
 * @param   bufSize         in  -  编码缓冲区长度
 * @param   pOffset         in  -  编码前缓冲区偏移
 *                          out -  编码后缓冲区偏移
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT AddInt64Field(int32 tag, int64 value, char* buf, int32 bufSize, 
        int32* pOffset)
{
    TRY
    {
        char* bufBegin    = buf + *pOffset;
        int32 bufLeftSize = bufSize - *pOffset;
#if defined (__LINUX__) || defined (__HPUX__)
        int len = snprintf(bufBegin, bufLeftSize, "%d=%lld%c", tag, value, STEP_DELIMITER);
#endif

#if defined (__WINDOWS__)
        int len = snprintf(bufBegin, bufLeftSize, "%d=%I64d%c", tag, value, STEP_DELIMITER);
#endif
        if (bufLeftSize < len + 1)
        {
            THROW_ERROR(ERCD_STEP_BUFFER_OVERFLOW);
        }

        *pOffset += len; 

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
 * 编码uint8数据
 *
 * @param   tag             in  -  标记
 * @param   value           in  -  值
 * @param   buf             out -  编码缓冲区
 * @param   bufSize         in  -  编码缓冲区长度
 * @param   pOffset         in  -  编码前缓冲区偏移
 *                          out -  编码后缓冲区偏移
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT AddUint8Field(int32 tag, uint8 value, char* buf, int32 bufSize, 
        int32* pOffset)
{
    TRY
    {
        char* bufBegin    = buf + *pOffset;
        int32 bufLeftSize = bufSize - *pOffset;
        int len = snprintf(bufBegin, bufLeftSize, "%d=%u%c", tag, value, STEP_DELIMITER);
        if (bufLeftSize < len + 1)
        {
            THROW_ERROR(ERCD_STEP_BUFFER_OVERFLOW);
        }

        *pOffset += len; 
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
 * 编码uint16数据
 *
 * @param   tag             in  -  标记
 * @param   value           in  -  值
 * @param   buf             out -  编码缓冲区
 * @param   bufSize         in  -  编码缓冲区长度
 * @param   pOffset         in  -  编码前缓冲区偏移
 *                          out -  编码后缓冲区偏移
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT AddUint16Field(int32 tag, uint16 value, char* buf, int32 bufSize, 
        int32* pOffset)
{
    TRY
    {
        char* bufBegin    = buf + *pOffset;
        int32 bufLeftSize = bufSize - *pOffset;
        int len = snprintf(bufBegin, bufLeftSize, "%d=%u%c", tag, value, STEP_DELIMITER);
        if (bufLeftSize < len + 1)
        {
            THROW_ERROR(ERCD_STEP_BUFFER_OVERFLOW);
        }

        *pOffset += len;
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
 * 编码uint32数据
 *
 * @param   tag             in  -  标记
 * @param   value           in  -  值
 * @param   buf             out -  编码缓冲区
 * @param   bufSize         in  -  编码缓冲区长度
 * @param   pOffset         out -  编码长度 
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT AddUint32Field(int32 tag, uint32 value, char* buf, int32 bufSize, 
        int32* pOffset)
{
    TRY
    {
        char* bufBegin    = buf + *pOffset;
        int32 bufLeftSize = bufSize - *pOffset;
        int len = snprintf(bufBegin, bufLeftSize, "%d=%u%c", tag, value, STEP_DELIMITER);
        if (bufLeftSize < len + 1)
        {
            THROW_ERROR(ERCD_STEP_BUFFER_OVERFLOW);
        }

        *pOffset += len; 
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
 * 编码uint64数据
 *
 * @param   tag             in  -  标记
 * @param   value           in  -  值
 * @param   buf             out -  编码缓冲区
 * @param   bufSize         in  -  编码缓冲区长度
 * @param   pOffset         in  -  编码前缓冲区偏移
 *                          out -  编码后缓冲区偏移
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT AddUint64Field(int32 tag, uint64 value, char* buf, int32 bufSize, 
        int32* pOffset)
{
    TRY
    {
        char* bufBegin    = buf + *pOffset;
        int32 bufLeftSize = bufSize - *pOffset;
#if defined (__LINUX__) || defined (__HPUX__)
        int len = snprintf(bufBegin, bufLeftSize, "%d=%llu%c", tag, value, STEP_DELIMITER);
#endif

#if defined (__WINDOWS__)
        int len = snprintf(bufBegin, bufLeftSize, "%d=%I64u%c", tag, value, STEP_DELIMITER);
#endif
        if (bufLeftSize < len + 1)
        {
            THROW_ERROR(ERCD_STEP_BUFFER_OVERFLOW);
        }

        *pOffset += len;
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
 * 编码string数据
 *
 * @param   tag             in  -  标记
 * @param   value           in  -  值
 * @param   buf             out -  编码缓冲区
 * @param   bufSize         in  -  编码缓冲区长度
 * @param   pOffset         in  -  编码前缓冲区偏移
 *                          out -  编码后缓冲区偏移
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT AddStringField(int32 tag, const char* value, char* buf, int32 bufSize, 
        int32* pOffset)
{
    TRY
    {
        char* bufBegin    = buf + *pOffset;
        int32 bufLeftSize = bufSize - *pOffset;
        int len = snprintf(bufBegin, bufLeftSize, "%d=%s%c", tag, value, STEP_DELIMITER);
        if (bufLeftSize < len + 1)
        {
            THROW_ERROR(ERCD_STEP_BUFFER_OVERFLOW);
        }

        *pOffset += len;
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
 * 编码binary数据
 *
 * @param   tag             in  -  标记
 * @param   value           in  -  值
 * @param   valueLen        in  -  值长度
 * @param   buf             out -  编码缓冲区
 * @param   bufSize         in  -  编码缓冲区长度
 * @param   pOffset         in  -  编码前缓冲区偏移
 *                          out -  编码后缓冲区偏移
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT AddBinaryField(int32 tag, const char* value, int32 valueLen, char* buf, 
        int32 bufSize, int32* pOffset)
{
    TRY
    {
        char* bufBegin    = buf + *pOffset;
        int32 bufLeftSize = bufSize - *pOffset;
        int len = snprintf(bufBegin, bufLeftSize, "%d=", tag);
        int32 fieldSize = len + valueLen + 1;
        if (bufLeftSize < fieldSize)
        {
            THROW_ERROR(ERCD_STEP_BUFFER_OVERFLOW);
        }

        memcpy(bufBegin+len, value, valueLen);
        bufBegin[fieldSize - 1] = STEP_DELIMITER;
        *pOffset += fieldSize;
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
 * 获取字符型数据域
 *
 * @param   buf         in  - 编码缓冲区
 * @param   bufSize     in  - 编码缓冲区长度
 * @param   pField      out - 解析字段
 * @param   pOffset     in  - 解码前缓冲区偏移
 *                      out - 解码后缓冲区偏移
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT GetTextField(const char* buf, int32 bufSize, StepFieldT* pField, int32* pOffset)
{
    TRY
    {
        memset(pField, 0x00, sizeof(StepFieldT));
        
        const char* bufBegin    = buf + *pOffset;
        const int32 bufLeftSize = bufSize - *pOffset;

        int32 offset1, offset2;

        offset1 = FindDelimiter(bufBegin, bufLeftSize, '='); 
        if (offset1 < 0)
        {
            THROW_ERROR(ERCD_STEP_INVALID_FLDFORMAT, "Sign (=) not found");
        }
        else if (offset1 == 0)
        {
            THROW_ERROR(ERCD_STEP_INVALID_FLDFORMAT, "Tag not found");
        }

        /* 校验TAG有效性: 字符必须是有效10进制数，且首字符不能为'0' */
        int32 i = 0;
        const char* p = bufBegin;
        if (*p > '9' || *p < '1')
        {
            THROW_ERROR(ERCD_STEP_INVALID_TAG, offset1, bufBegin);
        }
        i++; p++;
        for (; i < offset1; i++, p++)
        {
            if (*p > '9' || *p < '0')
            {
                THROW_ERROR(ERCD_STEP_INVALID_TAG, offset1, bufBegin);
            }
        }
    
        offset2 = FindDelimiter(bufBegin+offset1+1, bufLeftSize-offset1-1, STEP_DELIMITER); 
        if (offset2 < 0)
        {
            THROW_ERROR(ERCD_STEP_INVALID_FLDFORMAT, "Sign (<SOH>) not found");
        }
        else if (offset2 == 0)
        {
            THROW_ERROR(ERCD_STEP_INVALID_FLDFORMAT, "Value not found");
        }
         
        pField->tag = atoi(bufBegin);
        pField->value = (char*)(bufBegin + offset1 + 1);
        pField->valueSize = offset2;
        *pOffset += offset1 + offset2 + 2;
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
 * 获取二进制数据域
 *
 * @param   buf         in  - 编码缓冲区
 * @param   bufSize     in  - 编码缓冲区长度
 * @param   valueSize   in  - 值域长度
 * @param   pField      out - 解析字段
 * @param   pOffset     in  - 解码前缓冲区偏移
 *                      out - 解码后缓冲区偏移
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT GetBinaryField(const char* buf, int32 bufSize, int32 valueSize, StepFieldT* pField, 
        int32* pOffset)
{
    TRY
    {
        memset(pField, 0x00, sizeof(StepFieldT));
        
        const char* bufBegin    = buf + *pOffset;
        const int32 bufLeftSize = bufSize - *pOffset;

        int32 offset1, offset2;

        offset1 = FindDelimiter(bufBegin, bufLeftSize, '='); 
        if (offset1 < 0)
        {
            THROW_ERROR(ERCD_STEP_INVALID_FLDFORMAT, "Sign (=) not found");
        }
        else if (offset1 == 0)
        {
            THROW_ERROR(ERCD_STEP_INVALID_FLDFORMAT, "Tag not found");
        }

        /* 校验TAG有效性: 字符必须是有效10进制数，且首字符不能为'0' */
        int32 i = 0;
        const char* p = bufBegin;
        if (*p > '9' || *p < '1')
        {
            THROW_ERROR(ERCD_STEP_INVALID_TAG, offset1, bufBegin);
        }
        i++; p++;
        for (; i < offset1; i++, p++)
        {
            if (*p > '9' || *p < '0')
            {
                THROW_ERROR(ERCD_STEP_INVALID_TAG, offset1, bufBegin);
            }
        }

        offset2 = offset1 + valueSize + 1;
        if (offset2+1 > bufLeftSize)    
        {
            THROW_ERROR(ERCD_STEP_INVALID_FLDFORMAT, "value size overflow");
        }
        else if (bufBegin[offset2] != STEP_DELIMITER)
        {
            THROW_ERROR(ERCD_STEP_INVALID_FLDFORMAT, "Sign (<SOH>) not found");
        }
         
        pField->tag = atoi(bufBegin);
        pField->value = (char*)(bufBegin + offset1 + 1);
        pField->valueSize = valueSize;

        *pOffset += offset2 + 1; 
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
 * 计算校验和
 *
 * @param   buf         in  -  缓冲区
 * @param   bufSize     in  -  缓冲区长度
 * @param   checksum    out -  校验和
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT CalcChecksum(const char* buf, int32 bufSize, char* checksum)
{
    TRY
    {
        uint8 iChecksum = 0;
        int32 idx;
        
        for (idx = 0; idx < bufSize; )
        {
            iChecksum += buf[idx++];
        }
        sprintf(checksum, "%03d", iChecksum);
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
 * 搜索分隔符
 *
 * @param   buf         in  -  搜索缓冲区
 * @param   bufSize     in  -  搜索缓冲区长度
 * @param   delimiter   in  -  分隔符
 *
 * @return  成功返回分隔符在缓冲中的位移，否则返回-1
 */
static int32 FindDelimiter(const char* buf, int32 bufSize, char delimiter)
{
    int32 idx = 0, offset = -1;
    for (idx = 0; idx < bufSize; idx++)
    {
        if(delimiter == buf[idx])
        {
            offset = idx;
            break;
        }
    }

    return offset;
}

