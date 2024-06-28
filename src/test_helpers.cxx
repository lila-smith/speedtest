#include "uhalspeedtest.hh"

void test_print(std::chrono::time_point<std::chrono::high_resolution_clock> begin, uint64_t loops){
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count();
	double speed = 2.*32.*loops/duration;
	cout << std::dec << loops << " reads done, speed = " << speed <<  " Mbps" << endl;
}

void test_print(std::chrono::time_point<std::chrono::high_resolution_clock> begin, uint64_t loops, uint32_t block_size){
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count();
	double speed = 2.*32.*loops*block_size/duration;
	cout << std::dec << loops << " reads done, speed = " << speed <<  " Mbps" << endl;
}

void test_summary(std::chrono::time_point<std::chrono::high_resolution_clock> begin, uint64_t loops, std::string reg){
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count();

  cout << endl << "Speed test: " << std::dec << loops << " write-reads of " << reg << endl;
  cout << duration << " us total, average : " << duration / loops << " us." << endl;

  double speed = 2.*32.*loops/duration;
  cout << "Speed = " << speed << " Mbps" << endl;
}

void test_summary(std::chrono::time_point<std::chrono::high_resolution_clock> begin, uint64_t loops, std::string reg, uint32_t block_size){
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count();

  cout << endl << "Speed test: " << std::dec << loops << " write-reads of " << reg << "with block size of " << std::dec << block_size << endl;
  cout << duration << " us total, average : " << duration / loops << " us." << endl;

  double speed = 2.*32.*loops*block_size/duration;
  cout << "Speed = " << speed << " Mbps" << endl;
}