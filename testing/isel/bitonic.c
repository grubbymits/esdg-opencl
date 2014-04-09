
typedef unsigned int uint;
unsigned get_group_id(unsigned);

void bitonicSort(uint * theArray,
                 const uint stage, 
                 const uint passOfStage,
                 const uint direction)
{
  unsigned __esdg_idx = 0;
for (__esdg_idx = 0; __esdg_idx < 256; ++__esdg_idx) {
    uint sortIncreasing = direction;
    uint threadId = get_group_id(0) * 256 + __esdg_idx;//get_global_id(0);
    
    uint pairDistance = 1 << (stage - passOfStage);
    uint blockWidth   = 2 * pairDistance;

    uint leftId = (threadId % pairDistance) 
                   + (threadId / pairDistance) * blockWidth;

    uint rightId = leftId + pairDistance;
    
    uint leftElement = theArray[leftId];
    uint rightElement = theArray[rightId];
    
    uint sameDirectionBlockWidth = 1 << stage;
    
    if((threadId/sameDirectionBlockWidth) % 2 == 1)
        sortIncreasing = 1 - sortIncreasing;

    uint greater;
    uint lesser;
    if(leftElement > rightElement)
    {
        greater = leftElement;
        lesser  = rightElement;
    }
    else
    {
        greater = rightElement;
        lesser  = leftElement;
    }
    
    if(sortIncreasing)
    {
        theArray[leftId]  = lesser;
        theArray[rightId] = greater;
    }
    else
    {
        theArray[leftId]  = greater;
        theArray[rightId] = lesser;
    }

__ESDG_END: ;
} __esdg_idx = 0;
}

