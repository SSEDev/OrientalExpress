/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    uniQueue.c
 *
 * 单向队列实现文件
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

#include "common.h"
#include "epsTypes.h"
#include "errlib.h"
#include "errcode.h"

#include "uniQueue.h"


/**
 * 接口函数实现
 */

/**
 * 初始化单向队列
 *
 * @param   pQueue          in  - 单向队列对象
 * @param   size            in  - 单向队列大小
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT InitUniQueue(EpsUniQueueT* pQueue, uint32 size)
{
    TRY
    {
        if (pQueue == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "pQueue");
        }

        if (size == 0)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "size");
        }

        if (pQueue->container != NULL)
        {
            THROW_ERROR(ERCD_EPS_DUPLICATE_INITED, "UniQueue");
        }

        pQueue->container = calloc(size, sizeof(void*));
        if (pQueue->container == NULL)
        {
            int lstErrno = SYS_ERRNO;
            THROW_ERROR(ERCD_EPS_OPERSYSTEM_ERROR, EpsGetSystemError(lstErrno));
        }
        pQueue->size = size;
        pQueue->header = 0;
        pQueue->tailer = 0;
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/**
 * 反初始化单向队列
 *
 * @param   pQueue          in  - 单向队列对象
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT UninitUniQueue(EpsUniQueueT* pQueue)
{
    TRY
    {
        if (pQueue == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "pQueue");
        }

        if (pQueue->container != NULL)
        {
            free(pQueue->container);
            pQueue->container = NULL;
        }

        pQueue->size = 0;
        pQueue->header = 0;
        pQueue->tailer = 0;
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/**
 * 往单向队列尾部添加数据项
 *
 * @param   pQueue          in  - 单向队列对象
 * @param   pItem           in  - 待添加数据
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT PushUniQueue(EpsUniQueueT* pQueue, void* pItem)
{
    TRY
    {
        if (pQueue == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "pQueue");
        }

        if (pItem == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "pItem");
        }

        if (pQueue->container == NULL)
        {
            THROW_ERROR(ERCD_EPS_UNINITED, "UniQueue");
        }

        if (pQueue->header - pQueue->tailer >= pQueue->size)
        {
            THROW_ERROR(ERCD_EPS_INVALID_OPERATION, "UniQueue is full");
        }

        pQueue->container[pQueue->header++ % pQueue->size] = pItem;
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/**
 * 从单向队列头部提取数据项
 *
 *
 * @param   pQueue          in  - 单向队列对象
 * @param   ppItem          out - 待添加数据
 *
 * @return  成功返回NO_ERR，否则返回错误码
 */
ResCodeT PopUniQueue(EpsUniQueueT* pQueue, void** ppItem)
{
    TRY
    {
        if (pQueue == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "pQueue");
        }

        if (ppItem == NULL)
        {
            THROW_ERROR(ERCD_EPS_INVALID_PARM, "ppItem");
        }

        if (pQueue->container == NULL)
        {
            THROW_ERROR(ERCD_EPS_UNINITED, "UniQueue");
        }

        if (pQueue->tailer >= pQueue->header)
        {
            *ppItem = NULL;
        }
        else
        {
            *ppItem = pQueue->container[pQueue->tailer++ % pQueue->size];
        }
    }
    CATCH
    {
    }
    FINALLY
    {
        RETURN_RESCODE;
    }
}

/**
 * 判断单向队列是否已经初始化
 *
 * @param   pQueue          in  - 单向队列对象
 *
 * @return  已经初始化返回TRUE，否则返回FALSE
 */
BOOL IsUniQueueInited(EpsUniQueueT* pQueue)
{
    if (pQueue == NULL)
    {
        return FALSE;
    }

    return (pQueue->container != NULL);
}
