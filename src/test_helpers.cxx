#include "uhalspeedtest.hh"
/*
1048576 is the number of bytes in a megabyte
32 is the number of bits in a word. Speed is in Mbps
4 is the number of bytes in a word. Size in in MB
1e6 is the number of microseconds in a second
*/

void test_print(std::chrono::time_point<std::chrono::high_resolution_clock> begin, TestInfo testInfo){
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count();
	double speed = 32.*testInfo.loops*1e6 / (testInfo.duration * 1048576.);
	cout << std::dec << testInfo.loops << " reads done, speed = " << speed <<  " Mbps" << endl;
}

void test_print_b(std::chrono::time_point<std::chrono::high_resolution_clock> begin, TestInfo testInfo){
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count();
  double speed = (32. * testInfo.loops * testInfo.block_size * 1e6) / (duration * 1048576.) ;
  std::string test_type = testInfo.write_only ? "writes" : "reads";
	cout << std::dec << testInfo.loops << test_type + " done, speed = " << speed <<  " Mbps" << endl;
}

void test_summary(std::chrono::time_point<std::chrono::high_resolution_clock> begin, TestInfo testInfo){
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count();

  cout << endl << "Speed test: " << std::dec << loops << " write-reads of " << reg << endl;
  cout << duration << " us total, average : " << duration / loops << " us." << endl;

  double speed = 32.*testInfo.loops*1e6/ (duration * 1048576.);
  double size = 4.*testInfo.loops / 1048576.;
  double size_bits = size * 8;
  double time = duration / 1e6;
  cout << "Average Speed = " << speed << " Mbps" << endl;
  cout << "Total Size of transfer = " << size << " MB (" << size_bits <<  " Mb ) over a period of " << time << "s" << endl;

}

void test_summary_b(std::chrono::time_point<std::chrono::high_resolution_clock> begin, TestInfo testInfo){
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count();
  std::string test_type = testInfo.write_only ? "writes" : "write-reads";

  cout << endl << "Speed test: " << std::dec << testInfo.loops << " " + test_type + " of " << testInfo.reg << " with block size of " << std::dec << testInfo.block_size << endl;
  cout << duration << " us total, average : " << duration / testInfo.loops << " us." << endl;

  double speed = (32. * testInfo.loops * testInfo.block_size * 1e6) / (duration * 1048576.) ;
  double size = 4. * testInfo.loops * testInfoblock_size / 1048576.;
  double size_bits = size * 8;
  double time = duration / 1e6;

  cout << "Average Speed = " << speed << " Mbps" << endl;
  cout << "Total Size of transfer = " << size << " MB (" << size_bits << " Mb ) over a period of " << time  << "s" << endl;
}