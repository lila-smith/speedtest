#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>
#include "cdmacdev.h"
#include "bram_service.h"
// gcc cdma-test.c bram_service.c plmem_service.c -o cdma-test
#define BUFFER_SIZE 2048
#define BRAM_BASE_ADDR 0xB0000000 // 0xA0003000
#define BRAM_SIZE 0x1000

static int cdma_fd;
static unsigned char * bram_ptr;

static uint64_t get_posix_clock_time_usec ()
{
    struct timespec ts;

    if (clock_gettime (CLOCK_MONOTONIC, &ts) == 0)
        return (uint64_t) (ts.tv_sec * 1000000 + ts.tv_nsec / 1000);
    else
        return 0;
}

int test1( ) {

	uint64_t start_time, end_time, time_diff;
	unsigned char buf[ BUFFER_SIZE ];
	unsigned char * p;
	int i;
	int ntransfer;

	printf("Test #1: transfer speed to BRAM by program control\n");

	p = buf;
	for( i = 0; i < BUFFER_SIZE; i ++ ) * p ++ = ( unsigned char )( i & 0xff );

    start_time = get_posix_clock_time_usec();
	ntransfer = bram_write( buf, 0, BUFFER_SIZE );
    end_time = get_posix_clock_time_usec();
    time_diff = end_time - start_time;

	printf("Transferred %d bytes by program control to BRAM: elapsed = %u microseconds\n", ntransfer, time_diff);

	return 0;

}


int test2( int ntransfer, int direction ) {

    int st;
    int datasize;
    int diffcnt;
    uint64_t start_time, end_time, time_diff;
    int mdirection;
  int dummy;

    printf("Test #2: transfer speed to BRAM by CDMA\n");

  st = ioctl( cdma_fd, CDMACDEV_IOC_SET_DATA_SIZE, ntransfer );
  if( st < 0 ) {
      printf("ioctl IOC_SET_DATA_SIZE failed\n");
  }

  st = ioctl( cdma_fd, CDMACDEV_IOC_GET_DATA_SIZE, & datasize );
  if( st < 0 ) {
      printf("ioctl IOC_GET_DATA_SIZE failed\n");
  } else {
      printf("ioctl IOC_GET_DATA_SIZE returned = 0x%x\n", datasize );
  }

  st = ioctl( cdma_fd, CDMACDEV_IOC_SET_DIRECTION, direction );
  if( st < 0 )
      printf("ioctl IOC_SET_DIRECTION failed\n" );

  st = ioctl( cdma_fd, CDMACDEV_IOC_GET_DIRECTION, & mdirection );
  if( st < 0 )
      printf("ioctl IOC_GET_DIRECTION failed\n" );
  else
      printf("ioctl IOC_GET_DIRECTION returned = %d\n", mdirection );

  st = ioctl( cdma_fd, CDMACDEV_IOC_FILL_BUFFER, & datasize );
  if( st < 0 )
      printf("ioctl IOC_FILL_BUFFER failed\n");

  start_time = get_posix_clock_time_usec();
  st = ioctl( cdma_fd, CDMACDEV_IOC_DO_TRANSFER, & dummy );
  end_time = get_posix_clock_time_usec();
  time_diff = end_time - start_time;

  if( st < 0 )
      printf("ioctl IOC_DO_TRANSFER failed\n");

  printf("Transferred %d bytes by CDMA to BRAM: elapsed = %u microseconds\n", datasize, time_diff );

  st = ioctl( cdma_fd, CDMACDEV_IOC_COMPARE_BUFFER, & diffcnt );
  printf("ioctl IOC_COMPARE_BUFFER returned = %d\n", diffcnt );

  return 0;

}

int test3( ) {

	int st;
	unsigned char * srcp, * dstp;
	int i, mb_sec;
	uint64_t start_time, end_time, time_diff;
	int diffcnt;

	unsigned char srcbuf[ BUFFER_SIZE ];
	unsigned char dstbuf[ BUFFER_SIZE ];

	printf("Test #3: transfer speed to BRAM by read/write\n");

	srcp = srcbuf;
	dstp = dstbuf;

	for( i = 0; i < BUFFER_SIZE; i ++ ) {

		* srcp ++ = ( unsigned char ) ( i & 0xff );
		* dstp ++ = '\0';

	}

    start_time = get_posix_clock_time_usec();
	st = write( cdma_fd, srcbuf, BUFFER_SIZE );
    end_time = get_posix_clock_time_usec();
    time_diff = end_time - start_time;
    mb_sec = ((1000000 / (double)time_diff) * ((double)BUFFER_SIZE)) / 1000000;


	printf("Transferred %d bytes using write function: elapsed = %u microseconds\n", st, time_diff );
    printf("Throughput: %d MB / sec \n", mb_sec);

    start_time = get_posix_clock_time_usec();
	st = read( cdma_fd, dstbuf, BUFFER_SIZE );
    end_time = get_posix_clock_time_usec();
    time_diff = end_time - start_time;
    mb_sec = ((1000000 / (double)time_diff) * ((double)BUFFER_SIZE)) / 1000000;


	printf("Transferred %d bytes using read function: elapsed = %u microseconds\n", st, time_diff);
    printf("Throughput: %d MB / sec \n", mb_sec);

	diffcnt = 0;
	srcp = srcbuf;
	dstp = dstbuf;

	for( i = 0; i < BUFFER_SIZE; i ++ ) {
		if( * srcp ++ != * dstp ++ ) diffcnt ++;
	}

	if( diffcnt ) printf("%d differences detected\n", diffcnt );
	else printf("No difference found\n");

	return 0;

}

