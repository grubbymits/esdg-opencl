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

#ifdef API
#include "vtapi.h"
int readConfStatic(char *, galaxyConfigT *);
#endif
