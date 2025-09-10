#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "plmem_service.h"

static int plmem_fd = 0;
static int plmem_clients = 0;

void * plmem_init( struct plmem_context * pcon, unsigned long physaddr, unsigned long size ) {

	unsigned int page_size = sysconf( _SC_PAGESIZE );

	if( ! plmem_fd ) {
		plmem_fd = open( "/dev/mem", O_RDWR );
		if( plmem_fd < 1 ) {
			printf("Error opening /dev/mem\n" );
			return 0;
		}
		else {
			printf("Success opening /dev/mem\n" );
		}
	}

	pcon -> m_physaddr = ( physaddr & ~ ( page_size - 1 ));
	pcon -> m_offset = physaddr - pcon -> m_physaddr;
	pcon -> m_size = size;
	pcon -> p_plmem_fd = & plmem_fd;

	pcon -> p_base = mmap( NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, plmem_fd, pcon -> m_physaddr );
	if( ( int ) pcon -> p_base == -1 ) {
		printf("Failed to map physical address 0x%08lx\n", physaddr );
		return 0;
	}else {
			printf("Success map physical address 0x%08lx\n", physaddr );
		}
	
	plmem_clients ++;

	return pcon -> p_base + pcon -> m_offset;

}

void plmem_exit( struct plmem_context * pcon ) {

	munmap( pcon -> p_base, pcon -> m_size );
	if( plmem_clients > 0 ) {
		plmem_clients --;
		if( plmem_clients == 0 ) {
			close( plmem_fd );
			plmem_fd = 0;
		}
	}

}

inline void * plmem_pointer( struct plmem_context * pcon ) {

	return pcon -> p_base;

}