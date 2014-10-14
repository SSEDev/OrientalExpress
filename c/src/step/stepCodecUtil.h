/*
 * Express Project Studio, Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    stepCodecUtil.h
 *
 * STEP协议编解码工具库头文件
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

#ifndef EPS_STEP_CODEC_UTIL_H
#define EPS_STEP_CODEC_UTIL_H


#ifdef __cplusplus
extern "C" {
#endif


/*
 * 类型申明
 */

/*
 * STEP字段结构
 */
typedef struct StepFieldTag
{
    int32   tag;      
    char*   value;
    int32   valueSize;
} StepFieldT;


/*
 * 函数申明
 */

/*
 * 编码int8数据
 */
ResCodeT AddInt8Field(int32 tag, int8 value, char* buf, int32 bufSize, 
        int32* pOffset);

/*
 * 编码int16数据
 */
ResCodeT AddInt16Field(int32 tag, int16 value, char* buf, int32 bufSize, 
        int32* pOffset);

/*
 * 编码int32数据
 */
ResCodeT AddInt32Field(int32 tag, int32 value, char* buf, int32 bufSize, 
        int32* pOffset);

/*
 * 编码int64数据
 */
ResCodeT AddInt64Field(int32 tag, int64 value, char* buf, int32 bufSize, 
        int32* pOffset);

/*
 * 编码uint8数据
 */
ResCodeT AddUint8Field(int32 tag, uint8 value, char* buf, int32 bufSize, 
        int32* pOffset);

/*
 * 编码uint16数据
 */
ResCodeT AddUint16Field(int32 tag, uint16 value, char* buf, int32 bufSize, 
        int32* pOffset);

/*
 * 编码uint32数据
 */
ResCodeT AddUint32Field(int32 tag, uint32 value, char* buf, int32 bufSize, 
        int32* pOffset);

/*
 * 编码uint64数据
 */
ResCodeT AddUint64Field(int32 tag, uint64 value, char* buf, int32 bufSize, 
        int32* pOffset);

/*
 * 编码string数据
 */
ResCodeT AddStringField(int32 tag, const char* value, char* buf, int32 bufSize, 
        int32* pOffset);

/*
 * 编码binary数据
 */
ResCodeT AddBinaryField(int32 tag, const char* value, int32 valueLen, 
        char* buf, int32 bufSize, int32* pOffset);

/*
 * 获取字符型数据域
 */
ResCodeT GetTextField(const char* buf, int32 bufSize, StepFieldT* pField, int32* pOffset);

/*
 * 获取二进制数据域
 */
ResCodeT GetBinaryField(const char* buf, int32 bufSize, int32 valueSize, StepFieldT* pField, 
        int32* pOffset);

/*
 * 计算校验和
 */
ResCodeT CalcChecksum(const char* buf, int32 bufSize, char* checksum);

/*
 * 校验纯数字域的字符串
 *
 * @param   _field          in  - Step字段
 */
#define STEP_CHECK_NUMBERONLY_TEXT(_field) \
    do\
    {\
        int32 i = 0;\
        for(; i < _field.valueSize; i++)\
        {\
            if (_field.value[i] > '9' || _field.value[i] < '0') \
            {\
                THROW_ERROR(ERCD_STEP_INVALID_FLDVALUE,\
                        _field.tag, _field.valueSize, _field.value,\
                        "Invalid number only string value");\
            }\
        }\
  } while(0)

/*
 * 校验ASCII域(字母和数字)的字符串
 *
 * @param   _field          in  - Step字段
 */
#define STEP_CHECK_ASCIIONLY_TEXT(_field) \
    do\
    {\
        int32 i = 0;\
        for(; i < _field.valueSize; i++)\
        {\
            if ( (_field.value[i] >'9' || _field.value[i] < '0') && \
                    (_field.value[i] > 'z' || _field.value[i] < 'a') && \
                    (_field.value[i] > 'Z' || _field.value[i] < 'A') && \
                    (_field.value[i] != '.') && \
                    (_field.value[i] != ' ')) \
            {\
                THROW_ERROR(ERCD_STEP_INVALID_FLDVALUE, \
                    _field.tag, _field.valueSize, _field.value,\
                    "Invalid ASCII only string value");\
            }\
        }\
    } while(0)

