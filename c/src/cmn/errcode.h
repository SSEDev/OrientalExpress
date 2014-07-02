/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/*
 * @file    errcode.h
 *
 * 错误码定义头文件
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
#ifndef EPS_ERRCODE_H
#define EPS_ERRCODE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 错误码定义
 */

/* Express接口库错误码 */
#define ERCD_EPS_OPERSYSTEM_ERROR               0x20010001
#define ERCD_EPS_SOCKET_ERROR                   0x20010002
#define ERCD_EPS_SOCKET_TIMEOUT                 0x20010003
#define ERCD_EPS_INVALID_PARM                   0x20010004
#define ERCD_EPS_DUPLICATE_INITED               0x20010005
#define ERCD_EPS_UNINITED                       0x20010006
#define ERCD_EPS_INVALID_CONNMODE               0x20010007
#define ERCD_EPS_INVALID_HID                    0x20010008
#define ERCD_EPS_DUPLICATE_REGISTERED           0x20010009
#define ERCD_EPS_INVALID_MKTTYPE                0x2001000a
#define ERCD_EPS_UNEXPECTED_MSGTYPE             0x2001000b
#define ERCD_EPS_INVALID_ADDRESS                0x2001000c
#define ERCD_EPS_DUPLICATE_CONNECT              0x2001000d
#define ERCD_EPS_MKTTYPE_UNSUBSCRIBED           0x2001000e
#define ERCD_EPS_MKTDATA_BACKFLOW               0x2001000f
#define ERCD_EPS_DATASOURCE_CHANGED             0x20010010
#define ERCD_EPS_MKTTYPE_DUPSUBSCRIBED          0x20010011
#define ERCD_EPS_INVALID_OPERATION              0x20010012
#define ERCD_EPS_LOGIN_FAILED                   0x20010013
#define ERCD_EPS_SUBMARKETDATA_FAILED           0x20010014
#define ERCD_EPS_CHECK_KEEPALIVE_TIMEOUT        0x20010015   
#define ERCD_EPS_HID_COUNT_BEYOND_LIMIT         0x20010016


/* STEP协议错误码 */
#define ERCD_STEP_INVALID_FLDVALUE              0x20020001
#define ERCD_STEP_BUFFER_OVERFLOW               0x20020002
#define ERCD_STEP_INVALID_FLDFORMAT             0x20020003
#define ERCD_STEP_INVALID_TAG                   0x20020004
#define ERCD_STEP_INVALID_MSGTYPE               0x20020005
#define ERCD_STEP_FLD_NOTFOUND                  0x20020006
#define ERCD_STEP_STREAM_NOT_ENOUGH             0x20020007
#define ERCD_STEP_INVALID_MSGFORMAT             0x20020008
#define ERCD_STEP_CHECKSUM_FAILED               0x20020009
#define ERCD_STEP_UNEXPECTED_TAG                0x2002000a

/* FAST错误码 */



#ifdef __cplusplus
}
#endif

#endif /* EPS_ERRCODE_H */

