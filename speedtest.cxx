#include <string>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <unistd.h>

#include "uioLabelFinder.hh"
#include <sys/mman.h>
//g++ -std=c++11 -Wall -rdynamic -lboost_system -lboost_filesystem -lpthread speedtest.cxx -o speedtest
int main(){
uint32_t address= 0x000007F0;
uint32_t count = 1;
char* UIO_DEBUG = getenv("UIO_DEBUG");

// found using ls /dev/uio*
std::string device_name  = "PL_MEM";

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

  uint32_t * ptr = (uint32_t *) mmap(NULL,sizeof(uint32_t)*(address+count),
				  PROT_READ|PROT_WRITE, MAP_SHARED,
				  fdUIO,0x0);
  
  uint32_t length = 1000000;
  auto start = std::chrono::high_resolution_clock::now();
  int error = 0;
  for(uint32_t i = 0;i<length;i++){
    ptr[address] = i;
    if(ptr[address] != i){
      error++;
    }
  }
  
  auto stop = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
  std::cout << "Time taken by loop: " << duration.count() << " milliseconds" << std::endl;
  
  std::cout << "Errors: " << error << std::endl;
  //printf("Errors: %d",error);
  double time_ms = std::chrono::duration<double>(duration).count();
  // writing a single word so 4 bytes
  double bytes =  (double) length*4;
  double bps =  bytes / (time_ms);
  std::cout << "Duration: " << time_ms << std::endl;
  std::cout << "Bytes per second: " << bps << std::endl;

  
  return 0;
}