int test4( int ntransfer ) {

	unsigned char * p_psmem;
	int st;
	unsigned char * p;
	int i;
	unsigned int dummy;
	uint64_t start_time, end_time, time_diff;
    int mb_sec;
	int diffcnt;

	printf("Test #4: transfer speed to BRAM by ioctl and mmap\n");

    st = ioctl( cdma_fd, CDMACDEV_IOC_SET_DEV_ADDR, BRAM_BASE_ADDR );
    if( st < 0 ) {
    	printf("ioctl IOC_SET_DEV_ADDR failed\n");
    }

    st = ioctl( cdma_fd, CDMACDEV_IOC_SET_CPU_ADDR, 0x0 );
    if( st < 0 ) {
    	printf("ioctl IOC_SET_CPU_ADDR failed\n");
    }

    st = ioctl( cdma_fd, CDMACDEV_IOC_SET_DATA_SIZE, ntransfer );
    if( st < 0 ) {
    	printf("ioctl IOC_SET_DATA_SIZE failed\n");
    }

    st = ioctl( cdma_fd, CDMACDEV_IOC_SET_DIRECTION, CDMACDEV_DIR_MEM_TO_DEV );
    if( st < 0 )
    	printf("ioctl IOC_SET_DIRECTION failed\n" );

    p_psmem = ( unsigned char * ) mmap( 0, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, cdma_fd, 0 );
    if( ! p_psmem ) {
    	printf("failed to map memory\n" );
    	return -1;
    }

    p = p_psmem;
    for( i = 0; i < BUFFER_SIZE; i ++ ) {
    	* p ++ = ( unsigned char ) ( i & 0xff );
    }

    start_time = get_posix_clock_time_usec();
    st = ioctl( cdma_fd, CDMACDEV_IOC_DO_TRANSFER, & dummy );
    end_time = get_posix_clock_time_usec();
    time_diff = end_time - start_time;
    mb_sec = ((1000000 / (double)time_diff) * ((double)BUFFER_SIZE)) / 1000000;


    if( st < 0 ) {
    	printf("ioctl IOC_DO_TRANSFER failed\n");
    	return st;
    }

	printf("Transferred %d bytes by CDMA to BRAM: elapsed = %u microseconds\n", ntransfer, time_diff);
    printf("Throughput: %d MB / sec \n", mb_sec);

    st = ioctl( cdma_fd, CDMACDEV_IOC_SET_DIRECTION, CDMACDEV_DIR_DEV_TO_MEM );
    if( st < 0 ) {
    	printf("ioctl IOC_SET_DIRECTION failed\n" );
    	return st;
    }

    p = p_psmem;
    for( i = 0; i < BUFFER_SIZE; i ++ ) {
    	* p ++ = '\0';
    }
    
    start_time = get_posix_clock_time_usec();
    st = ioctl( cdma_fd, CDMACDEV_IOC_DO_TRANSFER, & dummy );
    end_time = get_posix_clock_time_usec();
    time_diff = end_time - start_time;
    mb_sec = ((1000000 / (double)time_diff) * ((double)BUFFER_SIZE)) / 1000000;


    if( st < 0 ) {
    	printf("ioctl IOC_DO_TRANSFER failed\n");
    	return st;
    }

	printf("Transferred %d bytes by CDMA from BRAM: elapsed = %u microseconds\n", ntransfer, time);
    printf("Throughput: %d MB / sec \n", mb_sec);
    p = p_psmem;
    diffcnt = 0;
    for( i = 0; i < BUFFER_SIZE; i ++ ) {
    	if( * p ++ != ( unsigned char ) ( i & 0xff )) diffcnt ++;
    }

    if( diffcnt ) printf("%d difference detected\n", diffcnt );
    else printf("No difference found\n");

	return 0;
}

int main()
{
    int st;
    int devaddr, cpuaddr;

    printf("A test program for cdmacdev driver\n");

    bram_ptr = ( unsigned char * ) bram_init( BRAM_BASE_ADDR, BRAM_SIZE);
    if( bram_ptr == 0 ) {
    	printf("Failed to initialize BRAM service\n");
    	return st;
    }
    else {
        printf("Initialized BRAM service\n");
    }

    test1( );

    cdma_fd = open( "/dev/cdmach", O_RDWR );
    if( cdma_fd < 0 ) {
        printf("Failed to open /dev/cdmach\n" );
        return -1;
    }

    st = ioctl( cdma_fd, CDMACDEV_IOC_SET_DEV_ADDR, BRAM_BASE_ADDR );
    if( st < 0 ) {
    	printf("ioctl IOC_SET_DEV_ADDR failed\n");
    }

    st = ioctl( cdma_fd, CDMACDEV_IOC_GET_DEV_ADDR, & devaddr );
    if( st < 0 ) {
    	printf("ioctl IOC_GET_DEV_ADDR failed\n");
    } else {
    	printf("ioctl IOC_GET_DEV_ADDR returned = 0x%x\n", devaddr );
    }

    st = ioctl( cdma_fd, CDMACDEV_IOC_SET_CPU_ADDR, 0x0 );
    if( st < 0 ) {
    	printf("ioctl IOC_SET_CPU_ADDR failed\n");
    }

    st = ioctl( cdma_fd, CDMACDEV_IOC_GET_CPU_ADDR, & cpuaddr );
    if( st < 0 ) {
    	printf("ioctl IOC_GET_CPU_ADDR failed\n");
    } else {
    	printf("ioctl IOC_GET_CPU_ADDR returned = 0x%x\n", cpuaddr );
    }

    test2( 2048, CDMACDEV_DIR_MEM_TO_DEV );
    test3();
    test4( 2048 );

    close( cdma_fd );
    bram_exit( );

    return 0;

}

