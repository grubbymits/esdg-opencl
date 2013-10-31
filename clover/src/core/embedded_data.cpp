#include "embedded_data.h"

using namespace Coal;

unsigned EmbeddedData::getTotalSize() {

  totalSize = 0;

  for (word_iterator WI = globalWords.begin(), WE = globalWords.end();
       WI != WE; ++WI)
    totalSize += (*WI)->getSize();

  for (half_iterator HI = globalHalves.begin(), HE = globalHalves.end();
       HI != HE; ++HI)
    totalSize += HI->getSize();

  for (byte_iterator BI = globalBytes.begin(), BE = globalBytes.end();
       BI != BE; ++BI)
    totalSize += BI->getSize();

  return totalSize;
}
