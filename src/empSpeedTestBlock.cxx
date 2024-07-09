#include "uhalspeedtest.hh"


namespace emp {

int SPEED_TEST::empSpeedTestBlock()
{
  uhal::ValVector< uint32_t > read_mem;

  uint64_t loops = testInfo.loops;
  const size_t block_size = testInfo.block_size;


  cout << endl << "empSpeedTestBlock" << endl 
       << std::dec << loops << " loops doing write-read of incrementing 32-bit words to " << testInfo.reg 
	    << endl;

  // https://ipbus.web.cern.ch/doc/user/html/software/uhalQuickTutorial.html
  uhal::ConnectionManager lConnectionMgr("file://" + testInfo.emp_connections_file);
  uhal::HwInterface lHW = lConnectionMgr.getDevice(testInfo.DeviceId);
  const uhal::Node& lNode = lHW.getNode(testInfo.reg);

  uint32_t depth = lNode.getSize();
  uhal::defs::BlockReadWriteMode lMode = lNode.getMode();
  std::string ModeStr = "UNKNOWN";

  switch (lMode)
  {
    case uhal::defs::INCREMENTAL:
      mode = INCREMENTAL;
      ModeStr = "INCREMENTAL";
      // Code for INCREMENTAL mode
      break;
    
    case uhal::defs::NON_INCREMENTAL:
      mode = NON_INCREMENTAL;
      ModeStr = "NON_INCREMENTAL";
      // Code for NON_INCREMENTAL mode
      break;

    case uhal::defs::HIERARCHICAL:
      mode = PORT;
      ModeStr = "PORT";
      // Code for HIERARCHICAL mode
      break;
    
    default:
      // Code for unknown mode
      break;
  }
  
  cout << endl << "Depth of Block RAM: " << depth << " 32 bit words" << endl;
  cout << "Mode:  " << ModeStr << endl << endl;
  uint64_t intervals = loops / 10;
  std::chrono::time_point<std::chrono::high_resolution_clock> begin = std::chrono::high_resolution_clock::now();

  if(loops != 0){
      
    for(uint64_t i = 0; i < loops; ++i) {
      TestIteration(testInfo, i, lNode, block_size, begin, intervals);
    }

  }else{
    intervals = 100000;
  // infinite loop to end by sigint
    uint64_t i = 0;
    while(GlobalVars::running){
      TestIteration(i, lNode, block_size, begin, intervals);
      i++;
    }
    loops = i;
  }
  testInfo.loops = loops;

  test_summary_b(begin, testInfo);
  return 0;
}

void SPEED_TEST::TestIteration(uint64_t i, uhal::Node& lNode, std::chrono::time_point<std::chrono::high_resolution_clock> begin, uint64_t intervals)
{
    uhal::ValVector< uint32_t > read_mem;
    std::vector<uint32_t> write_mem;
  
    for(size_t j=0; j!= testInfo.block_size; ++j){
      write_mem.push_back(distrib(gen));
    }
  
  lNode.writeBlock(write_mem);
  if((i<1 || testInfo.write_only == false)){
    if(mode == NON_INCREMENTAL)
      lHW.dispatch();
    read_mem = lNode.readBlock(testInfo.block_size);
  }
  lHW.dispatch();
  
  if((i<1 || testInfo.write_only == false)) {
    for (size_t j=0; j < testInfo.block_size; ++j) {
      if(mode == INCREMENTAL){
        if (write_mem[j] != read_mem[j]) {
          cout << "R/W error at " << j << " : loop " << i << ", write_mem = " << std::hex << write_mem[j] 
        << ", read_mem = " << read_mem[j] << endl << endl;
          return -1;
        }
      }
    }
  }

  if (i < 1) {
    for (size_t j=0; j < 10; ++j) {
      cout << "write_mem = " << std::hex << write_mem[j] << ", read_mem = " << std::hex << read_mem[j] << endl;
    }
      for (uhal::ValVector<uint32_t>::const_iterator lIt = read_mem.begin(); lIt != read_mem.end(); lIt++)
        std::cout << "  0x" << std::hex << *lIt << std::endl;
  }
    
  if (i%intervals == 0 && i != 0) {
    testInfo.loops = i;
    test_print_b(begin, testInfo);
  }

 return 0;

}
}