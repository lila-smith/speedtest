#include "uhalspeedtest.hh"
/*
1048576 is the number of bytes in a megabyte
32 is the number of bits in a word
4 is the number of bytes in a word
1e6 is the number of microseconds in a second
*/

void test_print(std::chrono::time_point<std::chrono::high_resolution_clock> begin, uint64_t loops){
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count();
	double speed = 32.*loops*1e6 / (duration * 1048576.);
	cout << std::dec << loops << " reads done, speed = " << speed <<  " Mbps" << endl;
}

void test_print(std::chrono::time_point<std::chrono::high_resolution_clock> begin, uint64_t loops, uint32_t block_size){
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count();
  double speed = (32. * loops * block_size * 1e6) / (duration * 1048576.) ;
	cout << std::dec << loops << " reads done, speed = " << speed <<  " Mbps" << endl;
}

void test_summary(std::chrono::time_point<std::chrono::high_resolution_clock> begin, uint64_t loops, std::string reg){
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count();

  cout << endl << "Speed test: " << std::dec << loops << " write-reads of " << reg << endl;
  cout << duration << " us total, average : " << duration / loops << " us." << endl;

  double speed = 32.*loops*1e6/ (duration * 1048576.);
  double size = 4.*loops / 1048576.;
  double size_bits = size * 8;
  double time = duration / 1e6;
  cout << "Speed = " << speed << " Mbps" << endl;
  cout << "Total Size of transfer = " << size << " MB (" << size_bits <<  " Mb ) over a period of " << time << "s" << endl;

}

void test_summary(std::chrono::time_point<std::chrono::high_resolution_clock> begin, uint64_t loops, std::string reg, uint32_t block_size){
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count();

  cout << endl << "Speed test: " << std::dec << loops << " write-reads of " << reg << " with block size of " << std::dec << block_size << endl;
  cout << duration << " us total, average : " << duration / loops << " us." << endl;

  double speed = (32. * loops * block_size * 1e6) / (duration * 1048576.) ;
  double size = 4. * loops * block_size / 1048576.;
  double size_bits = size * 8;
  double time = duration / 1e6;

  cout << "Speed = " << speed << " Mbps" << endl;
  cout << "Total Size of transfer = " << size << " MB (" << size_bits << " Mb ) over a period of " << time  << "s" << endl;
}