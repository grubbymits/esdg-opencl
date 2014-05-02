#include "xmlRead.h"


int readConf(char *filename) {
  systemConfig *SYS = NULL;
  contextConfig *CNT = NULL;
  hyperContextConfig *HCNT = NULL;
  clusterTemplateConfig *CLUT = NULL;

  xmlTextReaderPtr reader;
  int ret;

  xmlReturn xmlR;
  unsigned currentSystem = -1;
  unsigned currentContext = 0;
  unsigned currentClusterTemplate = 0;
  unsigned currentHyperContext = 0;
  int templ, inst;
  int homogeneous = 0;

#ifndef LIBXML_READER_ENABLED
  printf("XML support not enabled\n");
  return -1;
#endif


  reader = xmlReaderForFile(filename, NULL, 0);
  if(reader == NULL) {
    printf("Could not open file (%s)\n", filename);
    return -1;
  }
  else {
    ret = xmlTextReaderRead(reader);
    while(ret == 1) {
      processNode(reader, &xmlR);
      if(xmlR.type == 1) {
	if(!strcmp(xmlR.name, "type")) {
	  ret = xmlTextReaderRead(reader);
	  processNode(reader, &xmlR);
	  if(!strcmp(xmlR.value, "homogeneous")) {
	    printf("homogeneous system\n");
	    homogeneous = 1;
	  }
	}
	if(!strcmp(xmlR.name, "systems")) {
	  ret = xmlTextReaderRead(reader);
	  processNode(reader, &xmlR);
	  GALAXY_CONFIG = 0;
	  GALAXY_CONFIG |= atoi(xmlR.value);

	  SYSTEM = (systemConfig *)malloc(sizeof(systemConfig) * (GALAXY_CONFIG & 0xff));
	  if(SYSTEM == NULL)
	    return -1;


	}
	else if(!strcmp(xmlR.name, "system")) {
	  /* need to make sure the number of these is less than the number of systems */
	  currentSystem++;
	  if(currentSystem >= (GALAXY_CONFIG & 0xff)) {
	    printf("xml file contains more system definitions than are specified\n");
	    return -1;
	  }
	  else {
	    SYS = (systemConfig *)((size_t)SYSTEM + (currentSystem * sizeof(systemConfig)));
	    SYS->SYSTEM_CONFIG = 0;
	    SYS->PERIPH_WRAP_CONFIG = 0;
	    SYS->DRAM_SHARED_CONFIG = 0;
	    SYS->DRAM_SHARED_CTRL = 0;

	    currentContext = -1;
	  }
	}
	else if(!strcmp(xmlR.name, "contexts")) {
	  if(SYS != NULL) {
	    ret = xmlTextReaderRead(reader);
	    processNode(reader, &xmlR);
	    SYS->SYSTEM_CONFIG |= atoi(xmlR.value);

	    SYS->CONTEXT = (contextConfig *)malloc(sizeof(contextConfig) * (SYS->SYSTEM_CONFIG & 0xff));
	    if(SYS->CONTEXT == NULL)
	      return -1;
	  }
	  else {
	    printf("trying to define a context when not in a system\n");
	    return -1;
	  }
	}
	else if(!strcmp(xmlR.name, "SCALARSYS_PRESENT")) {
	  if(SYS != NULL) {
	    ret = xmlTextReaderRead(reader);
	    processNode(reader, &xmlR);
	    SYS->SYSTEM_CONFIG |= (atoi(xmlR.value) << 8);
	  }
	  else {
	    printf("trying to define SCALARSYS_PRESENT when not in a system\n");
	    return -1;
	  }
	}
	else if(!strcmp(xmlR.name, "PERIPH_PRESENT")) {
	  if(SYS != NULL) {
	    ret = xmlTextReaderRead(reader);
	    processNode(reader, &xmlR);
	    SYS->SYSTEM_CONFIG |= (atoi(xmlR.value) << 10);
	  }
	  else {
	    printf("trying to define PERIPH_PRESENT when not in a system\n");
	    return -1;
	  }
	}
	else if(!strcmp(xmlR.name, "DRAM_BLK_SIZE")) {
	  if(SYS != NULL) {
	    ret = xmlTextReaderRead(reader);
	    processNode(reader, &xmlR);
	    SYS->DRAM_SHARED_CONFIG |= atoi(xmlR.value);
	  }
	  else {
	    printf("trying to define DRAM_BLK_SIZE when not in a system\n");
	    return -1;
	  }
	}
	else if(!strcmp(xmlR.name, "DRAM_SIZE")) {
	  if(SYS != NULL) {
	    ret = xmlTextReaderRead(reader);
	    processNode(reader, &xmlR);
	    unsigned int dramSize = 0;
	    sscanf(xmlR.value, "0x%x", &dramSize);
	    SYS->DRAM_SHARED_CONFIG |= (dramSize << 8);
	  }
	  else {
	    printf("trying to define DRAM_SIZE when not in a system\n");
	    return -1;
	  }
	}
	else if(!strcmp(xmlR.name, "STACK_SIZE")) {
	  ret = xmlTextReaderRead(reader);
	  processNode(reader, &xmlR);
	  unsigned int STACK_SIZE;
	  sscanf(xmlR.value, "0x%x", &STACK_SIZE);
	  SYS->STACK_SIZE = STACK_SIZE;
	}
	else if(!strcmp(xmlR.name, "DRAM_BANKS")) {
	  if(SYS != NULL) {
	    ret = xmlTextReaderRead(reader);
	    processNode(reader, &xmlR);
	    SYS->DRAM_SHARED_CONFIG |= (atoi(xmlR.value) << 24);
	  }
	  else {
	    printf("trying to define DRAM_BANKS when not in a system\n");
	    return -1;
	  }
	}
	else if(!strcmp(xmlR.name, "DARCH")) {
	  if(SYS != NULL) {
	    ret = xmlTextReaderRead(reader);
	    processNode(reader, &xmlR);

	    if(!strcmp(xmlR.value, "DRAM_SHARED"))
	      SYS->SYSTEM_CONFIG |= (0 << 11);
	    else {
	      printf("unknown DARCH: %s\n", xmlR.value);
	      return -1;
	    }
	  }
	  else {
	    printf("trying to define DARCH when not in a system\n");
	    return -1;
	  }
	}

	else if(!strcmp(xmlR.name, "context")) {
	  /* need to make sure the number of these is less than the number of systems */
	  currentContext++;
	  if(currentContext >= (SYS->SYSTEM_CONFIG & 0xff)) {
	    printf("xml file contains more context definitions than are specified for this system (%d)\n", currentSystem);
	    return -1;
	  }
	  else {
	    CNT = (contextConfig *)((size_t)SYS->CONTEXT + (currentContext * sizeof(contextConfig)));

	    CNT->CONTEXT_CONFIG = 0;
	    CNT->CONTEXT_CTRL = 0;
	    CNT->IFE_SIMPLE_IRAM_PRIV_CONFIG = 0;

	    currentClusterTemplate = -1;
	    currentHyperContext = -1;
	  }
	}
	else if(!strcmp(xmlR.name, "ISA_PRSPCTV")) {
	  if(CNT != NULL) {
	    ret = xmlTextReaderRead(reader);
	    processNode(reader, &xmlR);

	    if(!strcmp(xmlR.value, "VT32PP"))
	      CNT->CONTEXT_CONFIG |= 0;
	    else if(!strcmp(xmlR.value, "VT64PP"))
	      CNT->CONTEXT_CONFIG |= 1;
	    else if(!strcmp(xmlR.value, "VT32EPIC"))
	      CNT->CONTEXT_CONFIG |= 2;
	    else if(!strcmp(xmlR.value, "VT64EPIC"))
	      CNT->CONTEXT_CONFIG |= 3;
	    else {
	      printf("unknown ISA_PRSPCTV: %s\n", xmlR.value);
	      return -1;
	    }
	  }
	  else {
	    printf("trying to define ISA_PRSPCTV when not in a context\n");
	    return -1;
	  }
	}
	else if(!strcmp(xmlR.name, "HYPERCONTEXTS")) {
	  if(CNT != NULL) {
	    ret = xmlTextReaderRead(reader);
	    processNode(reader, &xmlR);
	    CNT->CONTEXT_CONFIG |= (atoi(xmlR.value) << 4);

	    /* allocate memory for the hypercontexts */
	    CNT->HCONTEXT = (hyperContextConfig *)malloc(sizeof(hyperContextConfig) *((CNT->CONTEXT_CONFIG >> 4) & 0xf));
	    if(CNT->HCONTEXT == NULL)
	      return -1;
	  }
	  else {
	    printf("trying to define HYPERCONTEXTS when not in a context\n");
	    return -1;
	  }
	}
	else if(!strcmp(xmlR.name, "CLUST_TEMPL")) {
	  if(CNT != NULL) {
	    ret = xmlTextReaderRead(reader);
	    processNode(reader, &xmlR);
	    CNT->CONTEXT_CONFIG |= (atoi(xmlR.value) << 8);

	    /* allocate memory for the cluster templates */
	    CNT->CLUSTER_TEMPL = (clusterTemplateConfig *)malloc(sizeof(clusterTemplateConfig) * ((CNT->CONTEXT_CONFIG >> 8) & 0xf));
	    if(CNT->CLUSTER_TEMPL == NULL)
	      return -1;
	  }
	  else {
	    printf("trying to define CLUST_TEMPL when not in a context\n");
	    return -1;
	  }
	}
	else if(!strcmp(xmlR.name, "ISSUE_WIDTH_MAX")) {
	  if(CNT != NULL) {
	    ret = xmlTextReaderRead(reader);
	    processNode(reader, &xmlR);
	    CNT->CONTEXT_CONFIG |= (atoi(xmlR.value) << 12);
	  }
	  else {
	    printf("trying to define ISSUE_WIDTH_MAX when not in a context\n");
	    return -1;
	  }
	}
	else if(!strcmp(xmlR.name, "IARCH")) {
	  if(CNT != NULL) {
	    ret = xmlTextReaderRead(reader);
	    processNode(reader, &xmlR);

	    if(!strcmp(xmlR.value, "IFE_SIMPLE_IRAM_PRIV"))
	      CNT->CONTEXT_CONFIG |= (0 << 20);
	    else {
	      printf("unknown IARCH: %s\n", xmlR.value);
	      return -1;
	    }
	  }
	  else {
	    printf("trying to define IARCH when not in a context\n");
	    return -1;
	  }
	}
	else if(!strcmp(xmlR.name, "IFETCH_WIDTH")) {
	  if(CNT != NULL) {
	    ret = xmlTextReaderRead(reader);
	    processNode(reader, &xmlR);
	    CNT->IFE_SIMPLE_IRAM_PRIV_CONFIG |= atoi(xmlR.value);
	  }
	  else {
	    printf("trying to define IFETCH_WIDTH when not in a context\n");
	    return -1;
	  }
	}
	else if(!strcmp(xmlR.name, "IRAM_SIZE")) {
	  if(CNT != NULL) {
	    ret = xmlTextReaderRead(reader);
	    processNode(reader, &xmlR);
	    unsigned int iramSize = 0;
	    sscanf(xmlR.value, "0x%x", &iramSize);
	    CNT->IFE_SIMPLE_IRAM_PRIV_CONFIG |= (iramSize << 8);
	  }
	  else {
	    printf("trying to define IRAM_SIZE when not in a context\n");
	    return -1;
	  }
	}

	else if(!strcmp(xmlR.name, "clusterTemplate")) {
	  /* need to make sure the number of these is less than the number of systems */
	  currentClusterTemplate++;
	  if(currentClusterTemplate >= ((CNT->CONTEXT_CONFIG >> 8) & 0xf)) {
	    printf("xml file contains more clusterTemplate definitions than are specified for this context (%d)\n", currentContext);
	    return -1;
	  }
	  else {
	    CLUT = (clusterTemplateConfig *)((size_t)CNT->CLUSTER_TEMPL + (currentClusterTemplate * sizeof(clusterTemplateConfig)));

	    CLUT->CLUST_TEMPL_CONFIG = 0;
	    CLUT->CLUST_TEMPL_STATIC_REGFILE_CONFIG = 0;
	    CLUT->CLUST_TEMPL_SCORE_CONFIG = 0;
	  }
	}
	else if(!strcmp(xmlR.name, "SCORE_PRESENT")) {
	  if(CLUT != NULL) {
	    ret = xmlTextReaderRead(reader);
	    processNode(reader, &xmlR);
	    CLUT->CLUST_TEMPL_CONFIG |= atoi(xmlR.value);
	  }
	  else {
	    printf("trying to define SCORE_PRESENT when not in a clusterTemplate\n");
	    return -1;
	  }
	}
	else if(!strcmp(xmlR.name, "VCORE_PRESENT")) {
	  if(CLUT != NULL) {
	    ret = xmlTextReaderRead(reader);
	    processNode(reader, &xmlR);
	    CLUT->CLUST_TEMPL_CONFIG |= (atoi(xmlR.value) << 1);
	  }
	  else {
	    printf("trying to define VCORE_PRESENT when not in a clusterTemplate\n");
	    return -1;
	  }
	}
	else if(!strcmp(xmlR.name, "FPCORE_PRESENT")) {
	  if(CLUT != NULL) {
	    ret = xmlTextReaderRead(reader);
	    processNode(reader, &xmlR);
	    CLUT->CLUST_TEMPL_CONFIG |= (atoi(xmlR.value) << 2);
	  }
	  else {
	    printf("trying to define FPCORE_PRESENT when not in a clusterTemplate\n");
	    return -1;
	  }
	}
	else if(!strcmp(xmlR.name, "CCORE_PRESENT")) {
	  if(CLUT != NULL) {
	    ret = xmlTextReaderRead(reader);
	    processNode(reader, &xmlR);
	    CLUT->CLUST_TEMPL_CONFIG |= (atoi(xmlR.value) << 3);
	  }
	  else {
	    printf("trying to define CCORE_PRESENT when not in a clusterTemplate\n");
	    return -1;
	  }
	}
	else if(!strcmp(xmlR.name, "INSTANCES")) {
	  if(CLUT != NULL) {
	    ret = xmlTextReaderRead(reader);
	    processNode(reader, &xmlR);
	    CLUT->CLUST_TEMPL_CONFIG |= (atoi(xmlR.value) << 17);
	  }
	  else {
	    printf("trying to define INSTANCES when not in a clusterTemplate\n");
	    return -1;
	  }
	}
	else if(!strcmp(xmlR.name, "INSTANTIATE")) {
	  if(CLUT != NULL) {
	    ret = xmlTextReaderRead(reader);
	    processNode(reader, &xmlR);
	    CLUT->CLUST_TEMPL_CONFIG |= (atoi(xmlR.value) << 16);
	  }
	  else {
	    printf("trying to define INSTANTIATE when not in a clusterTemplate\n");
	    return -1;
	  }
	}
	else if(!strcmp(xmlR.name, "ISSUE_WIDTH")) {
	  if(CLUT != NULL) {
	    ret = xmlTextReaderRead(reader);
	    processNode(reader, &xmlR);
	    CLUT->CLUST_TEMPL_CONFIG |= (atoi(xmlR.value) << 8);
	  }
	  else {
	    printf("trying to define ISSUE_WIDTH when not in a clusterTemplate\n");
	    return -1;
	  }
	}
	else if(!strcmp(xmlR.name, "S_GPR_FILE_SIZE")) {
	  if(CLUT != NULL) {
	    ret = xmlTextReaderRead(reader);
	    processNode(reader, &xmlR);
	    CLUT->CLUST_TEMPL_STATIC_REGFILE_CONFIG |= atoi(xmlR.value);
	  }
	  else {
	    printf("trying to define S_GPR_FILE_SIZE when not in a clusterTemplate\n");
	    return -1;
	  }
	}
	else if(!strcmp(xmlR.name, "S_FPR_FILE_SIZE")) {
	  if(CLUT != NULL) {
	    ret = xmlTextReaderRead(reader);
	    processNode(reader, &xmlR);
	    CLUT->CLUST_TEMPL_STATIC_REGFILE_CONFIG |= (atoi(xmlR.value) << 8);
	  }
	  else {
	    printf("trying to define S_FPR_FILE_SIZE when not in a clusterTemplate\n");
	    return -1;
	  }
	}
	else if(!strcmp(xmlR.name, "S_VR_FILE_SIZE")) {
	  if(CLUT != NULL) {
	    ret = xmlTextReaderRead(reader);
	    processNode(reader, &xmlR);
	    CLUT->CLUST_TEMPL_STATIC_REGFILE_CONFIG |= (atoi(xmlR.value) << 16);
	  }
	  else {
	    printf("trying to define S_VR_FILE_SIZE when not in a clusterTemplate\n");
	    return -1;
	  }
	}
	else if(!strcmp(xmlR.name, "S_PR_FILE_SIZE")) {
	  if(CLUT != NULL) {
	    ret = xmlTextReaderRead(reader);
	    processNode(reader, &xmlR);
	    CLUT->CLUST_TEMPL_STATIC_REGFILE_CONFIG |= (atoi(xmlR.value) << 24);
	  }
	  else {
	    printf("trying to define S_PR_FILE_SIZE when not in a clusterTemplate\n");
	    return -1;
	  }
	}
	else if(!strcmp(xmlR.name, "IALUS")) {
	  if(CLUT != NULL) {
	    ret = xmlTextReaderRead(reader);
	    processNode(reader, &xmlR);
	    CLUT->CLUST_TEMPL_SCORE_CONFIG |= atoi(xmlR.value);
	  }
	  else {
	    printf("trying to define IALUS when not in a clusterTemplate\n");
	    return -1;
	  }
	}
	else if(!strcmp(xmlR.name, "IMULTS")) {
	  if(CLUT != NULL) {
	    ret = xmlTextReaderRead(reader);
	    processNode(reader, &xmlR);
	    CLUT->CLUST_TEMPL_SCORE_CONFIG |= (atoi(xmlR.value) << 8);
	  }
	  else {
	    printf("trying to define IMULTS when not in a clusterTemplate\n");
	    return -1;
	  }
	}
	else if(!strcmp(xmlR.name, "LSU_CHANNELS")) {
	  if(CLUT != NULL) {
	    ret = xmlTextReaderRead(reader);
	    processNode(reader, &xmlR);
	    CLUT->CLUST_TEMPL_SCORE_CONFIG |= (atoi(xmlR.value) << 16);
	  }
	  else {
	    printf("trying to define LSU_CHANNELS when not in a clusterTemplate\n");
	    return -1;
	  }
	}
	else if(!strcmp(xmlR.name, "BRUS")) {
	  if(CLUT != NULL) {
	    ret = xmlTextReaderRead(reader);
	    processNode(reader, &xmlR);
	    CLUT->CLUST_TEMPL_SCORE_CONFIG |= (atoi(xmlR.value) << 24);
	  }
	  else {
	    printf("trying to define BRUS when not in a clusterTemplate\n");
	    return -1;
	  }
	}

	else if(!strcmp(xmlR.name, "hypercontext")) {
	  /* need to make sure the number of these is less than the number of systems */
	  currentHyperContext++;
	  if(currentHyperContext >= ((CNT->CONTEXT_CONFIG >> 4) & 0xf)) {
	    printf("xml file contains more hyperContext definitions than are specified for this context (%d)\n", currentContext);
	    return -1;
	  }
	  else {
	    HCNT = (hyperContextConfig *)((size_t)CNT->HCONTEXT + (currentHyperContext * sizeof(hyperContextConfig)));

	    HCNT->HCONTEXT_CONFIG = 0;
	    HCNT->HCONTEXT_CLUST_TEMPL0_1.hi = 0;
	    HCNT->HCONTEXT_CLUST_TEMPL0_1.lo = 0;

	    HCNT->HCONTEXT_CLUST_TEMPL_INST0_1.hi = 0;
	    HCNT->HCONTEXT_CLUST_TEMPL_INST0_1.lo = 0;
	  }
	}
	else if(!strcmp(xmlR.name, "cluster")) {
	  if(HCNT != NULL) {
	    ret = xmlTextReaderRead(reader);
	    processNode(reader, &xmlR);
	    sscanf(xmlR.value, "%d_%d", &templ, &inst);

	    if((HCNT->HCONTEXT_CONFIG & 0xf) > 8) {
	      HCNT->HCONTEXT_CLUST_TEMPL0_1.hi |= (templ << (4 * (HCNT->HCONTEXT_CONFIG & 0xf)));
	      HCNT->HCONTEXT_CLUST_TEMPL_INST0_1.hi |= (inst << (4 * (HCNT->HCONTEXT_CONFIG & 0xf)));
	    }
	    else {
	      HCNT->HCONTEXT_CLUST_TEMPL0_1.lo |= (templ << (4 * (HCNT->HCONTEXT_CONFIG & 0xf)));
	      HCNT->HCONTEXT_CLUST_TEMPL_INST0_1.lo |= (inst << (4 * (HCNT->HCONTEXT_CONFIG & 0xf)));
	    }

	    HCNT->HCONTEXT_CONFIG += 1;
	  }
	  else {
	    printf("trying to define cluster when not in a hyperContext\n");
	    return -1;
	  }
	}

      }
      ret = xmlTextReaderRead(reader);
    }
    xmlFreeTextReader(reader);
    if(ret != 0) {
      printf("unable to parse file (%s)\n", filename);
      return -1;
    }
  }
  if(homogeneous) {
    unsigned int i, j, k;
  
    systemConfig *SYS;
    contextConfig *CNT;
    contextConfig *CNT_homogeneous;
    hyperContextConfig *HCNT;
    hyperContextConfig *HCNT_homogeneous;
    clusterTemplateConfig *CLUT;
    clusterTemplateConfig *CLUT_homogeneous;

    /* need to copy everything from context 0 to all available context */
    /* loop through each available hypercontext */
    for(i=0;i<(GALAXY_CONFIG & 0xff);i++)
      {
	SYS = (systemConfig *)((size_t)SYSTEM + (i * sizeof(systemConfig)));
	
	if((SYS->SYSTEM_CONFIG & 0xff) > 1) {
	  CNT = (contextConfig *)((size_t)SYS->CONTEXT + (0 * sizeof(contextConfig)));

	  /* need to copy context 0 to all of these */
	  for(j=1;j<(SYS->SYSTEM_CONFIG & 0xff);j++) {
	    CNT_homogeneous = (contextConfig *)((size_t)SYS->CONTEXT + (j * sizeof(contextConfig)));
	    CNT_homogeneous->CONTEXT_CONFIG = CNT->CONTEXT_CONFIG;
	    CNT_homogeneous->CONTEXT_CTRL = CNT->CONTEXT_CTRL = 0;
	    CNT_homogeneous->IFE_SIMPLE_IRAM_PRIV_CONFIG = CNT->IFE_SIMPLE_IRAM_PRIV_CONFIG;

	    /* setup hypercontext */
	    CNT_homogeneous->HCONTEXT = (hyperContextConfig *)malloc(sizeof(hyperContextConfig) *((CNT->CONTEXT_CONFIG >> 4) & 0xf));
	    if(CNT_homogeneous->HCONTEXT == NULL)
	      return -1;

	    /* loop through hypercontexts and copy info */
	    for(k=0;k<((CNT->CONTEXT_CONFIG >> 4) & 0xf);k++) {
	      HCNT = (hyperContextConfig *)((size_t)CNT->HCONTEXT + (k * sizeof(hyperContextConfig)));
	      HCNT_homogeneous = (hyperContextConfig *)((size_t)CNT_homogeneous->HCONTEXT + (k * sizeof(hyperContextConfig)));

	      HCNT_homogeneous->HCONTEXT_CONFIG = HCNT->HCONTEXT_CONFIG;
	      HCNT_homogeneous->HCONTEXT_CLUST_TEMPL0_1.hi = HCNT->HCONTEXT_CLUST_TEMPL0_1.hi;
	      HCNT_homogeneous->HCONTEXT_CLUST_TEMPL0_1.lo = HCNT->HCONTEXT_CLUST_TEMPL0_1.lo;

	      HCNT_homogeneous->HCONTEXT_CLUST_TEMPL_INST0_1.hi = HCNT->HCONTEXT_CLUST_TEMPL_INST0_1.hi;
	      HCNT_homogeneous->HCONTEXT_CLUST_TEMPL_INST0_1.lo = HCNT->HCONTEXT_CLUST_TEMPL_INST0_1.lo;
	    }

	    /* setup clusterTemplates */
	    CNT_homogeneous->CLUSTER_TEMPL = (clusterTemplateConfig *)malloc(sizeof(clusterTemplateConfig) * ((CNT_homogeneous->CONTEXT_CONFIG >> 8) & 0xf));
	    if(CNT_homogeneous->CLUSTER_TEMPL == NULL)
	      return -1;
	    /* loop through clusterTemplates and copy info */
	    for(k=0;k<((CNT->CONTEXT_CONFIG >> 8) & 0xf);k++) {
	      CLUT = (clusterTemplateConfig *)((size_t)CNT->CLUSTER_TEMPL + (k * sizeof(clusterTemplateConfig)));
	      CLUT_homogeneous = (clusterTemplateConfig *)((size_t)CNT_homogeneous->CLUSTER_TEMPL + (k * sizeof(clusterTemplateConfig)));

	      CLUT_homogeneous->CLUST_TEMPL_CONFIG = CLUT->CLUST_TEMPL_CONFIG;
	      CLUT_homogeneous->CLUST_TEMPL_STATIC_REGFILE_CONFIG = CLUT->CLUST_TEMPL_STATIC_REGFILE_CONFIG;
	      CLUT_homogeneous->CLUST_TEMPL_SCORE_CONFIG = CLUT->CLUST_TEMPL_SCORE_CONFIG;
	    }
	  }
	}
      }
  }
  return 0;
}

