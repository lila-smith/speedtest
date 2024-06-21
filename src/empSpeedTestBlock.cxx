#include "uhalspeedtest.hh"
#include <chrono>
#include <random>

namespace emp {

int SPEED_TEST::empSpeedTest(string reg, uint64_t loops, string emp_connections_file)
{
  std::vector<uint32_t> write_mem;
  uhal::ValVector< uint32_t > read_mem;

  std::random_device rd;  //Will be used to obtain a seed for the random number engine
  std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<unsigned int> distrib(0, 0xFFFFFFFF);

  const std::string lConnectionFilePath = emp_connections_file;
  const std::string lDeviceId = "F1_IPBUS";
  const std::string lRegisterName = reg;

  std::chrono::time_point<std::chrono::high_resolution_clock> begin = std::chrono::high_resolution_clock::now();

  cout << endl << "empSpeedTest" << endl 
       << std::dec << loops << " loops doing write-read of incrementing 32-bit words to " << lRegisterName 
	    << endl << endl;

  // https://ipbus.web.cern.ch/doc/user/html/software/uhalQuickTutorial.html
  uhal::ConnectionManager lConnectionMgr("file://" + lConnectionFilePath);
  uhal::HwInterface lHW = lConnectionMgr.getDevice(lDeviceId);
  const uhal::Node& lNode = lHW.getNode(lRegisterName);
  const size_t N=100;

  if(loops != 0){
      for(uint64_t i = 0; i < loops/N; ++i) {
      
        for(size_t j=0; j!= N; ++j){
          write_mem.push_back(distrib(gen));
        }
      
      lNode.writeBlock(write_mem);
      read_mem = lNode.readBlock(N);
      lHW.dispatch();
      for (size_t j=0; j < N; ++j)
        if (write_mem[j] != read_mem[j].value()) {
          cout << "R/W error: loop " << i << ", write_mem = " << std::hex << write_mem[j] 
        << ", read_mem = " << read_mem[j].value() << endl << endl;
          return -1;
        }

      if (i < 1) {
        for (size_t j=0; j < 10; ++j) {
          cout << "write_mem = " << std::hex << write_mem[j] << ", read_mem = " << std::hex << read_mem[j].value() << endl;
        }
      }
        
      if (i%100000 == 0 && i != 0) {
        test_print(begin, i);
      }

    }
  }else{
  // infinite loop to end by sigint
    uint64_t i = 0;
    while(GlobalVars::running){

      for(size_t j=0; j!= N; ++j){
          write_mem.push_back(distrib(gen));
        }
      
      lNode.writeBlock(write_mem);
      read_mem = lNode.readBlock(N);
      lHW.dispatch();
      for (size_t i=0; i!= N; ++i)
        if (write_mem[i] != read_mem[i].value()) {
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
      i++;
    }
    loops = i;
  }

  test_summary(begin, loops, reg);
  return 0;
}
}

