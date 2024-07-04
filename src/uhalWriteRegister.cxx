#include "uhalspeedtest.hh"
#include <chrono>
#include <random>
namespace emp {

int SPEED_TEST::uhalWriteRegister(TestInfo testInfo)
{
  uint32_t write_mem;
  uint32_t read_mem;

  const string node = testInfo.reg;

  std::random_device rd;  //Will be used to obtain a seed for the random number engine
  std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<unsigned int> distrib(0, 0xFFFFFFFF);

  auto begin = std::chrono::high_resolution_clock::now();
    
  cout << endl << "uhal speedtest" << endl 
       << std::dec << loops << " loops doing write-read of random 32-bit words to " << node 
	    << endl << endl; 
  if(loops != 0){
    for(uint64_t i = 0; i < loops; ++i) {

      write_mem = distrib(gen);
      SM->WriteRegister(node,write_mem);
      read_mem = SM->ReadRegister(node);

      if (write_mem != read_mem) {
        cout << "R/W error: loop " << i << ", write_mem = " << std::hex << write_mem 
      << ", read_mem = " << read_mem << endl << endl;
        return -1;
      }

      if (i < 10) {
        cout << "write_mem = " << std::hex << write_mem << ", read_mem = " << read_mem << endl;
      }
        
      if (i%100000 == 0 && i != 0) {
        testInfo.loops = i;
        test_print(begin, testInfo);
      }
    }
  }else{
  // infinite loop to end by sigint
    uint64_t i = 0;
    while(GlobalVars::running){
      
        write_mem = distrib(gen);
        SM->WriteRegister(node,write_mem);
        read_mem = SM->ReadRegister(node);

        if (write_mem != read_mem) {
          cout << "R/W error: loop " << i << ", write_mem = " << std::hex << write_mem 
        << ", read_mem = " << read_mem << endl << endl;
          return -1;
        }

        if (i < 10) {
          cout << "write_mem = " << std::hex << write_mem << ", read_mem = " << read_mem << endl;
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