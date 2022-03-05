#ifndef __GO_CPSV_H__
#define __GO_CPSV_H__
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <saCkpt.h>
#include <time.h>

#define DEMO_CKPT_NAME "safCkpt=DemoCkpt,safApp=safCkptService"

void AppCkptOpenCallback(SaInvocationT invocation,
			 SaCkptCheckpointHandleT checkpointHandle,
			 SaAisErrorT error);
void AppCkptSyncCallback(SaInvocationT invocation, SaAisErrorT error);
void cpsv_sync_app_process(void *info, char* data);

#endif