/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    errtable.h
 *
 * 错误码表定义头文件
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

#ifndef EPS_ERRTABLE_H
#define EPS_ERRTABLE_H


#ifdef __cplusplus
extern "C" {
#endif

/**
 * 接口函数定义
 */

/*
 * 加载错误码表
 */
ResCodeT EpsLoadErrorTable();

#ifdef __cplusplus
}
#endif

#endif /* EPS_ERRTABLE_H */
