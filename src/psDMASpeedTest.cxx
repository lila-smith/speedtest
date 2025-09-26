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
#include <sys/ioctl.h>
#include "uhalspeedtest.hh"
extern "C" {
    #include "bram_service.h"
}
#include "cdmacdev.h"
namespace emp {
#define BUFFER_SIZE 0x8010
#define BUS_ERROR_PROTECTION(ACCESS) \
  if(SIGBUS == sigsetjmp(env,1)){						\
    uhal_mock::exception::UIOBusError * e = new uhal_mock::exception::UIOBusError();\
    throw *e;\
  }else{ \
    ACCESS;					\
  }
static int cdma_fd[2];
// sigjmp_buf static env;
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


int SPEED_TEST::psDMASpeedTest()
{
    int status, diffcnt;
    unsigned int uio_addr, cpuaddr, devaddr;
	unsigned int uio_size;
    uint32_t buffer_size = testInfo.block_size;
    unsigned char srcbuf[ buffer_size ];
	unsigned char dstbuf[ buffer_size ];
    unsigned char * srcp, * dstp;
    
    FILE * size_uio;
    FILE * size_addr;
    uint64_t loops = testInfo.loops;
    
    // uint32_t address= testInfo.uio_address;
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
    char deviceFileName[] = "/dev/cdmachX";
    snprintf(UIOFilename,strlen(UIOFilename),
        "/dev/uio%d",uio);
    snprintf(UIOSizePath,strlen(UIOSizePath),
        "/sys/class/uio/uio%d/maps/map0/size",uio);
    snprintf(UIOAddrPath,strlen(UIOAddrPath),
        "/sys/class/uio/uio%d/maps/map0/addr",uio);
    //Open UIO
    // int fdUIO = open(UIOFilename,O_RDWR);
    // if(fdUIO < 0){
    //     fprintf(stderr,"UIO open error");
    //     return 1;
    // }
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

    for(uint32_t i = 0; i < 2; ++i) {
        snprintf(deviceFileName, sizeof(deviceFileName),
            "/dev/cdmach%d", i);
        // Initialize destination pointer
        cdma_fd[i] = open( deviceFileName, O_RDWR );
        if( cdma_fd[i] < 0 ) {
            printf("Failed to open /dev/cdmach\n" );
            return -1;
        }
        status = ioctl( cdma_fd[i], CDMACDEV_IOC_SET_DEV_ADDR, uio_addr );
        if( status < 0 ) {
            printf("ioctl IOC_SET_DEV_ADDR failed\n");
        }

        status = ioctl( cdma_fd[i], CDMACDEV_IOC_GET_DEV_ADDR, & devaddr );
        if( status < 0 ) {
            printf("ioctl IOC_GET_DEV_ADDR failed\n");
        } else {
            printf("ioctl IOC_GET_DEV_ADDR returned = 0x%x\n", devaddr );
        }

        status = ioctl( cdma_fd[i], CDMACDEV_IOC_SET_CPU_ADDR, 0x0 );
        if( status < 0 ) {
            printf("ioctl IOC_SET_CPU_ADDR failed\n");
        }

        status = ioctl( cdma_fd[i], CDMACDEV_IOC_GET_CPU_ADDR, & cpuaddr );
        if( status < 0 ) {
            printf("ioctl IOC_GET_CPU_ADDR failed\n");
        } else {
            printf("ioctl IOC_GET_CPU_ADDR returned = 0x%x\n", cpuaddr );
        }
        // uint32_t * ptr = (uint32_t *) mmap(NULL, uio_size, PROT_READ|PROT_WRITE, MAP_SHARED, fdUIO,0x0);
    }
    

    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<unsigned int> distrib(0, 0xFF);

    auto begin = std::chrono::high_resolution_clock::now();

    uhal_mock::UIO* mock_uio = new uhal_mock::UIO();
    mock_uio->SetupSignalHandler();
    
    cout << endl << "UIO Direct DMA Speedtest" << endl 
        << std::dec << loops << " loops doing write-read of incrementing " << buffer_size << " bytes to " << testInfo.reg 
            << endl << endl; 
    

    
    for(uint32_t i = 0; i < loops; ++i) { 
        srcp = srcbuf;
        dstp = dstbuf;
        for(uint32_t j = 0; j < buffer_size; j ++ ) {
            * srcp ++ = ( unsigned char ) ( distrib(gen) & 0xff );
            * dstp ++ = '\0';
	    }
        status = write( cdma_fd[0], srcbuf, buffer_size );
        status = read( cdma_fd[0], dstbuf, buffer_size );
        

        
        if (i < 10) {
            cout << std::hex << "address = " << uio_addr << endl;
            
            diffcnt = 0;
            srcp = srcbuf;
            dstp = dstbuf;

            for(uint32_t j = 0; j < buffer_size; j ++ ) {
                if( * srcp ++ != * dstp ++ ) diffcnt ++;
            }
            if( diffcnt ) printf("%d differences detected\n", diffcnt );
            else printf("No difference found\n");
        }
        
        if (i%100000 == 0 && i != 0) {
            testInfo.loops = i*(buffer_size/4);
            test_print(begin, testInfo);
        }
        
      }
    
    testInfo.loops = loops*(buffer_size/4);
    test_summary(begin, testInfo);
    // if (munmap(ptr, uio_size) == -1) {
	//     perror("Error un-mmapping the file");
    // }
    close(cdma_fd[0]);
    close(cdma_fd[1]);
    bram_exit( );
    return 0;
}
}
