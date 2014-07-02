/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/**
 * @file    recMutex.c
 *
 * µÝ¹é»¥³âËø¶¨ÒåÊµÏÖÎÄ¼þ
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
11-APR-2014 ZHENGWU         ´´½¨
================================================================================
</pre>
*/

/**
 * °üº¬Í·ÎÄ¼þ
 */

#include "recMutex.h"


/**
 * ½Ó¿Úº¯ÊýÊµÏÖ
 */

/**
 * ³õÊ¼»¯µÝ¹é»¥³âËø
 *
 * @param   pMutex                  in  - µÝ¹é»¥³âËø
 */
void InitRecMutex(EpsRecMutexT* pMutex)
{
    if (pMutex == NULL)
    {
        return;
    }

#if defined(__WINDOWS__)  
	pMutex->mutex = CreateMutex(NULL, FALSE, NULL);
#endif  
  
#if defined(__LINUX__) || defined(__HPUX__) 
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

    pthread_mutex_init(&pMutex->mutex, &attr);
#endif  
}

/**
 * ·´³õÊ¼»¯µÝ¹é»¥³âËø
 *
 * @param   pMutex                  in  - µÝ¹é»¥³âËø
 */
void UninitRecMutex(EpsRecMutexT* pMutex)
{
    if (pMutex == NULL)
    {
        return;
    }

#if defined(__WINDOWS__)  
	CloseHandle(pMutex->mutex);
#endif  
  
#if defined(__LINUX__) || defined(__HPUX__) 
    pthread_mutex_destroy(&pMutex->mutex);
#endif  
}

/**
 * ¶ÔµÝ¹é»¥³âËø¼ÓËø
 *
 * @param   pMutex                  in  - µÝ¹é»¥³âËø
 */
void LockRecMutex(EpsRecMutexT* pMutex)
{
    if (pMutex == NULL)
    {
        return;
    }

#if defined(__WINDOWS__)  
	WaitForSingleObject(pMutex->mutex, INFINITE);
#endif  
  
#if defined(__LINUX__) || defined(__HPUX__) 
    pthread_mutex_lock(&pMutex->mutex);
#endif 
}

/**
 * ¶ÔµÝ¹é»¥³âËø½âËø
 *
 * @param   pMutex                  in  - µÝ¹é»¥³âËø
 */
void UnlockRecMutex(EpsRecMutexT* pMutex)
{
    if (pMutex == NULL)
    {
        return;
    }

#if defined(__WINDOWS__)  
	ReleaseMutex(pMutex->mutex);
#endif  
  
#if defined(__LINUX__) || defined(__HPUX__) 
    pthread_mutex_unlock(&pMutex->mutex);
#endif 
}
