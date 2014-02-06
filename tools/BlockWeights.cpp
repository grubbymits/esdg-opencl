void BlockWeights::CalcWeights(MachineFunction &MF) {
  MachineBlockFrequencyInfo *MBFI = &getAnalysis<MachineBlockFrequencyInfo>();
  SmallVector<uint64_t, 16> blockWeights;
  uint64_t totalWeight = 0;

  for (MachineFunction::iterator MBBI = MF.begin(), MBBE = MF.end();
       MBBI != MBBE; ++MBBI) {
    BlockFrequency BF = MBFI->getBlockFreq(MBBI);
    uint64_t score =  BF.getFrequency() * (*MBBI)->size();
    blockWeights.push_back(score);
    totalWeight += score;
  }

  for (SmallVectorImpl<uint64_t>::iterator BI = blockWeights.begin(),
       BE = blockWeights.end(); BI != BE; ++BI)

