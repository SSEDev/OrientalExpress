/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    recMutex.h
 *
 * 递归互斥锁定义头文件
 *
 * @version $Id
 * @since   2014/04/11
 * @author  Wu Zheng
 */

/**
MODIFICATION HISTORY:
<pre>
================================================================================
DD-MMM-YYYY INIT.    SIR    Modification Description
----------- -------- ------ ----------------------------------------------------
11-APR-2014 ZHENGWU         创建
================================================================================
</pre>
*/

#ifndef EPS_RECMUTEX_H
#define EPS_RECMUTEX_H


#ifdef __cplusplus
extern "C" {
#endif

/**
 * 包含头文件
 */

#include "common.h"


/**
 * 类型定义
 */

/*
 * 递归互斥锁结构
 */
typedef struct EpsRecMutexTag
{
#if defined(__WINDOWS__)  
	HANDLE				mutex;
#endif  
  
#if defined(__LINUX__) || defined(__HPUX__) 
    pthread_mutex_t     mutex;
#endif  
} EpsRecMutexT;


/**
 * 接口函数定义
 */

/*
 * 初始化递归互斥锁
 */
void InitRecMutex(EpsRecMutexT* pMutex);

/*
 * 反初始化递归互斥锁
 */
void UninitRecMutex(EpsRecMutexT* pMutex);

/*
 * 对递归互斥锁加锁
 */
void LockRecMutex(EpsRecMutexT* pMutex);

/**
 * 对递归互斥锁解锁
 */
void UnlockRecMutex(EpsRecMutexT* pMutex);


#ifdef __cplusplus
}
#endif

#endif /* EPS_RECMUTEX_H */
