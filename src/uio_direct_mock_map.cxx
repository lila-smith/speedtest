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
namespace emp {

int SPEED_TEST::uio_direct_mock_map(TestInfo testInfo)
{
    uint32_t write_mem;
    uint32_t read_mem;

    uint64_t loops = testInfo.loops;

    uint32_t address= testInfo.uio_address;
    uint32_t count = 1;
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

    //Add mock map with 20 items and search for the same item and add value to write

    std::map<uint32_t, uint32_t> mock_map;
    mock_map[1] = distrib(gen);
    mock_map[2] = distrib(gen);
    mock_map[5] = distrib(gen);
    mock_map[9] = distrib(gen);
    mock_map[13] = distrib(gen);
    mock_map[14] = distrib(gen);
    mock_map[15] = distrib(gen);
    mock_map[19] = distrib(gen);
    mock_map[23] = distrib(gen);
    mock_map[32] = distrib(gen);
    mock_map[33] = distrib(gen);
    mock_map[35] = distrib(gen);
    mock_map[38] = distrib(gen);
    mock_map[43] = distrib(gen);
    mock_map[56] = distrib(gen);
    mock_map[40] = distrib(gen);
    mock_map[18] = distrib(gen);
    mock_map[67] = distrib(gen);
    mock_map[70] = distrib(gen);
    mock_map[72] = distrib(gen);
    mock_map[90] = distrib(gen);

    
    auto begin = std::chrono::high_resolution_clock::now();

    cout << endl << "UIO Direct Mock Map Speedtest" << endl 
        << std::dec << loops << " loops doing write-read of incrementing 32-bit words to " << testInfo.reg 
            << endl << endl; 
    if(loops != 0){
      for(uint32_t i = 0; i < loops; ++i) {        
        
        uint32_t const & mock = (--(mock_map.upper_bound(20)))->second;

        write_mem = distrib(gen) + mock;
        ptr[address] = write_mem;
        read_mem = ptr[address];
            
        if (write_mem != read_mem) {
          cout << "R/W error: loop " << i << ", write_mem = " << std::hex << write_mem 
              << ", read_mem = " << read_mem << endl << endl;
          return -1;
        }
        
        if (i < 10) {
          
          cout << "write_mem = " << std::hex << write_mem << ", read_mem = " << read_mem << ", mock = " << mock << endl;
        }
        
        if (i%100000 == 0 && i != 0) {
          testInfo.loops = i;
          test_print(begin, testInfo);
        }

      }
    }else{
      uint64_t i = 0;
      // infinite loop to end by sigint
      while(GlobalVars::running){

        uint32_t const & mock = (--(mock_map.upper_bound(20)))->second;

        write_mem = distrib(gen) + mock;
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
          testInfo.loops = i;
          test_print(begin, testInfo);
        }
        i++;
      }
      testInfo.loops = i;
    }
    test_summary(begin, testInfo);
    return 0;
}
}
