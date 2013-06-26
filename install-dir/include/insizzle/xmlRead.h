#ifndef _XMLREAD_H_
#define _XMLREAD_H_

#include <libxml/xmlreader.h>

#include <stdio.h>
#include <string.h>

#include "galaxyConfig.h"
#include "galaxy.h"


typedef struct {
  unsigned type;
  char *name;
  char *value;
} xmlReturn;

int readConf(char *);
void processNode(xmlTextReaderPtr, xmlReturn *);

#ifdef INSIZZLEAPI
#include "vtapi.h"
int readConfStatic(char *, galaxyConfigT *);
#endif

#endif
