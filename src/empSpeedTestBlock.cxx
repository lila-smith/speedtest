#include "uhalspeedtest.hh"
#include <chrono>
#include <random>

namespace emp {

int SPEED_TEST::empSpeedTestBlock(TestInfo testInfo)
{
  uhal::ValVector< uint32_t > read_mem;

  std::random_device rd;  //Will be used to obtain a seed for the random number engine
  std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<unsigned int> distrib(0, 0xFFFFFFFF);

  uint64_t loops = testInfo.loops;
  const size_t block_size = testInfo.block_size;

  const std::string lConnectionFilePath = testInfo.emp_connections_file;
  const std::string lDeviceId = testInfo.DeviceId;
  const std::string lRegisterName = testInfo.reg;

  std::chrono::time_point<std::chrono::high_resolution_clock> begin = std::chrono::high_resolution_clock::now();

  cout << endl << "empSpeedTestBlock" << endl 
       << std::dec << loops << " loops doing write-read of incrementing 32-bit words to " << lRegisterName 
	    << endl;

  // https://ipbus.web.cern.ch/doc/user/html/software/uhalQuickTutorial.html
  uhal::ConnectionManager lConnectionMgr("file://" + lConnectionFilePath);
  uhal::HwInterface lHW = lConnectionMgr.getDevice(lDeviceId);
  const uhal::Node& lNode = lHW.getNode(lRegisterName);

  uint32_t depth = lNode.getSize();
  uhal::defs::BlockReadWriteMode lMode = lNode.getMode();
  bool incremental = (lMode == uhal::defs::INCREMENTAL) 

  cout << endl << "Depth of Block RAM: " << depth << " 32 bit words" << endl << endl;
 
  if(loops != 0){
      uint64_t intervals = loops / 10;
      for(uint64_t i = 0; i < loops; ++i) {
        std::vector<uint32_t> write_mem;
      
        for(size_t j=0; j!= block_size; ++j){
          write_mem.push_back(distrib(gen));
        }
      
      lNode.writeBlock(write_mem);
      if(i<1 || testInfo.write_only == false && incremental){
        read_mem = lNode.readBlock(block_size);
      }
      lHW.dispatch();
      
      if(i<1 || testInfo.write_only == false && incremental) {
        for (size_t j=0; j < block_size; ++j) {
          if (write_mem[j] != read_mem[j]) {
            cout << "R/W error: loop " << i << ", write_mem = " << std::hex << write_mem[j] 
          << ", read_mem = " << read_mem[j] << endl << endl;
            return -1;
          }
        }
      }

      if (i < 1) {
        for (size_t j=0; j < 10; ++j) {
          cout << "write_mem = " << std::hex << write_mem[j] << ", read_mem = " << std::hex << read_mem[j] << endl;
        }
      }
        
      if (i%intervals == 0 && i != 0) {
        testInfo.loops = i;
        test_print_b(begin, testInfo);
      }

    }
  }else{
  // infinite loop to end by sigint
    uint64_t i = 0;
    while(GlobalVars::running){
      std::vector<uint32_t> write_mem;

      for(size_t j=0; j!= block_size; ++j){
          write_mem.push_back(distrib(gen));
        }
      
      lNode.writeBlock(write_mem);
      if(i<1 || testInfo.write_only == false && incremental){
        read_mem = lNode.readBlock(block_size);
      }
      lHW.dispatch();

      if (i < 1 || testInfo.write_only == false && incremental) {
        for (size_t j=0; j < block_size; ++j)
          if (write_mem[j] != read_mem[j]) {
            cout << "R/W error: loop " << i << ", write_mem = " << std::hex << write_mem[j] << ", read_mem = " << read_mem[j] << endl << endl;
            return -1;
          }
        }

      if (i < 1) {
        for (size_t j=0; j < 10; ++j) {
          cout << "write_mem = " << std::hex << write_mem[j] << ", read_mem = " << std::hex << read_mem[j] << endl;
        }
      }
        
      if (i%100000 == 0 && i != 0) {
        testInfo.loops = i;
        test_print_b(begin, testInfo);
      }
      i++;
    }
    loops = i;
  }
  testInfo.loops = loops;

  test_summary_b(begin, testInfo);
  return 0;
}
}

