/**********************************************************************
*File name 		:   Xml.c
*Copyright      :   Zhuzhou CRRC Times Electric Co. Ltd ALL Rights Reserved
*Creat Date     :   2018/8/16
*Description    :   extract the parameter of xml 
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/08/16  Create
*********************************************************************/

/**********************************************************************
*
*Debug switch  section
*
*********************************************************************/


/**********************************************************************
*
*Include file  section
*
*********************************************************************/
#include "xml.h"
/**********************************************************************
*
*Global Variable Declare Section
*
*********************************************************************/
extern DEBUG_TYPE_ENUM g_DebugType_EU;
/**********************************************************************
*
*Local Macro Define Section
*
*********************************************************************/

/**********************************************************************
*
*Local Struct Define Section
*
*********************************************************************/

/**********************************************************************
*
*Local Prototype Declare Section
*
*********************************************************************/

/**********************************************************************
*
*Static Variable Define Section
*
*********************************************************************/


/**********************************************************************
*
*Function Define Section
*
*********************************************************************/

/**********************************************************************
*Name			:   int8_t RecordCFGXml(char *xmlFilePath,RECORD_XML *RecordXml_p)
*Function       :   Extra the xml information of every line
*Para     		:   char *xmlFilePath, the path of xml
*					RECORD_XML *RecordXml_p
*Return    		:   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/5/27  Create
*********************************************************************/
int8_t RelayCfgXml(char * xmlFilePath,RECORD_XML *RecordXml_p,RELAY_LIFE_CFG *LifeCfg)
{
	char xmlarray[ZONE] = {0};
	char loginfo[LOG_INFO_LENG] = {0};
	uint8_t i = 0;
	uint8_t TypeIndex = 0;
	FILE* fp = fopen(xmlFilePath,"r");
			 //从prac.xml文件中加载xml
	if(fp  == NULL)
    {
        perror("open RelayCfgXml failed");
        snprintf(loginfo, sizeof(loginfo)-1, "open %s file failed",xmlFilePath);
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        return CODE_ERR;
    }	
	mxml_node_t *xml = mxmlLoadFile(NULL,fp,MXML_NO_CALLBACK);

	//载入完毕可关闭文件
	fclose(fp);
	//遍历节点
	mxml_node_t* tmp = NULL;
	mxml_node_t* elem_tmp = NULL;

	elem_tmp=mxmlFindElement(xml, xml, "RecConfVer", NULL, NULL, MXML_DESCEND);
	printf("RecConfVer=[%s]\n",mxmlGetText(elem_tmp, 0));

	tmp=mxmlFindElement(xml, xml, "FacCfg", NULL, NULL, MXML_DESCEND);
	memset(xmlarray,0,ZONE);
	memcpy(xmlarray,mxmlElementGetAttr(tmp, "RelayNumFactor"),strlen(mxmlElementGetAttr(tmp, "RelayNumFactor")));
	LifeCfg -> RelayNumFactor = atof(xmlarray);

	memset(xmlarray,0,ZONE);
	memcpy(xmlarray,mxmlElementGetAttr(tmp, "UpOprtTimeFactor"),strlen(mxmlElementGetAttr(tmp, "UpOprtTimeFactor")));
	LifeCfg -> UpOprtTimeFactor = atof(xmlarray);

	memset(xmlarray,0,ZONE);
	memcpy(xmlarray,mxmlElementGetAttr(tmp, "DownOprtTimeFactor"),strlen(mxmlElementGetAttr(tmp, "DownOprtTimeFactor")));
	LifeCfg -> DownOprtTimeFactor = atof(xmlarray);

	memset(xmlarray,0,ZONE);
	memcpy(xmlarray,mxmlElementGetAttr(tmp, "JumpTimeFactor"),strlen(mxmlElementGetAttr(tmp, "JumpTimeFactor")));
	LifeCfg -> JumpTimeFactor = atof(xmlarray);

	for(i= 0; i< RELAYCFG_NUM;i++)
	{
		elem_tmp=mxmlFindElement(tmp, xml, "RelayCfg", NULL, NULL, MXML_DESCEND);
		memset(xmlarray,0,ZONE);
		memcpy(xmlarray,mxmlElementGetAttr(elem_tmp,"NumIdx"),strlen(mxmlElementGetAttr(elem_tmp,"NumIdx")));
		TypeIndex = atoi(xmlarray);
		if(TypeIndex)
		{
	
			tmp=mxmlFindElement(elem_tmp, xml, "LifeCfg", NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlElementGetAttr(tmp, "RelayLifeNum"),strlen(mxmlElementGetAttr(tmp, "RelayLifeNum")));
			LifeCfg -> RelayChanCfg_ST[TypeIndex -1].RelayOprtLife_U32 = strtol(xmlarray,NULL,10);
			
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlElementGetAttr(tmp, "UpOprtTimeMin"),strlen(mxmlElementGetAttr(tmp, "UpOprtTimeMin")));
			LifeCfg -> RelayChanCfg_ST[TypeIndex-1].UpOprtTimeMin_U16 = atoi(xmlarray);

			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlElementGetAttr(tmp, "UpOprtTimeMax"),strlen(mxmlElementGetAttr(tmp, "UpOprtTimeMax")));
			LifeCfg -> RelayChanCfg_ST[TypeIndex-1].UpOprtTimeMax_U16 = atoi(xmlarray);

			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlElementGetAttr(tmp, "DownOprtTimeMin"),strlen(mxmlElementGetAttr(tmp, "DownOprtTimeMin")));
			LifeCfg -> RelayChanCfg_ST[TypeIndex-1].DownOprtTimeMin_U16 = atoi(xmlarray);

			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlElementGetAttr(tmp, "DownOprtTimeMax"),strlen(mxmlElementGetAttr(tmp, "DownOprtTimeMax")));
			LifeCfg -> RelayChanCfg_ST[TypeIndex-1].DownOprtTimeMax_U16 = atoi(xmlarray);

			memset(xmlarray,0,ZONE);			
			memcpy(xmlarray,mxmlElementGetAttr(tmp, "JumpTimeMin"),strlen(mxmlElementGetAttr(tmp, "JumpTimeMin")));
			LifeCfg -> RelayChanCfg_ST[TypeIndex-1].JumpTimeMin_U16 = atoi(xmlarray);

			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlElementGetAttr(tmp, "JumpTimeMax"),strlen(mxmlElementGetAttr(tmp, "JumpTimeMax")));
			LifeCfg -> RelayChanCfg_ST[TypeIndex-1].JumpTimeMax_U16 = atoi(xmlarray);
		} 
		
	}

	elem_tmp=mxmlFindElement(tmp, xml, "RecTypeIdx",NULL, NULL, MXML_DESCEND);
	memset(xmlarray,0,ZONE);
	memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
	RecordXml_p -> Rec_Event_ST.RecTypeIdx = atoi(xmlarray);

	elem_tmp=mxmlFindElement(tmp, xml, "RecPath",NULL, NULL, MXML_DESCEND);
	memcpy(&RecordXml_p -> Rec_Event_ST.RecPath,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));
	elem_tmp=mxmlFindElement(tmp, xml, "RecFileHead",NULL, NULL, MXML_DESCEND);
	memcpy(&RecordXml_p -> Rec_Event_ST.RecFileHead,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));
	
	elem_tmp=mxmlFindElement(tmp, xml, "RecTotalChn",NULL, NULL, MXML_DESCEND);
	memset(xmlarray,0,ZONE);
	memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
	RecordXml_p -> Rec_Event_ST.RecTotalChn_U8 = atoi(xmlarray);
	
	elem_tmp=mxmlFindElement(tmp, xml, "RecChnNum",NULL, NULL, MXML_DESCEND);
	memset(xmlarray,0,ZONE);
	memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
	RecordXml_p -> Rec_Event_ST.RecChnNum_U8 = atoi(xmlarray);
	
	elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask1",NULL, NULL, MXML_DESCEND);
	memset(xmlarray,0,ZONE);
	memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
	RecordXml_p -> Rec_Event_ST.RecChnBitMask1 = strtol(xmlarray,NULL,16);
	
	elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask2",NULL, NULL, MXML_DESCEND);
	memset(xmlarray,0,ZONE);
	memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
	RecordXml_p -> Rec_Event_ST.RecChnBitMask2 = strtol(xmlarray,NULL,16);
	
	elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask3",NULL, NULL, MXML_DESCEND);
	memset(xmlarray,0,ZONE);
	memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
	RecordXml_p -> Rec_Event_ST.RecChnBitMask3 = strtol(xmlarray,NULL,16);
	
	elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask4",NULL, NULL, MXML_DESCEND);
	memset(xmlarray,0,ZONE);
	memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
	RecordXml_p -> Rec_Event_ST.RecChnBitMask4 = strtol(xmlarray,NULL,16);
	
	elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask5",NULL, NULL, MXML_DESCEND);
	memset(xmlarray,0,ZONE);
	memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
	RecordXml_p -> Rec_Event_ST.RecChnBitMask5 = strtol(xmlarray,NULL,16);
	
	elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask6",NULL, NULL, MXML_DESCEND);
	memset(xmlarray,0,ZONE);
	memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
	RecordXml_p -> Rec_Event_ST.RecChnBitMask6 = strtol(xmlarray,NULL,16);
	
	elem_tmp=mxmlFindElement(tmp, xml, "RecInterval",NULL, NULL, MXML_DESCEND);
	memset(xmlarray,0,ZONE);
	memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
	RecordXml_p -> Rec_Event_ST.RecInterval = strtol(xmlarray,NULL,10);
	
	elem_tmp=mxmlFindElement(tmp, xml, "RecBeforeNum",NULL, NULL, MXML_DESCEND);
	memset(xmlarray,0,ZONE);
	memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
	RecordXml_p -> Rec_Event_ST.RecBeforeNum = strtol(xmlarray,NULL,10);			
	
	elem_tmp=mxmlFindElement(tmp, xml, "RecAfterNum",NULL, NULL, MXML_DESCEND);
	memset(xmlarray,0,ZONE);
	memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
	RecordXml_p -> Rec_Event_ST.RecAfterNum = strtol(xmlarray,NULL,10); 		
	
	elem_tmp=mxmlFindElement(tmp, xml, "RecToTalNum",NULL, NULL, MXML_DESCEND);
	memset(xmlarray,0,ZONE);
	memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
	RecordXml_p -> Rec_Event_ST.RecToTalNum = strtol(xmlarray,NULL,10);

	mxmlDelete(xml);
	if(XML_DEBUG == g_DebugType_EU)
	{
		printf("RelayNumFactor %f\n", LifeCfg -> RelayNumFactor);
		printf("UpOprtTimeFactor %f\n", LifeCfg -> UpOprtTimeFactor);
		printf("DownOprtTimeFactor %f\n", LifeCfg -> DownOprtTimeFactor);
		printf("JumpTimeFactor %f\n", LifeCfg -> JumpTimeFactor);

		for(i= 0; i< RELAYCFG_NUM;i++)
		{
			printf("%d \n",i);
			printf("RelayOprtLife_U32 %d\n", LifeCfg ->  RelayChanCfg_ST[i].RelayOprtLife_U32);
			printf("UpOprtTimeMin_U16 %d\n", LifeCfg -> RelayChanCfg_ST[i].UpOprtTimeMin_U16);
			printf("UpOprtTimeMax_U16 %d\n", LifeCfg -> RelayChanCfg_ST[i].UpOprtTimeMax_U16);
			printf("DownOprtTimeMin_U16 %d\n", LifeCfg -> RelayChanCfg_ST[i].DownOprtTimeMin_U16);
			printf("DownOprtTimeMax_U16 %d\n", LifeCfg -> RelayChanCfg_ST[i].DownOprtTimeMax_U16);
			printf("JumpTimeMin_U16 %d\n", LifeCfg -> RelayChanCfg_ST[i].JumpTimeMin_U16);
			printf("JumpTimeMax_U16 %d\n", LifeCfg -> RelayChanCfg_ST[i].JumpTimeMax_U16);
	
		}
		printf("Rec_Event_ST\n");
		printf("RecChnNum_U8 %d\n", RecordXml_p -> Rec_Event_ST.RecChnNum_U8);
		printf("RecChnBitMask1 %x\n", RecordXml_p -> Rec_Event_ST.RecChnBitMask1);
		printf("RecChnBitMask2 %x\n", RecordXml_p -> Rec_Event_ST.RecChnBitMask2);
		printf("RecChnBitMask3 %x\n", RecordXml_p -> Rec_Event_ST.RecChnBitMask3);
		printf("RecChnBitMask4 %x\n", RecordXml_p -> Rec_Event_ST.RecChnBitMask4);
		printf("RecChnBitMask5 %x\n", RecordXml_p -> Rec_Event_ST.RecChnBitMask5);
		printf("RecChnBitMask6 %x\n", RecordXml_p -> Rec_Event_ST.RecChnBitMask6);
		printf("RecInterval %d\n", RecordXml_p -> Rec_Event_ST.RecInterval);
		printf("RecBeforeNum %d\n", RecordXml_p -> Rec_Event_ST.RecBeforeNum);
		printf("RecAfterNum %d\n", RecordXml_p -> Rec_Event_ST.RecAfterNum);
		printf("RecToTalNum %u\n", RecordXml_p -> Rec_Event_ST.RecToTalNum);

	}
	return CODE_OK;	
}
/**********************************************************************
*Name			:   int8_t RecordCFGXml(char *xmlFilePath,RECORD_XML *RecordXml_p)
*Function       :   Extra the xml information of every line
*Para     		:   char *xmlFilePath, the path of xml
*					RECORD_XML *RecordXml_p
*Return    		:   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/5/27  Create
*********************************************************************/
int8_t RecordCFGXml(char *xmlFilePath,RECORD_XML *RecordXml_p,VERSION *Versionp)
{
   uint8_t i =0; 
   char xmlarray[ZONE] = {0};
   	char loginfo[LOG_INFO_LENG] = {0};
	FILE* fp = fopen(xmlFilePath,"r");
	if(fp  == NULL)
    {
        perror("open RecordCFGXml failed");
        snprintf(loginfo, sizeof(loginfo)-1, "open %s file failed",xmlFilePath);
        WRITELOGFILE(LOG_ERROR_1,loginfo);
        return CODE_ERR;

    }	
	mxml_node_t *xml = mxmlLoadFile(NULL,fp,MXML_NO_CALLBACK);
	fclose(fp);
	
	mxml_node_t* tmp = NULL;
	mxml_node_t* elem_tmp = NULL;
	uint8_t TypeIndex = 0;
	//xml文件版本
	tmp=mxmlFindElement(xml, xml, "RecConfVer", NULL, NULL, MXML_DESCEND);
	memcpy(xmlarray,mxmlGetText(tmp, 0),strlen(mxmlGetText(tmp, 0)));
	Versionp -> RrdCfgFile_Ver_U16 = atoi(xmlarray);
	//记录文件配置信息
	tmp=mxmlFindElement(xml, xml, "RecTypeCfg", NULL, NULL, MXML_DESCEND);
	elem_tmp=mxmlFindElement(tmp, xml, "RecType", NULL, NULL, MXML_DESCEND);
    for(i= 0; i< 6;i++)//i代表向下读取次数，若TypeIndex有6个，则要读6次
    {
		tmp=mxmlFindElement(elem_tmp, xml, "RecTypeIdx", NULL, NULL, MXML_DESCEND);
		memset(xmlarray,0,ZONE);
		memcpy(xmlarray,mxmlGetText(tmp, 0),strlen(mxmlGetText(tmp, 0)));
		TypeIndex = atoi(xmlarray);
		
		if(EVENT_FILE == TypeIndex)
		{
            RecordXml_p -> Rec_Event_ST.RecTypeIdx = TypeIndex;

			elem_tmp=mxmlFindElement(tmp, xml, "RecPath",NULL, NULL, MXML_DESCEND);
			memcpy(&RecordXml_p -> Rec_Event_ST.RecPath,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));
			elem_tmp=mxmlFindElement(tmp, xml, "RecFileHead",NULL, NULL, MXML_DESCEND);
			memcpy(&RecordXml_p -> Rec_Event_ST.RecFileHead,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecTotalChn",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_Event_ST.RecTotalChn_U8 = atoi(xmlarray);
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecChnNum",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_Event_ST.RecChnNum_U8 = atoi(xmlarray);
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask1",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_Event_ST.RecChnBitMask1 = strtol(xmlarray,NULL,16);
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask2",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_Event_ST.RecChnBitMask2 = strtol(xmlarray,NULL,16);
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask3",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_Event_ST.RecChnBitMask3 = strtol(xmlarray,NULL,16);
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask4",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_Event_ST.RecChnBitMask4 = strtol(xmlarray,NULL,16);
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask5",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_Event_ST.RecChnBitMask5 = strtol(xmlarray,NULL,16);
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask6",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_Event_ST.RecChnBitMask6 = strtol(xmlarray,NULL,16);
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecInterval",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_Event_ST.RecInterval = strtol(xmlarray,NULL,10);
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecBeforeNum",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_Event_ST.RecBeforeNum = strtol(xmlarray,NULL,10);			
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecAfterNum",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_Event_ST.RecAfterNum = strtol(xmlarray,NULL,10); 		
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecToTalNum",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_Event_ST.RecToTalNum = strtol(xmlarray,NULL,10);
		}
		else if(FAST_FILE == TypeIndex)
		{
			RecordXml_p -> Rec_Fast_ST.RecTypeIdx = TypeIndex;
			
			 elem_tmp=mxmlFindElement(tmp, xml, "RecPath",NULL, NULL, MXML_DESCEND);
			 memcpy(&RecordXml_p -> Rec_Fast_ST.RecPath,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));
			 elem_tmp=mxmlFindElement(tmp, xml, "RecFileHead",NULL, NULL, MXML_DESCEND);
			 memcpy(&RecordXml_p -> Rec_Fast_ST.RecFileHead,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));
			 
			 elem_tmp=mxmlFindElement(tmp, xml, "RecTotalChn",NULL, NULL, MXML_DESCEND);
			 memset(xmlarray,0,ZONE);
			 memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));	 
			 RecordXml_p -> Rec_Fast_ST.RecTotalChn_U8 = atoi(xmlarray);
			 
			 elem_tmp=mxmlFindElement(tmp, xml, "RecChnNum",NULL, NULL, MXML_DESCEND);
			 memset(xmlarray,0,ZONE);
			 memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));	 
			 RecordXml_p -> Rec_Fast_ST.RecChnNum_U8 = atoi(xmlarray);
			 
			 elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask1",NULL, NULL, MXML_DESCEND);
			 memset(xmlarray,0,ZONE);
			 memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));	 
			 RecordXml_p -> Rec_Fast_ST.RecChnBitMask1 = strtol(xmlarray,NULL,16);
			 
			 elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask2",NULL, NULL, MXML_DESCEND);
			 memset(xmlarray,0,ZONE);
			 memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));	 
			 RecordXml_p -> Rec_Fast_ST.RecChnBitMask2 = strtol(xmlarray,NULL,16);
			 
			 elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask3",NULL, NULL, MXML_DESCEND);
			 memset(xmlarray,0,ZONE);
			 memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));	 
			 RecordXml_p -> Rec_Fast_ST.RecChnBitMask3 = strtol(xmlarray,NULL,16);
			 
			 elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask4",NULL, NULL, MXML_DESCEND);
			 memset(xmlarray,0,ZONE);
			 memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));	 
			 RecordXml_p -> Rec_Fast_ST.RecChnBitMask4 = strtol(xmlarray,NULL,16);
			 
			 elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask5",NULL, NULL, MXML_DESCEND);
			 memset(xmlarray,0,ZONE);
			 memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));	 
			 RecordXml_p -> Rec_Fast_ST.RecChnBitMask5 = strtol(xmlarray,NULL,16);
			 
			 elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask6",NULL, NULL, MXML_DESCEND);
			 memset(xmlarray,0,ZONE);
			 memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));	 
			 RecordXml_p -> Rec_Fast_ST.RecChnBitMask6 = strtol(xmlarray,NULL,16);
			 
			 elem_tmp=mxmlFindElement(tmp, xml, "RecInterval",NULL, NULL, MXML_DESCEND);
			 memset(xmlarray,0,ZONE);
			 memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));	 
			 RecordXml_p -> Rec_Fast_ST.RecInterval = strtol(xmlarray,NULL,10);
			 
			 elem_tmp=mxmlFindElement(tmp, xml, "RecBeforeNum",NULL, NULL, MXML_DESCEND);
			 memset(xmlarray,0,ZONE);
			 memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));	 
			 RecordXml_p -> Rec_Fast_ST.RecBeforeNum = strtol(xmlarray,NULL,10);			 
			 
			 elem_tmp=mxmlFindElement(tmp, xml, "RecAfterNum",NULL, NULL, MXML_DESCEND);
			 memset(xmlarray,0,ZONE);
			 memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));	 
			 RecordXml_p -> Rec_Fast_ST.RecAfterNum = strtol(xmlarray,NULL,10);		 
			 
			 elem_tmp=mxmlFindElement(tmp, xml, "RecToTalNum",NULL, NULL, MXML_DESCEND);
			 memset(xmlarray,0,ZONE);
			 memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));	 
			 RecordXml_p -> Rec_Fast_ST.RecToTalNum = strtol(xmlarray,NULL,10);
		}
		else if(REAL_FILE == TypeIndex)
		{
			 RecordXml_p -> Rec_Real_ST.RecTypeIdx = TypeIndex;			
			 elem_tmp=mxmlFindElement(tmp, xml, "RecPath",NULL, NULL, MXML_DESCEND);
			 memcpy(&RecordXml_p -> Rec_Real_ST.RecPath,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));
			 elem_tmp=mxmlFindElement(tmp, xml, "RecFileHead",NULL, NULL, MXML_DESCEND);
			 memcpy(&RecordXml_p -> Rec_Real_ST.RecFileHead,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));
			 
			 elem_tmp=mxmlFindElement(tmp, xml, "RecTotalChn",NULL, NULL, MXML_DESCEND);
			 memset(xmlarray,0,ZONE);
			 memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));	 
			 RecordXml_p -> Rec_Real_ST.RecTotalChn_U8 = atoi(xmlarray);
			 
			 elem_tmp=mxmlFindElement(tmp, xml, "RecChnNum",NULL, NULL, MXML_DESCEND);
			 memset(xmlarray,0,ZONE);
			 memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));	 
			 RecordXml_p -> Rec_Real_ST.RecChnNum_U8 = atoi(xmlarray);
			 
			 elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask1",NULL, NULL, MXML_DESCEND);
			 memset(xmlarray,0,ZONE);
			 memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));	 
			 RecordXml_p -> Rec_Real_ST.RecChnBitMask1 = strtol(xmlarray,NULL,16);
			 
			 elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask2",NULL, NULL, MXML_DESCEND);
			 memset(xmlarray,0,ZONE);
			 memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));	 
			 RecordXml_p -> Rec_Real_ST.RecChnBitMask2 = strtol(xmlarray,NULL,16);
			 
			 elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask3",NULL, NULL, MXML_DESCEND);
			 memset(xmlarray,0,ZONE);
			 memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));	 
			 RecordXml_p -> Rec_Real_ST.RecChnBitMask3 = strtol(xmlarray,NULL,16);
			 
			 elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask4",NULL, NULL, MXML_DESCEND);
			 memset(xmlarray,0,ZONE);
			 memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));	 
			 RecordXml_p -> Rec_Real_ST.RecChnBitMask4 = strtol(xmlarray,NULL,16);
			 
			 elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask5",NULL, NULL, MXML_DESCEND);
			 memset(xmlarray,0,ZONE);
			 memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));	 
			 RecordXml_p -> Rec_Real_ST.RecChnBitMask5 = strtol(xmlarray,NULL,16);
			 
			 elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask6",NULL, NULL, MXML_DESCEND);
			 memset(xmlarray,0,ZONE);
			 memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));	 
			 RecordXml_p -> Rec_Fast_ST.RecChnBitMask6 = strtol(xmlarray,NULL,16);
			 
			 elem_tmp=mxmlFindElement(tmp, xml, "RecInterval",NULL, NULL, MXML_DESCEND);
			 memset(xmlarray,0,ZONE);
			 memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));	 
			 RecordXml_p -> Rec_Real_ST.RecInterval = strtol(xmlarray,NULL,10);
			 
			 elem_tmp=mxmlFindElement(tmp, xml, "RecBeforeNum",NULL, NULL, MXML_DESCEND);
			 memset(xmlarray,0,ZONE);
			 memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));	 
			 RecordXml_p -> Rec_Real_ST.RecBeforeNum = strtol(xmlarray,NULL,10);			 
			 
			 elem_tmp=mxmlFindElement(tmp, xml, "RecAfterNum",NULL, NULL, MXML_DESCEND);
			 memset(xmlarray,0,ZONE);
			 memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));	 
			 RecordXml_p -> Rec_Real_ST.RecAfterNum = strtol(xmlarray,NULL,10);		 
			 
			 elem_tmp=mxmlFindElement(tmp, xml, "RecToTalNum",NULL, NULL, MXML_DESCEND);
			 memset(xmlarray,0,ZONE);
			 memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));	 
			 RecordXml_p -> Rec_Real_ST.RecToTalNum = strtol(xmlarray,NULL,10);
	
		}
		else if(CAN_FILE == TypeIndex)
		{
            RecordXml_p -> Rec_CAN_ST.RecTypeIdx = TypeIndex;

			elem_tmp=mxmlFindElement(tmp, xml, "RecPath",NULL, NULL, MXML_DESCEND);
			memcpy(&RecordXml_p -> Rec_CAN_ST.RecPath,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));
			elem_tmp=mxmlFindElement(tmp, xml, "RecFileHead",NULL, NULL, MXML_DESCEND);
			memcpy(&RecordXml_p -> Rec_CAN_ST.RecFileHead,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecTotalChn",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_CAN_ST.RecTotalChn_U8 = atoi(xmlarray);
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecChnNum",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_CAN_ST.RecChnNum_U8 = atoi(xmlarray);
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask1",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_CAN_ST.RecChnBitMask1 = strtol(xmlarray,NULL,16);
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask2",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_CAN_ST.RecChnBitMask2 = strtol(xmlarray,NULL,16);
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask3",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_CAN_ST.RecChnBitMask3 = strtol(xmlarray,NULL,16);
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask4",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_CAN_ST.RecChnBitMask4 = strtol(xmlarray,NULL,16);
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask5",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_CAN_ST.RecChnBitMask5 = strtol(xmlarray,NULL,16);
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask6",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_CAN_ST.RecChnBitMask6 = strtol(xmlarray,NULL,16);
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecInterval",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_CAN_ST.RecInterval = strtol(xmlarray,NULL,10);
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecBeforeNum",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_CAN_ST.RecBeforeNum = strtol(xmlarray,NULL,10);			
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecAfterNum",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_CAN_ST.RecAfterNum = strtol(xmlarray,NULL,10); 		
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecToTalNum",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_CAN_ST.RecToTalNum = strtol(xmlarray,NULL,10);
		}
		else if(UART_FILE == TypeIndex)
		{
            RecordXml_p -> Rec_Uart_ST.RecTypeIdx = TypeIndex;

			elem_tmp=mxmlFindElement(tmp, xml, "RecPath",NULL, NULL, MXML_DESCEND);
			memcpy(&RecordXml_p -> Rec_Uart_ST.RecPath,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));
			elem_tmp=mxmlFindElement(tmp, xml, "RecFileHead",NULL, NULL, MXML_DESCEND);
			memcpy(&RecordXml_p -> Rec_Uart_ST.RecFileHead,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0)));
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecTotalChn",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_Uart_ST.RecTotalChn_U8 = atoi(xmlarray);
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecChnNum",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_Uart_ST.RecChnNum_U8 = atoi(xmlarray);
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask1",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_Uart_ST.RecChnBitMask1 = strtol(xmlarray,NULL,16);
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask2",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_Uart_ST.RecChnBitMask2 = strtol(xmlarray,NULL,16);
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask3",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_Uart_ST.RecChnBitMask3 = strtol(xmlarray,NULL,16);
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask4",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_Uart_ST.RecChnBitMask4 = strtol(xmlarray,NULL,16);
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask5",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_Uart_ST.RecChnBitMask5 = strtol(xmlarray,NULL,16);
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecChnBitMask6",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_Uart_ST.RecChnBitMask6 = strtol(xmlarray,NULL,16);
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecInterval",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_Uart_ST.RecInterval = strtol(xmlarray,NULL,10);
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecBeforeNum",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_Uart_ST.RecBeforeNum = strtol(xmlarray,NULL,10);			
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecAfterNum",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_Uart_ST.RecAfterNum = strtol(xmlarray,NULL,10); 		
			
			elem_tmp=mxmlFindElement(tmp, xml, "RecToTalNum",NULL, NULL, MXML_DESCEND);
			memset(xmlarray,0,ZONE);
			memcpy(xmlarray,mxmlGetText(elem_tmp, 0),strlen(mxmlGetText(elem_tmp, 0))); 	
			RecordXml_p -> Rec_Uart_ST.RecToTalNum = strtol(xmlarray,NULL,10);
		}
		else
		{
			  elem_tmp=mxmlFindElement(tmp, xml, "RecFileHead",NULL, NULL, MXML_DESCEND);			
		}
	}
	mxmlDelete(xml);

	if(XML_DEBUG == g_DebugType_EU)
	{
		printf("Rec_Event_ST\n");
		printf("CFG xml Versionp %d\n", Versionp -> RrdCfgFile_Ver_U16);
		printf("RecChnNum_U8 %d\n", RecordXml_p -> Rec_Event_ST.RecChnNum_U8);
		printf("RecChnBitMask1 %x\n", RecordXml_p -> Rec_Event_ST.RecChnBitMask1);
		printf("RecChnBitMask2 %x\n", RecordXml_p -> Rec_Event_ST.RecChnBitMask2);
		printf("RecChnBitMask3 %x\n", RecordXml_p -> Rec_Event_ST.RecChnBitMask3);
		printf("RecChnBitMask4 %x\n", RecordXml_p -> Rec_Event_ST.RecChnBitMask4);
		printf("RecChnBitMask5 %x\n", RecordXml_p -> Rec_Event_ST.RecChnBitMask5);
		printf("RecChnBitMask6 %x\n", RecordXml_p -> Rec_Event_ST.RecChnBitMask6);
		printf("RecInterval %d\n", RecordXml_p -> Rec_Event_ST.RecInterval);
		printf("RecBeforeNum %d\n", RecordXml_p -> Rec_Event_ST.RecBeforeNum);
		printf("RecAfterNum %d\n", RecordXml_p -> Rec_Event_ST.RecAfterNum);
		printf("RecToTalNum %u\n", RecordXml_p -> Rec_Event_ST.RecToTalNum);

		printf("Rec_Fast_ST\n");
		printf("RecChnNum_U8 %d\n", RecordXml_p -> Rec_Fast_ST.RecChnNum_U8);
		printf("RecChnBitMask1 %x\n", RecordXml_p -> Rec_Fast_ST.RecChnBitMask1);
		printf("RecChnBitMask2 %x\n", RecordXml_p -> Rec_Fast_ST.RecChnBitMask2);
		printf("RecChnBitMask3 %x\n", RecordXml_p -> Rec_Fast_ST.RecChnBitMask3);
		printf("RecChnBitMask4 %x\n", RecordXml_p -> Rec_Fast_ST.RecChnBitMask4);
		printf("RecChnBitMask5 %x\n", RecordXml_p -> Rec_Fast_ST.RecChnBitMask5);
		printf("RecChnBitMask6 %x\n", RecordXml_p -> Rec_Fast_ST.RecChnBitMask6);
		printf("RecInterval %d\n", RecordXml_p -> Rec_Fast_ST.RecInterval);
		printf("RecBeforeNum %d\n", RecordXml_p -> Rec_Fast_ST.RecBeforeNum);
		printf("RecAfterNum %d\n", RecordXml_p -> Rec_Fast_ST.RecAfterNum);
		printf("RecToTalNum %u\n", RecordXml_p -> Rec_Fast_ST.RecToTalNum);

		printf("Rec_Real_ST\n");
		printf("RecChnNum_U8 %d\n", RecordXml_p -> Rec_Real_ST.RecChnNum_U8);
		printf("RecChnBitMask1 %x\n", RecordXml_p -> Rec_Real_ST.RecChnBitMask1);
		printf("RecChnBitMask2 %x\n", RecordXml_p -> Rec_Real_ST.RecChnBitMask2);
		printf("RecChnBitMask3 %x\n", RecordXml_p -> Rec_Real_ST.RecChnBitMask3);
		printf("RecChnBitMask4 %x\n", RecordXml_p -> Rec_Real_ST.RecChnBitMask4);
		printf("RecChnBitMask5 %x\n", RecordXml_p -> Rec_Real_ST.RecChnBitMask5);
		printf("RecChnBitMask6 %x\n", RecordXml_p -> Rec_Real_ST.RecChnBitMask6);
		printf("RecInterval %d\n", RecordXml_p -> Rec_Real_ST.RecInterval);
		printf("RecBeforeNum %d\n", RecordXml_p -> Rec_Real_ST.RecBeforeNum);
		printf("RecAfterNum %d\n", RecordXml_p -> Rec_Real_ST.RecAfterNum);
		printf("RecToTalNum %u\n", RecordXml_p -> Rec_Real_ST.RecToTalNum);

		printf("Rec_CAN_ST\n");
		printf("RecChnNum_U8 %d\n", RecordXml_p -> Rec_CAN_ST.RecChnNum_U8);
		printf("RecChnBitMask1 %x\n", RecordXml_p -> Rec_CAN_ST.RecChnBitMask1);
		printf("RecChnBitMask2 %x\n", RecordXml_p -> Rec_CAN_ST.RecChnBitMask2);
		printf("RecChnBitMask3 %x\n", RecordXml_p -> Rec_CAN_ST.RecChnBitMask3);
		printf("RecChnBitMask4 %x\n", RecordXml_p -> Rec_CAN_ST.RecChnBitMask4);
		printf("RecChnBitMask5 %x\n", RecordXml_p -> Rec_CAN_ST.RecChnBitMask5);
		printf("RecChnBitMask6 %x\n", RecordXml_p -> Rec_CAN_ST.RecChnBitMask6);
		printf("RecInterval %d\n", RecordXml_p -> Rec_CAN_ST.RecInterval);
		printf("RecBeforeNum %d\n", RecordXml_p -> Rec_CAN_ST.RecBeforeNum);
		printf("RecAfterNum %d\n", RecordXml_p -> Rec_CAN_ST.RecAfterNum);
		printf("RecToTalNum %u\n", RecordXml_p -> Rec_CAN_ST.RecToTalNum);

		printf("Rec_Uart_ST\n");
		printf("RecChnNum_U8 %d\n", RecordXml_p -> Rec_Uart_ST.RecChnNum_U8);
		printf("RecChnBitMask1 %x\n", RecordXml_p -> Rec_Uart_ST.RecChnBitMask1);
		printf("RecChnBitMask2 %x\n", RecordXml_p -> Rec_Uart_ST.RecChnBitMask2);
		printf("RecChnBitMask3 %x\n", RecordXml_p -> Rec_Uart_ST.RecChnBitMask3);
		printf("RecChnBitMask4 %x\n", RecordXml_p -> Rec_Uart_ST.RecChnBitMask4);
		printf("RecChnBitMask5 %x\n", RecordXml_p -> Rec_Uart_ST.RecChnBitMask5);
		printf("RecChnBitMask6 %x\n", RecordXml_p -> Rec_Uart_ST.RecChnBitMask6);
		printf("RecInterval %d\n", RecordXml_p -> Rec_Uart_ST.RecInterval);
		printf("RecBeforeNum %d\n", RecordXml_p -> Rec_Uart_ST.RecBeforeNum);
		printf("RecAfterNum %d\n", RecordXml_p -> Rec_Uart_ST.RecAfterNum);
		printf("RecToTalNum %u\n", RecordXml_p -> Rec_Uart_ST.RecToTalNum);

	}
	return CODE_OK;
}

