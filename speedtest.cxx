
#include "uioLabelFinder.hh"
#include "speedtest.hh"




//g++ -std=c++11 -Wall -rdynamic -lboost_system -lboost_filesystem speedtest.cxx -o speedtest

void output_test(double time_ms, uint32_t length){
  // Prints tests to cmd  
	// writing a single word so 4 bytes
	double bytes =  (double) length*WORD;
	std::cout << "Number of words: " << length << "(%d)" <<bytes <<std::endl;
	double bps =  bytes / (time_ms);
	std::cout << "Duration: " << time_ms << std::endl;
	std::cout << "Bytes per second: " << bps << "\n" <<std::endl;
}

void read_test(uint32_t length,uint32_t address, uint32_t * ptr){
  	// variable to assign read output
  	std::cout << "Running read test";
  	uint32_t read = 0;
  	auto start = std::chrono::high_resolution_clock::now();

	//Speed test loop
	for(uint32_t i = 0;i<length;i++){ 
		read = ptr[address];
	}

  	auto stop = std::chrono::high_resolution_clock::now();
  	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
  	double time_ms =std::chrono::duration<double>(duration).count();
	std::cout << "Time taken by loop: " << duration.count() << " milliseconds" << std::endl;
  	output_test(time_ms,length);
}

void write_test(uint32_t length, uint32_t address, uint32_t * ptr){
  // variable to assign read output
  std::cout << "Running write test";
  auto start = std::chrono::high_resolution_clock::now();

	//Speed test loop
	for(uint32_t i = 0;i<length;i++){
		ptr[address] = i;
	}
  
  	auto stop = std::chrono::high_resolution_clock::now();
  	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
  	double time_ms =std::chrono::duration<double>(duration).count();
	std::cout << "Time taken by loop: " << duration.count() << " milliseconds" << std::endl;
  	output_test(time_ms,length);
}

void write_read_test(uint32_t length, uint32_t address, uint32_t * ptr){
  // variable to assign read output
  std::cout << "Running write test";
  uint32_t read = 0;
  auto start = std::chrono::high_resolution_clock::now();

	//Speed test loop
	for(uint32_t i = 0;i<length;i++){
		ptr[address] = i;
		read = ptr[address];
	}
  
  	auto stop = std::chrono::high_resolution_clock::now();
  	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
  	double time_ms =std::chrono::duration<double>(duration).count();
	std::cout << "Time taken by loop: " << duration.count() << " milliseconds" << std::endl;
  	output_test(time_ms,length);
}

void write_read_error_test(uint32_t length, uint32_t address, uint32_t * ptr){
  // variable to assign read output
  std::cout << "Running write test";
  uint32_t read = 0;
  //error counter
  int error = 0;

  auto start = std::chrono::high_resolution_clock::now();

	//Speed test loop
	for(uint32_t i = 0;i<length;i++){
		ptr[address] = i;
		if(ptr[address] != i){
			error++;
		}
	}
  	std::cout << "Errors: " << error << std::endl;
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	double time_ms =std::chrono::duration<double>(duration).count();
	std::cout << "Time taken by loop: " << duration.count() << " milliseconds" << std::endl;
	output_test(time_ms,length);
}

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

	uint32_t * ptr = (uint32_t *) mmap(NULL,sizeof(uint32_t)*(address+count), PROT_READ|PROT_WRITE, MAP_SHARED, fdUIO,0x0);
	
	//length of loop
	uint32_t length = 100000000;

  read_test(length,address,ptr);
  write_test(length,address,ptr);
  write_read_test(length,address,ptr);
  write_read_error_test(length,address,ptr);
	
	return 0;
}

