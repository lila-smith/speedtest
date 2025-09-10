#include <stdio.h>
#include "plmem_service.h"

static struct plmem_context bram_context;

void * bram_init( unsigned int base_addr, unsigned int size ) {

	return plmem_init( & bram_context, base_addr, size );

}

void bram_exit( ) {

	plmem_exit( & bram_context );

}

void * bram_pointer( ) {

	return plmem_pointer( & bram_context );

}

int bram_write( unsigned char * buf, unsigned int offset, unsigned int size ) {

	unsigned char * src, * dst;
	int i;

	src = buf;
	dst = bram_pointer( ) + offset;
	for( i = 0; i < size; i ++ ) * dst ++ = * src ++;

	return size;

}

int bram_read( unsigned char * buf, unsigned int offset, unsigned int size ) {

	unsigned char * src, * dst;
	int i;

	src = bram_pointer( ) + offset;
	dst = buf;
	for( i = 0; i < size; i ++ ) * dst ++ = * src ++;

	return size;

}

void bram_dump( unsigned int offset, unsigned int size ) {

	unsigned char * p;
	int i;
	unsigned long physaddr;

	physaddr = bram_context.m_physaddr + offset;

	p = bram_pointer( ) + offset;
	for( i = 0; i < size; i ++ ) {
		if( i % 16 == 0 ) {
			printf("0x%08lx:", physaddr );
		}
		printf(" %02x", * p ++ );
		physaddr ++;
		if( i % 16 == 15 ) printf("\n");
	}
	if( i % 16 != 0 ) printf("\n");

}