/**********************************************************************
*Name			:   int8_t SysXmlParInit(char *xmlFilePath)
*Function       :   Init the xml parameter
*Para     		:   char *xmlFilePath 
*Return    		:   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2018/5/27  Create
*********************************************************************/
int8_t SysXmlParInit(char *xmlFilePath,RECORD_XML *RecordXml_p,VERSION *Version_p)
{
    if(XML_DEBUG == g_DebugType_EU)
	{
		printf("xml file %s \n", xmlFilePath);
	}

    RecordCFGXml(xmlFilePath,RecordXml_p,Version_p);

}

/**********************************************************************
*Name			:   int8_t SysXmlParInit(char *xmlFilePath)
*Function       :   Init the xml parameter
*Para     		:   char *xmlFilePath 
*Return    		:   int8_t 0,success;else false.
*Version        :   REV1.0.0       
*Author:        :   feng
*
*History:
*REV1.0.0     feng    2021/5/06  Create  for the xml file is not exit
*********************************************************************/
int8_t XmlParJudge(RECORD_XML *RecordXml_p)
{
   if((! RecordXml_p ->Rec_Event_ST.RecInterval) || (! RecordXml_p ->Rec_Event_ST.RecToTalNum)) //
   {
	   RecordXml_p ->Rec_Event_ST.RecInterval = XML_EVENT_REC_INTEVAL_MIN;
	   RecordXml_p ->Rec_Event_ST.RecToTalNum  = XML_EVENT_REC_TOTAL_MIN;
	   sprintf(RecordXml_p ->Rec_Event_ST.RecPath,"%s", "/yaffs/REC_EVTDATA/");
	   sprintf(RecordXml_p ->Rec_Fast_ST.RecPath,"%s", "/yaffs/REC_FASTFLT/");

   }
   if(! RecordXml_p ->Rec_Real_ST.RecToTalNum)
   {
	   RecordXml_p ->Rec_Real_ST.RecToTalNum  = XML_REAL_REC_TOTAL_MIN;
	   RecordXml_p ->Rec_Real_ST.RecBeforeNum = XML_REAL_BEFOR_NUM;
	   RecordXml_p ->Rec_Fast_ST.RecToTalNum  = XML_REAL_REC_TOTAL_MIN;
	   RecordXml_p ->Rec_Fast_ST.RecBeforeNum = XML_REAL_BEFOR_NUM;
	   sprintf(RecordXml_p ->Rec_Real_ST.RecPath,"%s", "/yaffs/REC_REALFLT/");
   }	
}

