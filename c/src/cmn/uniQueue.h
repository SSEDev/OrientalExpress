/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    uniQueue.h
 *
 * 单向队列定义头文件
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

#ifndef EPS_UNIQUEUE_H
#define EPS_UNIQUEUE_H


#ifdef __cplusplus
extern "C" {
#endif


/**
 * 类型定义
 */

/*
 * 递归互斥锁结构
 */
typedef struct EpsUniQueueTag
{
    uint32      size;       /* 队列大小 */
    void**      container;  /* 队列容器 */
    uint32      header;     /* 队列头部位移 */
    uint32      tailer;     /* 队列尾部位移 */
} EpsUniQueueT;


/**
 * 接口函数定义
 */

/*
 * 初始化单向队列
 */
ResCodeT InitUniQueue(EpsUniQueueT* pQueue, uint32 size);

/*
 * 反初始化单向队列
 */
ResCodeT UninitUniQueue(EpsUniQueueT* pQueue);

/*
 * 往单向队列尾部添加数据项
 */
ResCodeT PushUniQueue(EpsUniQueueT* pQueue, void* pItem);

/*
 * 从单向队列头部提取数据项
 */
ResCodeT PopUniQueue(EpsUniQueueT* pQueue, void** ppItem);

/*
 * 判断单向队列是否已经初始化
 */
BOOL IsUniQueueInited(EpsUniQueueT* pQueue);


#ifdef __cplusplus
}
#endif

#endif /* EPS_UNIQUEUE_H */
