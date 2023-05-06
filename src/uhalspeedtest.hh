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

class UHAL_TEST
{
 public:
  
  UHAL_TEST(){
    //    cout << "In the constructor" << endl;
  };

  ApolloSM * SM;

  //my own test using getNode for faster sppeds
  int uhalspeedtest(string node, uint32_t loops);

  //From Butler's code, should be no different
  int AXI_C2C_loop_back_test(string node, uint32_t loops);

  //int MCU_I2C_loop_back_test(uint32_t loops);

};

#endif

