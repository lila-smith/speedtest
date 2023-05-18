#include "uhalspeedtest.hh"
#include <string>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <unistd.h>
#include <sys/mman.h>
#include <random>

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

int SPEED_TEST::uhalspeedtest(string reg, uint32_t loops)
{
    uint32_t write_mem;
    uint32_t read_mem;
    double speed;

    uint32_t address= 0x000007F0;
    uint32_t count = 1;
    char* UIO_DEBUG = getenv("UIO_DEBUG");

    size_t delim = reg.find('.');
    std::string device_name = reg.substr(0, delim);

    int uio = label2uio(device_name);

    if(uio < 0){
        // try the old version
        if (NULL != UIO_DEBUG) {
            printf("simple UIO finder failed, trying legacy\n");
        }

        uio = label2uio_old(device_name);
        if (uio < 0) {
            // at this point, old version has failed.
            fprintf(stderr,"Device not found\n");
            return 1;
        }
    }

    char UIOFilename[] = "/dev/uioXXXXXXXXXX";
    snprintf(UIOFilename,strlen(UIOFilename),
        "/dev/uio%d",uio);

    //Open UIO
    int fdUIO = open(UIOFilename,O_RDWR);
    if(fdUIO < 0){
        fprintf(stderr,"Error");
        return 1;
    }

    uint32_t * ptr = (uint32_t *) mmap(NULL,sizeof(uint32_t)*(address+count), PROT_READ|PROT_WRITE, MAP_SHARED, fdUIO,0x0);

    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<unsigned int> distrib(0, 0xFFFFFFFF);

    auto begin = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count();
        
    cout << endl << "UIO Speedtest" << endl 
        << std::dec << loops << " loops doing write-read of incrementing 32-bit words to " << reg 
            << endl << endl; 
    
    for(uint32_t i = 0; i < loops; ++i) {        
        
        write_mem = distrib(gen);
		ptr[address] = distrib(gen);
		read_mem = ptr[address];

        if (write_mem != read_mem) {
        cout << "R/W error: loop " << i << ", write_mem = " << std::hex << write_mem 
            << ", read_mem = " << read_mem << endl << endl;
        return -1;
        }

        if (i < 10) {
        cout << "write_mem = " << std::hex << write_mem << ", read_mem = " << read_mem << endl;
        }
        
        if (i%100000 == 0 && i != 0) {
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count();
        speed = 2.*32.*i/duration;
        cout << std::dec << i << " reads done, speed = " << speed <<  " Mbps" << endl;
        }

    }
    
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count();

    cout << endl << "Speed test: " << std::dec << loops << " write-reads of " << reg << endl;
    cout << duration << " us total, average : " << duration / loops << " us." << endl;

    speed = 2.*32.*loops/duration;
    cout << "Speed = " << speed << " Mbps" << endl;

    return 0;
}

