#ifndef SRC_PLMEM_SERVICE_H_
#define SRC_PLMEM_SERVICE_H_

struct plmem_context {

	void * p_base;
	unsigned long m_physaddr;
	unsigned long m_size;
	unsigned long m_offset;
	int * p_plmem_fd;

};

void * plmem_init( struct plmem_context * pcon, unsigned long physaddr, unsigned long size );
void plmem_exit( struct plmem_context * pcon );
void * plmem_pointer( struct plmem_context * pcon );

#endif /* SRC_PLMEM_SERVICE_H_ */