/*
 * Copyright (C) 2013, 2014 Shanghai Stock Exchange (SSE), Shanghai, China
 * All Rights Reserved.
 */

/*
 * @file    stepCodec.h

 * STEP协议编解码器定义头文件 

 * @version $Id
 * @since   2013/10/19
 * @author  Jin Chengxun 
*/

/*
 MODIFICATION HISTORY:
 <pre>
 ================================================================================
 DD-MMM-YYYY INIT.    SIR    Modification Description
 ----------- -------- ------ ----------------------------------------------------
 19-10-2013  CXJIN    创建
 11-AUG-2014 ZHENGWU  #5010  根据LFIXT会话协议规范调整
 ================================================================================
  </pre>
*/

#ifndef EPS_STEP_CODEC_H
#define EPS_STEP_CODEC_H

/*
 * 包含头文件
 */

#include "stepMessage.h"


#ifdef __cplusplus
extern "C" {
#endif

/*
 * 函数申明
 */

/*
 * 编码STEP消息
 */
ResCodeT EncodeStepMessage(StepMessageT* pMsg, char* buf, int32 bufSize, int32* pEncodeSize);

/*
 * 解码STEP消息
 */
ResCodeT DecodeStepMessage(const char* buf, int32 bufSize, StepMessageT* pMsg, int32* pDecodeSize);

/*
 * 校验STEP消息
 */
ResCodeT ValidateStepMessage(const StepMessageT* pMsg, StepDirectionT direction);

#ifdef __cplusplus
}
#endif

#endif /* EPS_STEP_CODEC_H */
