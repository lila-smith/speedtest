#ifndef H_BRAM_SERVICE
#define H_BRAM_SERVICE

#ifdef __cplusplus
extern "C" {
#endif


void * bram_init( unsigned int base_addr, unsigned int size );
void bram_exit( );

void * bram_pointer( );

int bram_write( unsigned char * buf, unsigned int offset, unsigned int size );
int bram_read( unsigned char * buf, unsigned int offset, unsigned int size );

void bram_dump( unsigned int offset, unsigned int size );

#ifdef __cplusplus
}
#endif
#endif /* SRC_BRAM_SERVICE_H_ */