#ifndef __SPEED_TEST_HH__
#define __SPEED_TEST_HH__

#include <string>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <unistd.h>
#include <sys/mman.h>

#define WORD 4 


void output_test(double time_ms, uint32_t length);

void read_test(uint32_t length, uint32_t address, uint32_t * ptr);

void write_test(uint32_t length, uint32_t address, uint32_t * ptr);

void write_read_test(uint32_t length, uint32_t address, uint32_t * ptr);

void write_read_error_test(uint32_t length, uint32_t address, uint32_t * ptr);




#endif