/*
 * 从StepFieldT中提取CHAR类型字段数据
 *
 * @param   _field          in  - StepFieldT类型域
 * @param   _type           in  - 提取类型(int8, uint8)
 * @param   _value          out - 数据
 *
 */
#define STEP_EXTRACT_CHAR_VALUE(_field, _type, _value)\
    do\
    {\
        STEP_CHECK_ASCIIONLY_TEXT(_field);\
        if(1 != sizeof(_type) || 1 != _field.valueSize)\
        {\
            THROW_ERROR(ERCD_STEP_INVALID_FLDVALUE,\
                    _field.tag, _field.valueSize, _field.value,\
                    "Invalid char value");\
        }\
        _value = (_type)_field.value[0];\
    } while(0)

/*
 * 从StepFieldT中提取INT类型字段数据
 *
 * @param   _field          in  - StepFieldT类型域
 * @param   _type           in  - 提取类型
 *                                 (int16, int32, int64,
 *                                 uint16, uint32, uint64)
 * @param   _value          out - 数据
 *
 */
#define STEP_EXTRACT_INT_VALUE(_field, _type, _value)\
    do\
    {\
        int32 i = 0;\
        if (_field.valueSize == 1) \
        {\
            if (_field.value[i] > '9' || _field.value[i] < '0')\
            {\
                THROW_ERROR(ERCD_STEP_INVALID_FLDVALUE, \
                    _field.tag, _field.valueSize, _field.value,\
                    "Invalid integer value");\
            }\
        }\
        else \
        { \
            if (_field.value[i] > '9' || _field.value[i] < '1') \
            {\
                THROW_ERROR(ERCD_STEP_INVALID_FLDVALUE, \
                        _field.tag, _field.valueSize, _field.value,\
                        "Invalid integer value");\
            }\
        }\
        i++; \
        for (; i < _field.valueSize; i++)\
        {\
            if (_field.value[i] > '9' || _field.value[i] < '0')\
            {\
                THROW_ERROR(ERCD_STEP_INVALID_FLDVALUE, \
                    _field.tag, _field.valueSize, _field.value,\
                    "Invalid integer value");\
            }\
        }\
        \
        if(1 == sizeof(_type))\
        {\
            _value = (_type)_field.value[0];\
        }\
        else\
        {\
            _value = (_type)atoll(_field.value);\
        }\
    } while(0)

/*
 * 从StepFieldT中提取字符串类型的字段数据
 *
 * @param   _field              in  - StepFieldT类型域
 * @param   _valueBuf           out - 字符数组
 * @param   _valueBufSize       in  - 用于存放字符数组的长度
 *
 */
#define STEP_EXTRACT_STRING_VALUE(_field, _valueBuf, _valueBufSize)\
    do\
    {\
        if((_field.valueSize+1) > _valueBufSize)\
        {\
            THROW_ERROR(ERCD_STEP_INVALID_FLDVALUE,\
                _field.tag, _field.valueSize, _field.value,\
                "Invalid value size");\
        }\
        \
        memcpy(_valueBuf, _field.value, _field.valueSize);\
        _valueBuf[_field.valueSize] = 0x00; \
    } while(0)

/*
 * 从StepFieldT中校验并提取二进制类型的字段数据
 *
 * @param   _field              in  - StepFieldT类型域
 * @param   _valueBuf           out - 字符数组
 * @param   _valueBufSize       in  - 用于存放字符数组的长度
 *
 */
#define STEP_EXTRACT_BINARY_VALUE(_field, _valueBuf, _valueBufSize)\
    do\
    {\
        if(_field.valueSize > _valueBufSize)\
        {\
            THROW_ERROR(ERCD_STEP_INVALID_FLDVALUE,\
                _field.tag, _field.valueSize, _field.value,\
                "Invalid value length");\
        }\
        memcpy(_valueBuf, _field.value, _field.valueSize);\
    } while(0)

#ifdef __cplusplus
}
#endif

#endif /* EPS_STEP_CODEC_UTIL_H */
