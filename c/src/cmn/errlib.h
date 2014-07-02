/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/*
 * @file    errlib.h
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

#ifndef EPS_ERRLIB_H
#define EPS_ERRLIB_H

/**
 * 包含头文件
 */

#include "errcode.h"
#include "epsTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 错误库接口定义
 */

/* 
 * 错误码数据类型定义 
 */
#ifndef ResCodeT
#define ResCodeT    int32
#endif


/* 
 * 错误信息结构体 
 */

#define EPS_ERRDESC_MAX_LEN         256         /* 错误描述最大长度 */

typedef struct ErrorInfoTag
{
    ResCodeT    errCode;                        /* 错误码 */
    char        errDscr[EPS_ERRDESC_MAX_LEN+1]; /* 错误描述 */
} ErrorInfoT;

/* 全局错误码申明 */
extern __thread ResCodeT __errCode;

/* 执行成功返回码 */
#define NO_ERR                                  1

/* 正常处理流程块 */
#define TRY                                     \
    ResCodeT __rc = NO_ERR;                     \
    
/* 捕获异常处理流程块 */
#define CATCH                                   \
    goto el_finally;                            \
    goto el_catch;                              \
    el_catch:
    
/* 收尾处理流程块 */
#define FINALLY                                 \
    el_finally:


/* 判断返回码是否成功 */
#define OK(_rc_)                                \
    ((_rc_) == NO_ERR)

/* 判断返回码是否错误 */
#define NOTOK(_rc_)                             \
    ((_rc_) != NO_ERR)

/* 设置返回码 */
#define SET_RESCODE(_rc_)                       \
    __rc = (_rc_);

/* 获取返回码 */
#define GET_RESCODE() (__rc)

/* 返回返回码 */
#define RETURN_RESCODE                          \
    return __rc;


/* 设置返回码，并跳转到CATCH */
#define THROW_RESCODE(_rc_)                     \
do                                              \
{                                               \
    if(OK(__rc = (_rc_))) {goto el_finally;}    \
    else                   {goto el_catch;}     \
} while (0);

/* THROW_ERROR 设置错误码及错误信息，并跳转到CATCH，参数可选 */
#define THROW_ERROR(_errCode, _params...)       \
do                                              \
{                                               \
    if ((_errCode) != NO_ERR)                   \
    {                                           \
        if (__errCode != (_errCode))            \
        {                                       \
            ErrSetError(_errCode, ##_params);   \
        }                                       \
        __rc = (_errCode);                      \
        goto el_catch;                          \
    }                                           \
} while (0)

/**
 * 加载错误信息表
 * @param   pTable               in  - 错误信息表
 * @param   tableSize            in  - 错误信息表长度
 */
void ErrLoadErrorTable(const ErrorInfoT* pTable, uint32 tableSize);
        
/**
 * 设置错误信息
 *
 * @param   errCode                 in  - 错误码
 * @param   ...                     in  - 错误信息参数(可选变参)
 */
void ErrSetError(ResCodeT errCode, ...);

/**
 * 获取错误信息描述
 *
 * @return  最近一次错误信息描述
 */
const char* ErrGetErrorDscr();

/**
 * 获取错误码
 *
 * @return  最近一次错误码
 */
ResCodeT ErrGetErrorCode();

/**
 * 清空错误信息
 */
void ErrClearError();

#ifdef __cplusplus
}
#endif

#endif  /* EPS_ERRLIB_H */
