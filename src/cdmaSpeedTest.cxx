#include "uhalspeedtest.hh"


namespace cdma {

int SPEED_TEST::cdmaSpeedTest()
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
	    << ", Device ID: " lDeviceId << << endl << endl;

	// https://ipbus.web.cern.ch/doc/user/html/software/uhalQuickTutorial.html
  uhal::ConnectionManager lConnectionMgr("file://" + lConnectionFilePath);
  uhal::HwInterface lHW = lConnectionMgr.getDevice(lDeviceId);
  const uhal::Node& lNode = lHW.getNode(lRegisterName);

  cout << endl << "mAddr: " << lNode.mAddr << ", mUid: " << lNode.mUid << endl << endl;
  // if(loops != 0){
  //     for(uint64_t i = 0; i < loops; ++i) {

      
  //     write_mem = distrib(gen);
  //     lNode.write(write_mem);
  //      if(i<10 || testInfo.write_only == false)
  //       read_mem = lNode.read();
  //     lHW.dispatch();

  //     if(i<10 || testInfo.write_only == false){
  //       if (write_mem != read_mem.value()) {
  //         cout << "R/W error: loop " << i << ", write_mem = " << std::hex << write_mem 
  //       << ", read_mem = " << read_mem << endl << endl;
  //         return -1;
  //       }
  //     }
  //     if (i < 10) {
  //       cout << "write_mem = " << std::hex << write_mem << ", read_mem = " << std::hex << read_mem.value() << endl;
  //     }
        
  //     if (i%100000 == 0 && i != 0) {
  //       testInfo.loops = i;
  //       test_print(begin, testInfo);
  //     }

  //   }
  // }else{
  // // infinite loop to end by sigint
  //   uint64_t i = 0;
  //   while(GlobalVars::running){

  //     write_mem = distrib(gen);
  //     lNode.write(write_mem);
  //     if(i<10 || testInfo.write_only == false)
  //       read_mem = lNode.read();
  //     lHW.dispatch();

  //     if(i<10 || testInfo.write_only == false){
  //       if (write_mem != read_mem.value()) {
  //         cout << "R/W error: loop " << i << ", write_mem = " << std::hex << write_mem 
  //       << ", read_mem = " << read_mem << endl << endl;
  //         return -1;
  //       }
  //     }

  //     if (i < 10) {
  //       cout << "write_mem = " << std::hex << write_mem << ", read_mem = " << std::hex << read_mem.value() << endl;
  //     }
        
  //     if (i%100000 == 0 && i != 0) {
  //       testInfo.loops = i;
  //       test_print(begin, testInfo);
  //     }
  //     i++;
  //   }
  //    loops = i;
  // }
  // testInfo.loops = loops;
  // test_summary(begin, testInfo);
  return 0;
}
}

