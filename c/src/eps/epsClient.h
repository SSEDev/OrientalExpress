/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    epsClient.h
 *
 * 东方快车接口API定义头文件
 *
 * @version $Id
 * @since   2014/02/14
 * @author  Wu Zheng
 */

/**
MODIFICATION HISTORY:
<pre>
================================================================================
DD-MMM-YYYY INIT.    SIR    Modification Description
----------- -------- ------ ----------------------------------------------------
17-FEB-2014 ZHENGWU         创建
================================================================================
</pre>
*/

#ifndef EPS_CLIENT_H
#define EPS_CLIENT_H

/**
 * 包含头文件
 */

#include "epsTypes.h"
#include "epsData.h"


#ifdef __cplusplus
extern "C" {
#endif


/**
 * 接口函数定义
 */

/**
 * 初始化Express库
 *
 * @return  成功返回1，否则返回错误码
 *
 * memo: 必须是第一个被调用的Express库函数
 */
int32 EpsInitLib();

/**
 * 反初始化Express库
 *
 * @return  成功返回1，否则返回错误码
 *
 * memo: 必须是最后一个调用的Express库函数，调用该函数会关闭所有句柄并释放资源
 */
int32 EpsUninitLib();

/**
 * 创建指定连接模式的操作句柄
 *
 * @param   pHid            out - 创建的句柄ID
 * @param   mode            in  - 连接模式
 *
 * @return  成功返回1，否则返回错误码
 *
 * memo: 句柄是用于执行后续连接、登陆、订阅等操作的基本单位；句柄ID用于唯一标识句柄。
 *       创建句柄用于分配相关资源
 */
int32 EpsCreateHandle(uint32* pHid, EpsConnModeT mode);

/**
 * 销毁句柄
 *
 * @param   hid             in  - 待销毁的句柄ID
 *
 * @return  成功返回1，否则返回错误码
 *
 * memo: 销毁句柄用于释放句柄相关资源
 */
int32 EpsDestroyHandle(uint32 hid);


/**
 * 注册用户回调接口
 *
 * @param   hid             in  - 待执行注册操作的句柄ID
 * @param   pSpi            in  - 待执行注册的用户回调接口
 *
 * @return  成功返回1，否则返回错误码
 *
 * memo: 注册用户回调接口，本接口在创建句柄成功后允许调用
 */
int32 EpsRegisterSpi(uint32 hid, const EpsClientSpiT* pSpi);

/**
 * 连接服务器
 *
 * @param   hid             in  - 待执行连接操作的句柄ID
 * @param   address         in  - 主机地址字符串，例如
 *                                TCP: 196.123.1.1:8000
 *                                UDP: 230.11.1.1:3333;196.123.71.1
 *
 * @return  成功返回1，否则返回错误码
 *
 * memo: 启动尝试连接指定地址的服务器，注意指定的服务器地址必须与连接模式相匹配，
 *       连接成功通过客户端回调函数connectedNotify通知用户，
 *       在调用EpsDisconnect()前，如连接断开，会自动尝试重连        
 */
int32 EpsConnect(uint32 hid, const char* address);

/**
 * 断开连接服务器
 *
 * @param   hid             in  - 待执行断开连接操作的句柄ID
 *
 * @return  成功返回1，否则返回错误码
 *
 * memo: 断开尝试连接服务器，调用成功后，用户将不再收到回调通知
 */
int32 EpsDisconnect(uint32 hid);

/**
 * 登陆服务器
 *
 * @param   hid             in  - 待执行登陆操作的句柄ID
 * @param   username        in  - 登陆用户名
 * @param   password        in  - 登录密码
 * @param   hearbeatIntl    in  - 心跳间隔
 *
 * @return  成功返回1，否则返回错误码
 *
 * memo: 向服务器发出登陆指令，本接口在连接成功后允许调用，登陆结果通过客户端回调函数loginRspNotify通知用户，
 *       本接口对于UDP模式的句柄是可选调用，总是返回成功。
 */
int32 EpsLogin(uint32 hid, const char* username, const char* password, uint16 heartbeatIntl);

/**
 * 登出服务器
 *
 * @param   hid             in  - 待执行登出操作的句柄ID
 *
 * @return  成功返回1，否则返回错误码
 *
 * memo: 向服务器发出登出指令，本接口在登陆成功后允许调用，登出结果通过客户端回调函数logoutRspNotify通知用户，
 *       本接口对于UDP模式的句柄是可选调用，总是返回成功。
 */
int32 EpsLogout(uint32 hid, const char* reason);

/**
 * 订阅指定市场类型的行情数据
 *
 * @param   hid             in  - 待执行订阅操作的句柄ID
 * @param   mktType         in  - 待订阅的市场类型
 *
 * @return  成功返回1，否则返回错误码
 *
 * memo: 向服务器发出订阅指令，本接口在登陆成功(TCP模式)或连接成功(UDP模式)后允许调用，
 *       订阅结果通过客户端回调函数mktDataSubRspNotify通知用户
 */
int32 EpsSubscribeMarketData(uint32 hid, EpsMktTypeT mktType);

/**
 * 获取最近一次错误信息描述
 *
 * @return  最近一次的错误描述信息
 *
 * memo: 当调用接口函数失败或回调接口显示失败时允许调用本接口，
 *       调用本接口获取的错误信息描述是线程相关的
 */
const char* EpsGetLastError();


#ifdef __cplusplus
}
#endif

#endif /* EPS_CLIENT_H */
