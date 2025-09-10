#ifndef __UHAL_TEST_HH
#define __UHAL_TEST_HH

#include <ApolloSM/ApolloSM.hh>
#include <ApolloSM/ApolloSM_Exceptions.hh>
#include "uhal/uhal.hpp"
#include "bram_service.h"

#include <vector>
using std::vector;
#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;
#include <fstream>

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <boost/filesystem.hpp>
#include <chrono>
#include <random>

struct TestInfo {
  std::string reg;
  uint64_t loops;
  size_t block_size;
  uint32_t uio_address;
  bool write_only;
  std::string emp_connections_file;
  std::string DeviceId; 
}; 

 enum Mode {INCREMENTAL,NON_INCREMENTAL,PORT,UNKNOWN};

namespace GlobalVars {
    extern bool running;
    extern string logFileName;
}
namespace emp {
namespace uhal_mock {
  namespace exception
  {
    UHAL_DEFINE_EXCEPTION_CLASS ( UIOBusError , "Exception class for when an axi transaction causes a BUS_ERROR." )
  }
  class UIO
  {
    public:
      UIO(){
        //cout << "In the constructor" << endl;
      };
      void SetupSignalHandler();
      void RemoveSignalHandler();
      struct sigaction saBusError;
      struct sigaction saBusError_old;
  };
}
class SPEED_TEST
{
 public:
  
  SPEED_TEST(){

  };
  ApolloSM * SM;
  
  //my own test using getNode for faster sppeds
  int uhalWriteNode();

  //From Butler's code, should be no different
  int uhalWriteRegister();

  //Block read/write to SM Fabric
  int uhalWriteBlock();

  //Fastest speeds by using UIO
  int uio_direct();

  //Same as uio but performs mock map search to simulate uhalspeedtest
  int uio_direct_mock_map();

  //Same as uio but sets up BUS_ERROR_PROTECTION
  int uio_direct_sigbus();

  //using emp to write through IPBUS to the CM
  int empSpeedTest();

  //using emp to write through IPBUS to the CM with Block Read/Write
  int empSpeedTestBlock();

  //using uio to write through to CM
  int c2cSpeedTest();

  //using mmaped uio memcpy to write through to CM
  int psDMASpeedTest();

  int TestIteration(uint64_t& i, const uhal::Node& lNode, uhal::HwInterface& lHW, std::chrono::time_point<std::chrono::high_resolution_clock>& begin, uint64_t& intervals, std::vector<uint32_t>& write_mem);

  TestInfo testInfo;

  Mode mode; 

};

};


uint64_t aSearchDeviceTree(std::string const & dvtPath,std::string const & name);

int alabel2uio_old(std::string ilabel);

int alabel2uio(std::string ilabel);

size_t aReadFileToBuffer(std::string const & fileName,char * buffer,size_t bufferSize);

void test_summary(std::chrono::time_point<std::chrono::high_resolution_clock> begin, TestInfo testInfo);

void test_summary_b(std::chrono::time_point<std::chrono::high_resolution_clock> begin, TestInfo testInfo);

void test_print(std::chrono::time_point<std::chrono::high_resolution_clock> begin, TestInfo testInfo);

void test_print_b(std::chrono::time_point<std::chrono::high_resolution_clock> begin, TestInfo testInfo);


#endif

