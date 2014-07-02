/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/*
 * @file    common.c
 *
 * 通用定义头文件
 *
 * @version $Id
 * @since   2014/05/12
 * @author  Wu Zheng
 *    
 */

/*
 MODIFICATION HISTORY:
 <pre>
 ================================================================================
 DD-MMM-YYYY INIT.    SIR    Modification Description
 ----------- -------- ------ ----------------------------------------------------
 12-05-2014  ZHENGWU         创建
 ================================================================================
  </pre>
*/

/**
 * 包含头文件
 */

#include "common.h"


/**
 * 全局定义
 */

__thread char __defErrDscr[128];       /* 默认错误描述 */


/**
 *  接口函数实现
 */

/**
 * 获取操作系统错误描述
 *
 * @param   errno         		in  - 操作系统错误码
 *
 * @return  操作系统错误描述
 */
const char* EpsGetSystemError(int errCode)
{
#if defined(__WINDOWS__)
	int nLen;
	void* lpInfo = NULL;
	char* lpszInfo = NULL;

	nLen = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, errCode, MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), (char*)&lpInfo, 0, NULL);
	if (nLen>0 && lpInfo!=NULL)
	{
		lpszInfo = (LPSTR)lpInfo;

		if (nLen >= 2)
		{
			if (lpszInfo[nLen-1] == '\n') nLen--;
			if (lpszInfo[nLen-1] == '\r') nLen--;
		}
		// 去除尾部的<LF><CR>
		lpszInfo[nLen] = 0x00;
		return lpszInfo;
	}
#endif

#if defined(__LINUX__) || defined(__HPUX__) 
	return strerror(errCode);
#endif

	snprintf(__defErrDscr, sizeof(__defErrDscr), "Unknown error code(%d)", errCode);
	return __defErrDscr;
}
