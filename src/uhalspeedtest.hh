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

namespace GlobalVars {
    extern bool running;
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
// class TestNode : public uhal::Node {
//   UHAL_DERIVEDNODE(TestNode)
//   public:
//   TestNode(const uhal::Node& aNode) :
//     uhal::Node(aNode) { }
//   virtual ~TestNode() { }
//   void write(string reg, unsigned aData) const;
//   unsigned read(string reg) const;
// };
class SPEED_TEST
{
 public:
  
  SPEED_TEST(){
    //    cout << "In the constructor" << endl;
  };

  ApolloSM * SM;

  //my own test using getNode for faster sppeds
  int uhalspeedtest(string reg, uint64_t loops);

  //From Butler's code, should be no different
  int AXI_C2C_loop_back_test(string node, uint64_t loops);

  //Fastest speeds by using UIO
  int uio_direct(string reg, uint64_t loops, uint32_t uio_address);

  //Same as uio but performs mock map search to simulate uhalspeedtest
  int uio_direct_mock_map(string reg, uint64_t loops, uint32_t uio_address);

  //Same as uio but sets up BUS_ERROR_PROTECTION
  int uio_direct_sigbus(string reg, uint64_t loops, uint32_t uio_address);

  //using emp to write through IPBUS to the CM
  int empSpeedTest(string reg, uint64_t loops, string emp_connections_file);
};
}

uint64_t aSearchDeviceTree(std::string const & dvtPath,std::string const & name);

int alabel2uio_old(std::string ilabel);

int alabel2uio(std::string ilabel);

size_t aReadFileToBuffer(std::string const & fileName,char * buffer,size_t bufferSize);

#endif

