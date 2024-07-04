#ifndef __UHAL_TEST_HH
#define __UHAL_TEST_HH

#include <ApolloSM/ApolloSM.hh>
#include <ApolloSM/ApolloSM_Exceptions.hh>
#include "uhal/uhal.hpp"

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

struct TestInfo {
  std::string reg;
  uint64_t loops;
  size_t block_size;
  uint32_t uio_address;
  bool write_only;
  std::string emp_connections_file;
  std::string DeviceId;
}; 

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
    //    cout << "In the constructor" << endl;
  };
  ApolloSM * SM;
  
  //my own test using getNode for faster sppeds
  int uhalWriteNode(TestInfo testInfo);

  //From Butler's code, should be no different
  int uhalWriteRegister(TestInfo testInfo);

  //Fastest speeds by using UIO
  int uio_direct(TestInfo testInfo);

  //Same as uio but performs mock map search to simulate uhalspeedtest
  int uio_direct_mock_map(TestInfo testInfo);

  //Same as uio but sets up BUS_ERROR_PROTECTION
  int uio_direct_sigbus(TestInfo testInfo);

  //using emp to write through IPBUS to the CM
  int empSpeedTest(TestInfo testInfo);

  //using emp to write through IPBUS to the CM with Block Read/Write
  int empSpeedTestBlock(TestInfo testInfo);
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

