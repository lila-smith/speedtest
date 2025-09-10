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
#include <signal.h>
#include <setjmp.h>
#include "uhalspeedtest.hh"
namespace emp {

#define BUS_ERROR_PROTECTION(ACCESS) \
  if(SIGBUS == sigsetjmp(env,1)){						\
    uhal_mock::exception::UIOBusError * e = new uhal_mock::exception::UIOBusError();\
    throw *e;\
  }else{ \
    ACCESS;					\
  }

sigjmp_buf static env;
// void static signal_handler(int sig){
//   if(SIGBUS == sig){
//     siglongjmp(env,sig);    
//   }
// }

// void uhal_mock::UIO::SetupSignalHandler(){
//     //this is here so the signal_handler can stay static
//     memset(&saBusError,0,sizeof(saBusError)); //Clear struct
//     saBusError.sa_handler = signal_handler; //assign signal handler
//     sigemptyset(&saBusError.sa_mask);
//     sigaction(SIGBUS, &saBusError,&saBusError_old);  //install new signal handler (save the old one)
// }

// void uhal_mock::UIO::RemoveSignalHandler(){    
//     sigaction(SIGBUS,&saBusError_old,NULL); //restore the signal handler from before creation for SIGBUS
// }

int SPEED_TEST::c2cSpeedTest()
{
    uint32_t write_mem;
    uint32_t read_mem;
    unsigned int uio_addr;
	  unsigned int uio_size;
    FILE * size_uio;
    FILE * size_addr;
    uint64_t loops = testInfo.loops;

    uint32_t address= testInfo.uio_address;
    char* UIO_DEBUG = getenv("UIO_DEBUG");

    size_t delim = testInfo.reg.find('.');
    std::string device_name = testInfo.reg.substr(0, delim);

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

    char UIOFilename[] = "/dev/uioXXXXXXXXXXXX";
    char UIOSizePath[] = "/sys/class/uio/uioXXXXXXXXXXXX/maps/map0/size";
    char UIOAddrPath[] = "/sys/class/uio/uioXXXXXXXXXXXX/maps/map0/addr";
    snprintf(UIOFilename,strlen(UIOFilename),
        "/dev/uio%d",uio);
    snprintf(UIOSizePath,strlen(UIOSizePath),
        "/sys/class/uio/uio%d/maps/map0/size",uio);
    snprintf(UIOAddrPath,strlen(UIOAddrPath),
        "/sys/class/uio/uio%d/maps/map0/addr",uio);
    //Open UIO
    int fdUIO = open(UIOFilename,O_RDWR);
    if(fdUIO < 0){
        fprintf(stderr,"UIO open error");
        return 1;
    }
    size_uio = fopen(UIOSizePath, "r");
    if (size_uio == NULL){
            perror("UIO Size Open Error:");
    }

    fscanf(size_uio,"0x%16X",&uio_size);
    printf("size of UIO Memory: 0x%x\n",uio_size);
    size_addr = fopen(UIOAddrPath, "r");

    if (size_addr == NULL){
            perror("UIO Addr Open Error:");
    }

    fscanf(size_addr,"0x%16X",&uio_addr);
    printf("addr of UIO Memory: 0x%x\n",uio_addr);

    uint32_t * ptr = (uint32_t *) mmap(NULL, uio_size, PROT_READ|PROT_WRITE, MAP_SHARED, fdUIO,0x0);

    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<unsigned int> distrib(0, 0xFFFFFFFF);

    auto begin = std::chrono::high_resolution_clock::now();

    uhal_mock::UIO* mock_uio = new uhal_mock::UIO();
    mock_uio->SetupSignalHandler();
    
    cout << endl << "UIO Direct SIGBUS Speedtest" << endl 
        << std::dec << loops << " loops doing write-read of incrementing 32-bit words to " << testInfo.reg 
            << endl << endl; 
    
    if(loops != 0){
      for(uint32_t i = 0; i < loops; ++i) { 
        
        for(uint32_t j = address; j < (uio_size / 4); ++j) {
            write_mem = distrib(gen);
            ptr[j] = write_mem;
            read_mem = ptr[j];
            // BUS_ERROR_PROTECTION(ptr[j] = write_mem);
            // BUS_ERROR_PROTECTION(read_mem = ptr[j]);
            if (write_mem != read_mem) {
              cout << "R/W error: loop " << i << ", write_mem = " << std::hex << write_mem 
                  << ", read_mem = " << read_mem << ", address = " << (j) << endl;
            }
        
            else if ((i < 10) && (j < 10)) {
              cout << "write_mem = " << std::hex << write_mem << ", read_mem = " << read_mem << ", address = " << (j) << endl;
            }
            
            if (i%100000 == 0 && i != 0) {
              testInfo.loops = i;
              test_print(begin, testInfo);
            }
        }
        
      }
      loops = loops*(uio_size / 4);
    }
    else{
      uint32_t i = 0;
      // infinite loop to end by sigint
      while(GlobalVars::running){
        
        write_mem = distrib(gen);
        BUS_ERROR_PROTECTION(ptr[address] = write_mem);
         if(i<10 || testInfo.write_only == false){
          BUS_ERROR_PROTECTION(read_mem = ptr[address]);
          if (write_mem != read_mem) {
          cout << "R/W error: loop " << i << ", write_mem = " << std::hex << write_mem 
              << ", read_mem = " << read_mem << endl << endl;
          return -1;
          }
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
      loops = i;
  }
    testInfo.loops = loops;
    mock_uio->RemoveSignalHandler();
    test_summary(begin, testInfo);

    return 0;
}
}
