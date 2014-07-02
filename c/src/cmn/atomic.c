/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    atomic.c
 *
 * 原子操作实现文件
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

/**
 * 包含头文件
 */

#include "atomic.h"


/**
 * 接口函数实现
 */

/**
 * 原子操作比较并替换
 *
 * @param   atomic          in  - 待比较值
 *                          out - 比较后新值
 * @param   oldVal          in  - 比较旧值
 * @param   newVal          in  - 比较新值
 *
 * @return  atomic与旧值相等返回TRUE, 否则返回FALSE
 */
BOOL EpsAtomicIntCompareAndExchange (volatile int *atomic, int oldVal, int newVal)
{  
    int result;   

    __asm__ __volatile__ ("lock; cmpxchgl %2, %1"
        : "=a" (result), "=m" (*atomic)         
        : "r" (newVal), "m" (*atomic), "0" (oldVal));   

    return result == oldVal;
}
