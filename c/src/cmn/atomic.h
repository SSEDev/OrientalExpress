/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    atomic.h
 *
 * 原子操作定义头文件
 *
 * @version $Id
 * @since   2014/04/14
 * @author  Wu Zheng
 */

/**
MODIFICATION HISTORY:
<pre>
================================================================================
DD-MMM-YYYY INIT.    SIR    Modification Description
----------- -------- ------ ----------------------------------------------------
14-APR-2014 ZHENGWU         创建
================================================================================
</pre>
*/

#ifndef EPS_ATOMIC_H
#define EPS_ATOMIC_H


#ifdef __cplusplus
extern "C" {
#endif

/**
 * 包含头文件
 */

#include "common.h"


/**
 * 接口函数定义
 */

/*
 * 原子操作比较并替换
 */
BOOL EpsAtomicIntCompareAndExchange (volatile int *atomic, int oldVal, int newVal);

#ifdef __cplusplus
}
#endif

#endif /* EPS_ATOMIC_H */
