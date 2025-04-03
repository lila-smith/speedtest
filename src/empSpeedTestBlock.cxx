#include "uhalspeedtest.hh"


namespace emp {

int SPEED_TEST::empSpeedTestBlock()
{
  std::random_device rd;  //Will be used to obtain a seed for the random number engine
  std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<unsigned int> distrib(0, 0xFFFFFFFF);

  uint64_t loops = testInfo.loops;

  cout << endl << "empSpeedTestBlock" << endl 
       << std::dec << loops << " loops doing write-read of incrementing 32-bit words to " << testInfo.reg 
	    << endl;

  // https://ipbus.web.cern.ch/doc/user/html/software/uhalQuickTutorial.html
  uhal::ConnectionManager lConnectionMgr("file://" + testInfo.emp_connections_file);
  uhal::HwInterface lHW = lConnectionMgr.getDevice(testInfo.DeviceId);
  const uhal::Node& lNode =  lHW.getNode(testInfo.reg);

  uhal::defs::BlockReadWriteMode lMode = lNode.getMode(); // Reading in the mode, this is set in the CM firmware
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
  
  cout << endl << "Depth of Block RAM: " << lNode.getSize() << " 32 bit words" << endl;
  cout << "Mode:  " << ModeStr << endl << endl;
  uint64_t intervals = loops / 10;
  std::chrono::time_point<std::chrono::high_resolution_clock> begin = std::chrono::high_resolution_clock::now();

  if(loops != 0){
      
    for(uint64_t i = 0; i < loops; ++i) {
      std::vector<uint32_t> write_mem;
  
      for(size_t j=0; j!= testInfo.block_size; ++j){
        write_mem.push_back(distrib(gen));
      }

      TestIteration(i, lNode, lHW, begin, intervals, write_mem);
    }

  }else{
    // interval every 1 GB
    intervals = (1073741824) / (testInfo.block_size*4) ;
  // infinite loop to end by sigint
    uint64_t i = 0;
    while(GlobalVars::running){
      std::vector<uint32_t> write_mem;
  
      for(size_t j=0; j!= testInfo.block_size; ++j){
        write_mem.push_back(distrib(gen));
      }

      TestIteration(i, lNode, lHW, begin, intervals, write_mem);
      i++;
    }
    loops = i;
  }
  testInfo.loops = loops;

  test_summary_b(begin, testInfo);
  return 0;
}

int SPEED_TEST::TestIteration(uint64_t& i, const uhal::Node& lNode, uhal::HwInterface& lHW, std::chrono::time_point<std::chrono::high_resolution_clock>& begin, uint64_t& intervals, std::vector<uint32_t>& write_mem)
{
  uhal::ValVector< uint32_t > read_mem;

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

  if (i < 1 && testInfo.loops != 0) {
    int k = 0;
    for (size_t j=0; j < 10; ++j) {
      cout << "write_mem = " << std::hex << write_mem[j] << ", read_mem = " << std::hex << read_mem[j] << endl;
    }
      for (uhal::ValVector<uint32_t>::const_iterator lIt = read_mem.begin(); lIt != read_mem.end(); lIt++){
        std::cout << "  0x" << std::hex << *lIt << std::endl;
        if (k > 10)
          break;
        k++;
      }
  }
    
  if (i%intervals == 0 && i != 0) {
    testInfo.loops = i;
    test_print_b(begin, testInfo);
  }

 return 0;

}
}