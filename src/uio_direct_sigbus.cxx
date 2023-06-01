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
#include "uhalspeedtest.hh"

#define BUS_ERROR_PROTECTION(ACCESS) \
  if(SIGBUS == sigsetjmp(env,1)){						\
    uhal::exception::UIOBusError * e = new uhal::exception::UIOBusError();\
    throw *e;\
  }else{ \
    ACCESS;					\
  }


int SPEED_TEST::uio_direct_sigbus(string reg, uint64_t loops)
{
    uint32_t write_mem;
    uint32_t read_mem;
    double speed;

    uint32_t address= 0x000007F0;
    uint32_t count = 1;
    char* UIO_DEBUG = getenv("UIO_DEBUG");

    size_t delim = reg.find('.');
    std::string device_name = reg.substr(0, delim);

    int uio = alabel2uio(device_name);

    if(uio < 0){
        // try the old version
        if (NULL != UIO_DEBUG) {
            printf("simple UIO finder failed, trying legacy\n");
        }

        uio = alabel2uio_old(device_name);
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
        
    cout << endl << "UIO Direct Speedtest" << endl 
        << std::dec << loops << " loops doing write-read of incrementing 32-bit words to " << reg 
            << endl << endl; 
    if(loops != 0){
      for(uint32_t i = 0; i < loops; ++i) {        
          
        write_mem = distrib(gen);
        ptr[address] = write_mem;
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
    }else{
      uint32_t i = 0;
      // infinite loop to end by sigint
      while(GlobalVars::running){
        write_mem = distrib(gen);
        ptr[address] = write_mem;
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
        i++;
      }
      loops = i;
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count();

    cout << endl << "Speed test: " << std::dec << loops << " write-reads of " << reg << endl;
    cout << duration << " us total, average : " << (double) duration / loops  << " us." << endl;

    speed = 2.*32.*loops/duration;
    cout << "Speed = " << speed << " Mbps" << endl;

    return 0;
}

