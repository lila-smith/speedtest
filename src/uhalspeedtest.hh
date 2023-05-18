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

};

#include <stdio.h>
#include <stdint.h>
#include <iostream>
#include <string.h>

#include <boost/filesystem.hpp>

using namespace boost::filesystem;

static size_t ReadFileToBuffer(std::string const & fileName,char * buffer,size_t bufferSize){
  //open the file
  FILE * inFile = fopen(fileName.c_str(),"r");
  if(NULL == inFile){
    //failed to opn file.
    return 0;
  }

  //read file  
  if(fgets(buffer,bufferSize,inFile) == NULL){
    fclose(inFile);
    return 0;
  }
  fclose(inFile);
  return 1;
}

uint64_t SearchDeviceTree(std::string const & dvtPath,std::string const & name){
  uint64_t address = 0;
  FILE *labelfile=0;
  char label[128];
  // traverse through the device-tree   
  for (directory_iterator x(dvtPath); x!=directory_iterator(); ++x){
    if (x->path().filename().native() != "label") { continue; }
    labelfile = fopen((x->path().native()).c_str(),"r");
    fgets(label,128,labelfile);
    fclose(labelfile);

    if(!strcmp(label, name.c_str())){
      //Get endpoint AXI address from path           
      // looks something like LABEL@DEADBEEFXX       
      std::string stringAddr=x->path().parent_path().native();

      //Check if we find the @          
      size_t addrStart = stringAddr.find("@");
      if(addrStart == std::string::npos){
	fprintf(stderr,"directory name %s has incorrect format. Missing \"@\" ",x->path().filename().native().c_str() );
	break; //expect the name to be in x@xxxxxxxx format for example myReg@0x41200000    
      }
      //Convert the found string into binary         
      if(addrStart+1 > stringAddr.size()){
	fprintf(stderr,"directory name %s has incorrect format. Missing size ", x->path().filename().native().c_str() );
	break; //expect the name to be in x@xxxxxxxx format for example myReg@0x41200000    

      }
      stringAddr = stringAddr.substr(addrStart+1);

      //Get the names's address from the path (in hex)            
      address = std::strtoull(stringAddr.c_str() , 0, 16);
      break;
    }
  }
  return address;
};


#endif

