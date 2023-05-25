#ifndef __UHAL_TEST_HH
#define __UHAL_TEST_HH

#include <ApolloSM/ApolloSM.hh>
#include <ApolloSM/ApolloSM_Exceptions.hh>

#include <vector>
using std::vector;
#include <string>
using std::string;
#include <iostream>
using std::cout;
using std::endl;
#include <fstream>

namespace GlobalVars {
  volatile bool running = true;
}


class SPEED_TEST
{
 public:
  
  SPEED_TEST(){
    //    cout << "In the constructor" << endl;
  };

  ApolloSM * SM;

  //my own test using getNode for faster sppeds
  int uhalspeedtest(string reg, uint32_t loops);

  //From Butler's code, should be no different
  int AXI_C2C_loop_back_test(string node, uint32_t loops);
  
  //Fastest speeds by using UIO
  int uio_direct(string reg, uint32_t loops);

  //Same as uio but performs mock map search to simulate uhalspeedtest
  int uio_direct_mock_map(string reg, uint32_t loops);

};


#endif

