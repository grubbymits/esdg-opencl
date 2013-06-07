// 4 hypercontexts per context
__local char[4] ready = {0};
__local char[4] finished = {0};

void local_barrier() {

  unsigned id = cpuid();
  ready[id] = 0xf;

  // wait for all hyper contexts to enter the barrier
  while(*(*unsigned int) ready != 0xffff);

  // write a byte that is only read by the others
  finished[id] = 0xf;

  // wait until each hyper context has finished, read all bytes.
  while (*(unsigned int*) finished != 0xffff);

  ready[id] = 0;

  // wait for the hyper contexts to have known that everyone has finished
  while(*(unsigned int) ready != 0);

  // reset finished so it will ready for the next barrier
  finished[id] = 0;
  return;
}