void processNode(xmlTextReaderPtr reader, xmlReturn *xmlR) {
  const xmlChar *name, *value;

  name = xmlTextReaderConstName(reader);
  if (name == NULL)
    name = BAD_CAST "--";

  value = xmlTextReaderConstValue(reader);

  xmlR->type = xmlTextReaderNodeType(reader);
  xmlR->name = (char *)name;
  xmlR->value = (char *)value;
}

#ifdef INSIZZLEAPI
int readConfStatic(char *filename, galaxyConfigT *galaxyConfig) {
  xmlTextReaderPtr reader;
  int ret;

  xmlReturn xmlR;
  unsigned currentSystem = -1;
  unsigned currentContext = 0;
  unsigned currentClusterTemplate = 0;
  unsigned currentHyperContext = 0;
  int templ, inst;

#ifndef LIBXML_READER_ENABLED
  printf("XML support not enabled\n");
  return -1;
#endif


  reader = xmlReaderForFile(filename, NULL, 0);
  if(reader == NULL) {
    printf("Could not open file (%s)\n", filename);
    return -1;
  }
  else {
    ret = xmlTextReaderRead(reader);
    while(ret == 1) {
      processNode(reader, &xmlR);
      if(xmlR.type == 1) {
	if(!strcmp(xmlR.name, "systems")) {
	  ret = xmlTextReaderRead(reader);
	  processNode(reader, &xmlR);

	  galaxyConfig->ctrlState.GALAXY_CONFIG = 0;
	  galaxyConfig->ctrlState.GALAXY_CONFIG |= atoi(xmlR.value);

	  if(galaxyConfig->ctrlState.GALAXY_CONFIG > MASTERCFG_SYSTEMS_MAX) {
	    printf("You have specified a number of SYSTEMS greater than the MAX\n");
	    return -1;
	  }
	}
	else if(!strcmp(xmlR.name, "system")) {
	  /* need to make sure the number of these is less than the number of systems */
	  currentSystem++;
	  if(currentSystem >= (galaxyConfig->ctrlState.GALAXY_CONFIG & 0xff)) {
	    printf("xml file contains more system definitions than are specified\n");
	    return -1;
	  }
	  else {
	    /*printf("currentSystem: %d\n", currentSystem);*/
	    /* TODO: this doesn't seem to be here? */
	    /* SYS->DRAM_SHARED_CTRL = 0;*/
	    galaxyConfig->ctrlState.SYSTEM_CONFIG[currentSystem] = 0;
	    galaxyConfig->ctrlState.PERIPH_WRAP_CONFIG[currentSystem] = 0;
	    galaxyConfig->ctrlState.DRAM_SHARED_CONFIG0[currentSystem] = 0;

	    currentContext = -1;
	  }
	}
	else if(!strcmp(xmlR.name, "contexts")) {
	  ret = xmlTextReaderRead(reader);
	  processNode(reader, &xmlR);
	  galaxyConfig->ctrlState.SYSTEM_CONFIG[currentSystem] |= atoi(xmlR.value);
	}
	else if(!strcmp(xmlR.name, "SCALARSYS_PRESENT")) {
	  ret = xmlTextReaderRead(reader);
	  processNode(reader, &xmlR);
	  galaxyConfig->ctrlState.SYSTEM_CONFIG[currentSystem] |= (atoi(xmlR.value) << 8);
	}
	else if(!strcmp(xmlR.name, "PERIPH_PRESENT")) {
	  ret = xmlTextReaderRead(reader);
	  processNode(reader, &xmlR);
	  galaxyConfig->ctrlState.SYSTEM_CONFIG[currentSystem] |= (atoi(xmlR.value) << 10);
	}
	else if(!strcmp(xmlR.name, "DRAM_BLK_SIZE")) {
	  ret = xmlTextReaderRead(reader);
	  processNode(reader, &xmlR);
	  galaxyConfig->ctrlState.DRAM_SHARED_CONFIG0[currentSystem] |= atoi(xmlR.value);
	}
	else if(!strcmp(xmlR.name, "DRAM_SIZE")) {
	  ret = xmlTextReaderRead(reader);
	  processNode(reader, &xmlR);
	  galaxyConfig->ctrlState.DRAM_SHARED_CONFIG0[currentSystem] |= (atoi(xmlR.value) << 8);
	}
	else if(!strcmp(xmlR.name, "DRAM_BANKS")) {
	  ret = xmlTextReaderRead(reader);
	  processNode(reader, &xmlR);
	  galaxyConfig->ctrlState.DRAM_SHARED_CONFIG0[currentSystem] |= (atoi(xmlR.value) << 24);
	}
	else if(!strcmp(xmlR.name, "DARCH")) {
	  ret = xmlTextReaderRead(reader);
	  processNode(reader, &xmlR);

	  /* TODO: enum type maybe ?*/
	  if(!strcmp(xmlR.value, "DRAM_SHARED"))
	    galaxyConfig->ctrlState.SYSTEM_CONFIG[currentSystem] |= (0 << 11);
	  else {
	    printf("unknown DARCH: %s\n", xmlR.value);
	    return -1;
	  }
	}

	else if(!strcmp(xmlR.name, "context")) {
	  /* need to make sure the number of these is less than the number of systems */
	  currentContext++;
	  if(currentContext >= (galaxyConfig->ctrlState.SYSTEM_CONFIG[currentSystem] & 0xff)) {
	    printf("xml file contains more context definitions than are specified for this system (%d)\n", currentSystem);
	    return -1;
	  }
	  else {

	    /* TODO: unknown */
	    /* CNT->CONTEXT_CTRL_REG = 0;*/

	    galaxyConfig->ctrlState.CONTEXT_CONFIG[currentSystem][currentContext] = 0;
	    galaxyConfig->ctrlState.IFE_SIMPLE_IRAM_PRIV_CONFIG0[currentSystem][currentContext] = 0;

	    currentClusterTemplate = -1;
	    currentHyperContext = -1;
	  }
	}
	else if(!strcmp(xmlR.name, "ISA_PRSPCTV")) {
	  ret = xmlTextReaderRead(reader);
	  processNode(reader, &xmlR);

	  if(!strcmp(xmlR.value, "VT32PP"))
	    galaxyConfig->ctrlState.CONTEXT_CONFIG[currentSystem][currentContext] |= VT32PP;
	  else if(!strcmp(xmlR.value, "VT64PP"))
	    galaxyConfig->ctrlState.CONTEXT_CONFIG[currentSystem][currentContext] |= VT64PP;
	  else if(!strcmp(xmlR.value, "VT32EPIC"))
	    galaxyConfig->ctrlState.CONTEXT_CONFIG[currentSystem][currentContext] |= VT32EPIC;
	  else if(!strcmp(xmlR.value, "VT64EPIC"))
	    galaxyConfig->ctrlState.CONTEXT_CONFIG[currentSystem][currentContext] |= VT64EPIC;
	  else {
	    printf("unknown ISA_PRSPCTV: %s\n", xmlR.value);
	    return -1;
	  }
	}
	else if(!strcmp(xmlR.name, "HYPERCONTEXTS")) {
	  ret = xmlTextReaderRead(reader);
	  processNode(reader, &xmlR);
	  galaxyConfig->ctrlState.CONTEXT_CONFIG[currentSystem][currentContext] |= (atoi(xmlR.value) << 4);
	}
	else if(!strcmp(xmlR.name, "CLUST_TEMPL")) {
	  ret = xmlTextReaderRead(reader);
	  processNode(reader, &xmlR);
	  galaxyConfig->ctrlState.CONTEXT_CONFIG[currentSystem][currentContext] |= (atoi(xmlR.value) << 8);
	}
	else if(!strcmp(xmlR.name, "ISSUE_WIDTH_MAX")) {
	  ret = xmlTextReaderRead(reader);
	  processNode(reader, &xmlR);
	  galaxyConfig->ctrlState.CONTEXT_CONFIG[currentSystem][currentContext] |= (atoi(xmlR.value) << 12);
	}
	else if(!strcmp(xmlR.name, "IARCH")) {
	  ret = xmlTextReaderRead(reader);
	  processNode(reader, &xmlR);

	  /* TODO: enum? */
	  if(!strcmp(xmlR.value, "IFE_SIMPLE_IRAM_PRIV"))
	    galaxyConfig->ctrlState.CONTEXT_CONFIG[currentSystem][currentContext] |= (0 << 20);
	  else {
	    printf("unknown IARCH: %s\n", xmlR.value);
	    return -1;
	  }
	}
	else if(!strcmp(xmlR.name, "IFETCH_WIDTH")) {
	  ret = xmlTextReaderRead(reader);
	  processNode(reader, &xmlR);
	  galaxyConfig->ctrlState.IFE_SIMPLE_IRAM_PRIV_CONFIG0[currentSystem][currentContext] |= atoi(xmlR.value);
	}
	else if(!strcmp(xmlR.name, "IRAM_SIZE")) {
	  ret = xmlTextReaderRead(reader);
	  processNode(reader, &xmlR);
	  galaxyConfig->ctrlState.IFE_SIMPLE_IRAM_PRIV_CONFIG0[currentSystem][currentContext] |= (atoi(xmlR.value) << 8);
	}
	else if(!strcmp(xmlR.name, "clusterTemplate")) {
	  /* need to make sure the number of these is less than the number of systems */
	  currentClusterTemplate++;
	  if(currentClusterTemplate >= ((galaxyConfig->ctrlState.CONTEXT_CONFIG[currentSystem][currentContext] >> 8) & 0xf)) {
	    printf("xml file contains more clusterTemplate definitions than are specified for this context (%d)\n", currentContext);
	    return -1;
	  }
	  else {
	    galaxyConfig->ctrlState.CLUST_TEMPL_CONFIG[currentSystem][currentContext][currentClusterTemplate] = 0;
	    galaxyConfig->ctrlState.CLUST_TEMPL_STATIC_REGFILE_CONFIG[currentSystem][currentContext][currentClusterTemplate] = 0;
	    galaxyConfig->ctrlState.CLUST_TEMPL_SCORE_CONFIG[currentSystem][currentContext][currentClusterTemplate] = 0;
	  }
	}
	else if(!strcmp(xmlR.name, "SCORE_PRESENT")) {
	  ret = xmlTextReaderRead(reader);
	  processNode(reader, &xmlR);
	  galaxyConfig->ctrlState.CLUST_TEMPL_CONFIG[currentSystem][currentContext][currentClusterTemplate] |= atoi(xmlR.value);
	}
	else if(!strcmp(xmlR.name, "VCORE_PRESENT")) {
	  ret = xmlTextReaderRead(reader);
	  processNode(reader, &xmlR);
	  galaxyConfig->ctrlState.CLUST_TEMPL_CONFIG[currentSystem][currentContext][currentClusterTemplate] |= (atoi(xmlR.value) << 1);
	}
	else if(!strcmp(xmlR.name, "FPCORE_PRESENT")) {
	  ret = xmlTextReaderRead(reader);
	  processNode(reader, &xmlR);
	  galaxyConfig->ctrlState.CLUST_TEMPL_CONFIG[currentSystem][currentContext][currentClusterTemplate] |= (atoi(xmlR.value) << 2);
	}
	else if(!strcmp(xmlR.name, "CCORE_PRESENT")) {
	  ret = xmlTextReaderRead(reader);
	  processNode(reader, &xmlR);
	  galaxyConfig->ctrlState.CLUST_TEMPL_CONFIG[currentSystem][currentContext][currentClusterTemplate] |= (atoi(xmlR.value) << 3);
	}
	else if(!strcmp(xmlR.name, "INSTANCES")) {
	  ret = xmlTextReaderRead(reader);
	  processNode(reader, &xmlR);
	  galaxyConfig->ctrlState.CLUST_TEMPL_CONFIG[currentSystem][currentContext][currentClusterTemplate] |= (atoi(xmlR.value) << 17);
	}
	else if(!strcmp(xmlR.name, "INSTANTIATE")) {
	  ret = xmlTextReaderRead(reader);
	  processNode(reader, &xmlR);
	  galaxyConfig->ctrlState.CLUST_TEMPL_CONFIG[currentSystem][currentContext][currentClusterTemplate] |= (atoi(xmlR.value) << 16);
	}
	else if(!strcmp(xmlR.name, "ISSUE_WIDTH")) {
	  ret = xmlTextReaderRead(reader);
	  processNode(reader, &xmlR);
	  galaxyConfig->ctrlState.CLUST_TEMPL_CONFIG[currentSystem][currentContext][currentClusterTemplate] |= (atoi(xmlR.value) << 8);
	}
	else if(!strcmp(xmlR.name, "S_GPR_FILE_SIZE")) {
	  ret = xmlTextReaderRead(reader);
	  processNode(reader, &xmlR);
	  galaxyConfig->ctrlState.CLUST_TEMPL_STATIC_REGFILE_CONFIG[currentSystem][currentContext][currentClusterTemplate] |= atoi(xmlR.value);
	}
	else if(!strcmp(xmlR.name, "S_FPR_FILE_SIZE")) {
	  ret = xmlTextReaderRead(reader);
	  processNode(reader, &xmlR);
	  galaxyConfig->ctrlState.CLUST_TEMPL_STATIC_REGFILE_CONFIG[currentSystem][currentContext][currentClusterTemplate] |= (atoi(xmlR.value) << 8);
	}
	else if(!strcmp(xmlR.name, "S_VR_FILE_SIZE")) {
	  ret = xmlTextReaderRead(reader);
	  processNode(reader, &xmlR);
	  galaxyConfig->ctrlState.CLUST_TEMPL_STATIC_REGFILE_CONFIG[currentSystem][currentContext][currentClusterTemplate] |= (atoi(xmlR.value) << 16);
	}
	else if(!strcmp(xmlR.name, "S_PR_FILE_SIZE")) {
	  ret = xmlTextReaderRead(reader);
	  processNode(reader, &xmlR);
	  galaxyConfig->ctrlState.CLUST_TEMPL_STATIC_REGFILE_CONFIG[currentSystem][currentContext][currentClusterTemplate] |= (atoi(xmlR.value) << 24);
	}
	else if(!strcmp(xmlR.name, "IALUS")) {
	  ret = xmlTextReaderRead(reader);
	  processNode(reader, &xmlR);
	  galaxyConfig->ctrlState.CLUST_TEMPL_SCORE_CONFIG[currentSystem][currentContext][currentClusterTemplate] |= atoi(xmlR.value);
	}
	else if(!strcmp(xmlR.name, "IMULTS")) {
	  ret = xmlTextReaderRead(reader);
	  processNode(reader, &xmlR);
	  galaxyConfig->ctrlState.CLUST_TEMPL_SCORE_CONFIG[currentSystem][currentContext][currentClusterTemplate] |= (atoi(xmlR.value) << 8);
	}
	else if(!strcmp(xmlR.name, "LSU_CHANNELS")) {
	  ret = xmlTextReaderRead(reader);
	  processNode(reader, &xmlR);
	  galaxyConfig->ctrlState.CLUST_TEMPL_SCORE_CONFIG[currentSystem][currentContext][currentClusterTemplate] |= (atoi(xmlR.value) << 16);
	}
	else if(!strcmp(xmlR.name, "BRUS")) {
	  ret = xmlTextReaderRead(reader);
	  processNode(reader, &xmlR);
	  galaxyConfig->ctrlState.CLUST_TEMPL_SCORE_CONFIG[currentSystem][currentContext][currentClusterTemplate] |= (atoi(xmlR.value) << 24);
	}

	else if(!strcmp(xmlR.name, "hypercontext")) {
	  /* need to make sure the number of these is less than the number of systems */
	  currentHyperContext++;
	  if(currentHyperContext >= ((galaxyConfig->ctrlState.CONTEXT_CONFIG[currentSystem][currentContext] >> 4) & 0xf)) {
	    printf("xml file contains more hyperContext definitions than are specified for this context (%d)\n", currentContext);
	    return -1;
	  }
	  else {
	    galaxyConfig->ctrlState.HCONTEXT_CONFIG[currentSystem][currentContext][currentHyperContext] = 0;
	    galaxyConfig->ctrlState.HCONTEXT_CLUST_TEMPL0[currentSystem][currentContext][currentHyperContext] = 0;
	    galaxyConfig->ctrlState.HCONTEXT_CLUST_TEMPL1[currentSystem][currentContext][currentHyperContext] = 0;
	    galaxyConfig->ctrlState.HCONTEXT_CLUST_TEMPL_INST0[currentSystem][currentContext][currentHyperContext] = 0;
	    galaxyConfig->ctrlState.HCONTEXT_CLUST_TEMPL_INST1[currentSystem][currentContext][currentHyperContext] = 0;
	  }
	}
	else if(!strcmp(xmlR.name, "cluster")) {
	  ret = xmlTextReaderRead(reader);
	  processNode(reader, &xmlR);
	  sscanf(xmlR.value, "%d_%d", &templ, &inst);

	  /* TODO: check hi and lo */
	  if((galaxyConfig->ctrlState.HCONTEXT_CONFIG[currentSystem][currentContext][currentHyperContext] & 0xf) > 8) {
	    galaxyConfig->ctrlState.HCONTEXT_CLUST_TEMPL1[currentSystem][currentContext][currentHyperContext]|= (templ << (4 * (galaxyConfig->ctrlState.HCONTEXT_CONFIG[currentSystem][currentContext][currentHyperContext] & 0xf)));
	    galaxyConfig->ctrlState.HCONTEXT_CLUST_TEMPL_INST1[currentSystem][currentContext][currentHyperContext] |= (inst << (4 * (galaxyConfig->ctrlState.HCONTEXT_CONFIG[currentSystem][currentContext][currentHyperContext] & 0xf)));
	  }
	  else {
	    galaxyConfig->ctrlState.HCONTEXT_CLUST_TEMPL0[currentSystem][currentContext][currentHyperContext] |= (templ << (4 * (galaxyConfig->ctrlState.HCONTEXT_CONFIG[currentSystem][currentContext][currentHyperContext] & 0xf)));
	    galaxyConfig->ctrlState.HCONTEXT_CLUST_TEMPL_INST0[currentSystem][currentContext][currentHyperContext] |= (inst << (4 * (galaxyConfig->ctrlState.HCONTEXT_CONFIG[currentSystem][currentContext][currentHyperContext] & 0xf)));
	  }
	  galaxyConfig->ctrlState.HCONTEXT_CONFIG[currentSystem][currentContext][currentHyperContext] += 1;
	}
      }
      ret = xmlTextReaderRead(reader);
    }
    xmlFreeTextReader(reader);
    if(ret != 0) {
      printf("unable to parse file (%s)\n", filename);
      return -1;
    }
  }
  return 0;
}
#endif
