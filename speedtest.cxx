#include <string>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <unistd.h>

#include <uioLabelFinder.hh>
#include <sys/mman.h>

int main(){
uint32_t address= 0x000007F0;
uint32_t count = 1;

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
    fprintf(stderr,"%s not found\n",device_name);
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
  
  int length = 1000;
  auto start = high_resolution_clock::now();
  int error = 0;
  for(int i = 0;i<length;i++){
    ptr[address] = i;
    if(ptr[address] != i){
      error++;
    }
  }
  
   auto stop = high_resolution_clock::now();
   auto duration = duration_cast<microseconds>(stop - start);
   std::cout << "Time taken by loop: " << duration.count() << " microseconds" << std::endl;
  
  printf("Errors: %d",error);
  printf("Bytes per ms: %d",length*sizeof(uint32_t)/(duration*10^3))

  return 0;
}
