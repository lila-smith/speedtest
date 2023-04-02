#include <string>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/mman.h>

int main(){
uint32_t address;
uint32_t count = 1;

int uio = label2uio(argv[1]);

char UIOFilename[] = "/dev/uioXXXXXXXXXX ";
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
}