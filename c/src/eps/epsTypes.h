/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    epsTypes.h
 *
 * 东方快车接口API数据类型定义头文件
 *
 * @version $Id
 * @since   2014/02/17
 * @author  Jin Chengxun
 */

/**
MODIFICATION HISTORY:
<pre>
================================================================================
DD-MMM-YYYY INIT.    SIR    Modification Description
----------- -------- ------ ----------------------------------------------------
17-FEB-2014 CXJIN           创建
================================================================================
</pre>
*/

#ifndef EPS_TYPES_H
#define EPS_TYPES_H


#ifdef __cplusplus
extern "C" {
#endif

/**
 * 数据类型定义
 */

/*
 * 基础数据类型定义
 */
typedef signed char                 int8;
typedef unsigned char               uint8;

typedef short                       int16;
typedef unsigned short              uint16;

typedef int                         int32;
typedef unsigned int                uint32;

typedef long long                   int64;
typedef unsigned long long          uint64;

typedef float                       float32;
typedef double                      float64;
typedef long double                 float128;

#ifdef __cplusplus
}
#endif

#endif /* EPS_TYPES_H */
