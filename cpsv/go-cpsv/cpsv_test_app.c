#include "go-cpsv.h"


void AppCkptOpenCallback(SaInvocationT invocation,
			 SaCkptCheckpointHandleT checkpointHandle,
			 SaAisErrorT error)
{
	if (error != SA_AIS_OK) {
		printf("Checkpoint Open Async callback unsuccessful\n");
		return;
	} else {

		printf(
		    "Checkpoint Open Async callback success and ckpt_hdl %llu \n",
		    checkpointHandle);
		return;
	}
}
void AppCkptSyncCallback(SaInvocationT invocation, SaAisErrorT error)
{
	if (error != SA_AIS_OK) {
		printf("Checkpoint Sync Callback unsuccessful\n");
		return;
	} else {
		printf("Checkpoint Sync Callback success\n");
		return;
	}
}

/****************************************************************************
 * Name          : cpsv_sync_app_process
 *
 * Description   : This is the function which is given as the input to the
 *                 Application task.
 *
 * Arguments     : info  - This is the information which is passed during
 *                         spawing Application task.
 *
 * Return Values : None.
 *
 * Notes         : None.
 *****************************************************************************/
void cpsv_sync_app_process(void *info, char* data)
{
	SaCkptHandleT ckptHandle;
	SaCkptCheckpointHandleT checkpointHandle;
	SaCkptCallbacksT callbk;
	SaVersionT version;
	SaNameT ckptName;
	SaAisErrorT rc;
	SaCkptCheckpointCreationAttributesT ckptCreateAttr;
	SaCkptCheckpointOpenFlagsT ckptOpenFlags;
	SaCkptSectionCreationAttributesT sectionCreationAttributes;
	SaCkptIOVectorElementT writeVector, readVector;
	SaUint32T erroneousVectorIndex;
	void *initialData = "Default data in the section";
	unsigned char read_buff[100] = {0};
	SaTimeT timeout = 1000000000;
	unsigned int temp_var = (unsigned int)(long)info;

	memset(&ckptName, 0, 255);
	ckptName.length = strlen(DEMO_CKPT_NAME);
	memcpy(ckptName.value, DEMO_CKPT_NAME, strlen(DEMO_CKPT_NAME));

	callbk.saCkptCheckpointOpenCallback = AppCkptOpenCallback;
	callbk.saCkptCheckpointSynchronizeCallback = AppCkptSyncCallback;
	version.releaseCode = 'B';
	version.majorVersion = 2;
	version.minorVersion = 2;

	printf(
	    "*******************************************************************\n");
	printf(
	    "Demonstrating Checkpoint Service Usage with a collocated Checkpoint \n");
	printf(
	    "*******************************************************************\n");
	sleep(2);

	printf("Initialising With Checkpoint Service....\n");
	rc = saCkptInitialize(&ckptHandle, &callbk, &version);
	if (rc == SA_AIS_OK)
		printf("PASSED \n");
	else
		printf("Failed \n");

	ckptCreateAttr.creationFlags =
	    SA_CKPT_CHECKPOINT_COLLOCATED | SA_CKPT_WR_ACTIVE_REPLICA;
	ckptCreateAttr.checkpointSize = 1024;
	ckptCreateAttr.retentionDuration = 100000;
	ckptCreateAttr.maxSections = 2;
	ckptCreateAttr.maxSectionSize = 700;
	ckptCreateAttr.maxSectionIdSize = 4;

	ckptOpenFlags = SA_CKPT_CHECKPOINT_CREATE | SA_CKPT_CHECKPOINT_READ |
			SA_CKPT_CHECKPOINT_WRITE;
	printf("Opening Collocated Checkpoint = %s with create flags....\n",
	       ckptName.value);
	rc = saCkptCheckpointOpen(ckptHandle, &ckptName, &ckptCreateAttr,
				  ckptOpenFlags, timeout, &checkpointHandle);
	if (rc == SA_AIS_OK)
		printf("PASSED \n");
	else
		printf("Failed \n");

	if (temp_var == 1) {

		printf("Setting the Active Replica for my checkpoint ....\t");
		rc = saCkptActiveReplicaSet(checkpointHandle);
		if (rc == SA_AIS_OK)
			printf("PASSED \n");
		else
			printf("Failed \n");

		sectionCreationAttributes.sectionId =
		    (SaCkptSectionIdT *)malloc(sizeof(SaCkptSectionIdT));
		sectionCreationAttributes.sectionId->id = (unsigned char *)"11";
		sectionCreationAttributes.sectionId->idLen = 2;
		/* Cpsv expects `expirationTime` as  absolute time
		   check  section 3.4.3.2 SaCkptSectionCreationAttributesT of
		   CKPT Specification for more details  */
		sectionCreationAttributes.expirationTime =
		    (SA_TIME_ONE_HOUR +
		     (time((time_t *)0) * 1000000000)); /* One Hour */

		printf("Created Section ....\t");
		rc = saCkptSectionCreate(checkpointHandle,
					 &sectionCreationAttributes,
					 initialData, 28);
		if (rc == SA_AIS_OK)
			printf("PASSED \n");
		else
			printf("Failed \n");

		writeVector.sectionId.id = (unsigned char *)"11";
		writeVector.sectionId.idLen = 2;
		writeVector.dataBuffer = data;
		writeVector.dataSize = strlen(writeVector.dataBuffer);
		writeVector.dataOffset = 0;
		writeVector.readSize = 0;

		printf("Writing to Checkpoint %s ....\n", DEMO_CKPT_NAME);
		printf("Section-Id = %s ....\n", writeVector.sectionId.id);
		printf("CheckpointData being written = \"%s\"\n",
		       (char *)writeVector.dataBuffer);
		printf("DataOffset = %llu ....\n", writeVector.dataOffset);
		rc = saCkptCheckpointWrite(checkpointHandle, &writeVector, 1,
					   &erroneousVectorIndex);
		if (rc == SA_AIS_OK)
			printf("PASSED \n");
		else
			printf("Failed \n");
		sleep(1);

		printf("Press <Enter> key to continue...\n");
		getchar();
	} else {
		sleep(4);
		readVector.sectionId.id = (unsigned char *)"11";
		readVector.sectionId.idLen = 2;
		readVector.dataBuffer = read_buff;
		readVector.dataSize = 90;
		readVector.dataOffset = 0;
		printf("Waiting to Read from Checkpoint %s....\n",
		       DEMO_CKPT_NAME);
		printf("Press <Enter> key to continue...\n");
		getchar();
		rc = saCkptCheckpointRead(checkpointHandle, &readVector, 1,
					  &erroneousVectorIndex);
		printf("Checkpoint Data Read = \"%s\"\n",
		       (char *)readVector.dataBuffer);
		if (rc == SA_AIS_OK)
			printf("PASSED \n");
		else
			printf("Failed \n");
	}
	printf("Synchronizing My Checkpoint being called ....\n");
	rc = saCkptCheckpointSynchronize(checkpointHandle, timeout);
	if (rc == SA_AIS_OK)
		printf("PASSED \n");
	else
		printf("Failed \n");

	if (temp_var == 1) {
		printf("Unlink My Checkpoint ....\t");
		rc = saCkptCheckpointUnlink(ckptHandle, &ckptName);
		if (rc == SA_AIS_OK)
			printf("PASSED \n");
		else
			printf("Failed \n");
	}
	printf("Ckpt Closed ....\t");
	rc = saCkptCheckpointClose(checkpointHandle);
	if (rc == SA_AIS_OK)
		printf("PASSED \n");
	else
		printf("Failed \n");

	printf("Ckpt Finalize being called ....\t");
	rc = saCkptFinalize(ckptHandle);
	if (rc == SA_AIS_OK)
		printf("PASSED \n");
	else
		printf("Failed \n");

	sleep(2);
	return;
}
