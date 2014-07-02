/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/*
 * @file    errlib.c
 *
 * 错误库定义头文件
 *
 * @version $Id
 * @since   2014/02/17
 * @author  Jin Chengxun
 *
 */

/*
 MODIFICATION HISTORY:
 <pre>
 ================================================================================
 DD-MMM-YYYY INIT.    SIR    Modification Description
 ----------- -------- ------ ----------------------------------------------------
 17-02-2014  CXJIN           创建
 ================================================================================
  </pre>
*/

/**
 * 包含头文件
 */


#include "common.h"
#include "epsTypes.h"

#include "errlib.h"

/**
 * 全局定义
 */

__thread char __errDscr[1024] = {0};        /* 最近的错误信息描述 */
__thread ResCodeT __errCode = NO_ERR;       /* 最近的错误码 */

static const ErrorInfoT* g_pErrorTable = NULL;
static uint32 g_errorTableSize = 0;

/**
 * 函数实现
 */

/**
 * 加载错误信息表
 * @param   pTable               in  - 错误信息表
 * @param   tableSize            in  - 错误信息表长度
 */
void ErrLoadErrorTable(const ErrorInfoT* pTable, uint32 tableSize)
{
    g_pErrorTable = pTable;
    g_errorTableSize = tableSize;
}

/**
 * 查找指定错误码对应的错误信息描述
 *
 * @param   errCode               in  - 错误码
 *
 * @return  找到则返回对应错误信息描述，否则返回NULL
 */
static const ErrorInfoT* ErrLookupError(ResCodeT errCode)
{
    int32 begin, end, middle;
    const ErrorInfoT* pInfo = NULL;

    if(g_pErrorTable == NULL)
    {
        return pInfo;
    }
    
    begin = 0;
    end = g_errorTableSize - 1;
    while (begin <= end)
    {
        middle = (begin + end) / 2;
        if ((g_pErrorTable[middle].errCode) == errCode)
        {
            pInfo = &g_pErrorTable[middle];
            break;
        }

        if((g_pErrorTable[middle].errCode) < errCode)
        {
            begin = middle + 1;
        }
        else
        {
            end = middle - 1;
        }
    }
    
    return pInfo;
}

/**
 * 设置错误信息
 *
 * @param   errCode                 in  - 错误码
 * @param   ...                     in  - 错误信息参数(可选变参) 
 *
 * @return  NO_ERR-成功; 其他-失败
 */
void ErrSetError(ResCodeT errCode, ...)
{
    __errCode = errCode;
    
    const ErrorInfoT* pErrorInfo = ErrLookupError(errCode);
    if (pErrorInfo == NULL)
    {
        snprintf(__errDscr, sizeof(__errDscr), "Unknown error code: %d", errCode);
        return;
    }

    va_list valist;
    va_start(valist, errCode);
    vsnprintf(__errDscr, sizeof(__errDscr), pErrorInfo->errDscr, valist);
    va_end(valist);
}

/**
 * 获取错误信息描述
 *
 * @return  错误信息描述
 */
const char* ErrGetErrorDscr()
{
    return __errDscr;
}

/**
 * 获取错误码
 *
 * @return  最近一次错误码
 */
ResCodeT ErrGetErrorCode()
{
    return __errCode;
}

/**
 * 清空错误信息
 */
void ErrClearError()
{
    __errCode    = NO_ERR;
    __errDscr[0] = '\0';
}
