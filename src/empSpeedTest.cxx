#include "uhalspeedtest.hh"
#include <chrono>
#include <random>

namespace emp {

int SPEED_TEST::empSpeedTest(TestInfo testInfo)
{
  uint32_t write_mem;
  uhal::ValWord<uint32_t> read_mem;

  std::random_device rd;  //Will be used to obtain a seed for the random number engine
  std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<unsigned int> distrib(0, 0xFFFFFFFF);

  uint64_t loops = testInfo.loops;

  const std::string lConnectionFilePath = testInfo.emp_connections_file;
  const std::string lDeviceId = testInfo.DeviceId;
  const std::string lRegisterName = testInfo.reg;

  std::chrono::time_point<std::chrono::high_resolution_clock> begin = std::chrono::high_resolution_clock::now();

  cout << endl << "empSpeedTest" << endl 
       << std::dec << loops << " loops doing write-read of incrementing 32-bit words to " << lRegisterName 
	    << endl << endl;

	// https://ipbus.web.cern.ch/doc/user/html/software/uhalQuickTutorial.html
  uhal::ConnectionManager lConnectionMgr("file://" + lConnectionFilePath);
  uhal::HwInterface lHW = lConnectionMgr.getDevice(lDeviceId);
  const uhal::Node& lNode = lHW.getNode(lRegisterName);

  if(loops != 0){
      for(uint64_t i = 0; i < loops; ++i) {

      
      write_mem = distrib(gen);
      lNode.write(write_mem);
      read_mem = lNode.read();
      lHW.dispatch();

      if (write_mem != read_mem.value()) {
        cout << "R/W error: loop " << i << ", write_mem = " << std::hex << write_mem 
      << ", read_mem = " << read_mem << endl << endl;
        return -1;
      }
      if (i < 10) {
        cout << "write_mem = " << std::hex << write_mem << ", read_mem = " << std::hex << read_mem.value() << endl;
      }
        
      if (i%100000 == 0 && i != 0) {
        test_print(begin, i);
      }

    }
  }else{
  // infinite loop to end by sigint
    uint64_t i = 0;
    while(GlobalVars::running){

      write_mem = distrib(gen);
      lNode.write(write_mem);
      //lHW.dispatch();
      read_mem = lNode.read();
      lHW.dispatch();

      if (write_mem != read_mem.value()) {
        cout << "R/W error: loop " << i << ", write_mem = " << std::hex << write_mem 
      << ", read_mem = " << read_mem << endl << endl;
        return -1;
      }

      if (i < 10) {
        cout << "write_mem = " << std::hex << write_mem << ", read_mem = " << std::hex << read_mem.value() << endl;
      }
        
      if (i%100000 == 0 && i != 0) {
        testInfo.loops = i;
        test_print(begin, testInfo);
      }
      i++;
    }
    testInfo.loops = i;
  }

  test_summary(begin, testInfo);
  return 0;
}
}

