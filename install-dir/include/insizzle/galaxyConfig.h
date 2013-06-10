#ifndef _GALAXYCONFIG
#define _GALAXYCONFIG

/* these are global to the whole galaxy */

/* galaxy
   - systems
    - contexts
     - hypercontexts
     - clusters
*/

typedef struct {
  unsigned CLUST_TEMPL_CONFIG;
  unsigned CLUST_TEMPL_STATIC_REGFILE_CONFIG;
  unsigned CLUST_TEMPL_SCORE_CONFIG;
  unsigned CLUST_TEMPL_CCORE7_CONFIG;
  unsigned CLUST_TEMPL_CCORE6_CONFIG;
  unsigned CLUST_TEMPL_CCORE5_CONFIG;
  unsigned CLUST_TEMPL_CCORE4_CONFIG;
  unsigned CLUST_TEMPL_CCORE2_CONFIG;
} clusterTemplateConfig;

typedef struct {
  unsigned hi;
  unsigned lo;
} clusterTempl;

typedef struct {
  unsigned HCONTEXT_CONFIG;
  clusterTempl HCONTEXT_CLUST_TEMPL0_1;
  clusterTempl HCONTEXT_CLUST_TEMPL_INST0_1;
} hyperContextConfig;

typedef struct {
  unsigned CONTEXT_CONFIG;
  unsigned CONTEXT_CTRL;
  unsigned IFE_SIMPLE_IRAM_PRIV_CONFIG;

  /* for each hypercontext & cluster template need to allocate memory */
  clusterTemplateConfig *CLUSTER_TEMPL;
  hyperContextConfig *HCONTEXT;
} contextConfig;


typedef struct {
  unsigned SYSTEM_CONFIG;
  unsigned PERIPH_WRAP_CONFIG;
  unsigned DRAM_SHARED_CONFIG;
  unsigned DRAM_SHARED_CTRL;
  unsigned STACK_SIZE;

  /* for each context need to allocate memory */
  contextConfig *CONTEXT;
} systemConfig;

unsigned GALAXY_CONFIG;
systemConfig *SYSTEM;

#endif
