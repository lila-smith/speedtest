#include "uhalspeedtest.hh"
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
#include <random>

#include <boost/filesystem.hpp>

using namespace boost::filesystem;

static size_t ReadFileToBuffer(std::string const & fileName,char * buffer,size_t bufferSize){
  //open the file
  FILE * inFile = fopen(fileName.c_str(),"r");
  if(NULL == inFile){
    //failed to opn file.
    return 0;
  }

  //read file  
  if(fgets(buffer,bufferSize,inFile) == NULL){
    fclose(inFile);
    return 0;
  }
  fclose(inFile);
  return 1;
}

uint64_t SearchDeviceTree(std::string const & dvtPath,std::string const & name){
  uint64_t address = 0;
  FILE *labelfile=0;
  char label[128];
  // traverse through the device-tree   
  for (directory_iterator x(dvtPath); x!=directory_iterator(); ++x){
    if (x->path().filename().native() != "label") { continue; }
    labelfile = fopen((x->path().native()).c_str(),"r");
    fgets(label,128,labelfile);
    fclose(labelfile);

    if(!strcmp(label, name.c_str())){
      //Get endpoint AXI address from path           
      // looks something like LABEL@DEADBEEFXX       
      std::string stringAddr=x->path().parent_path().native();

      //Check if we find the @          
      size_t addrStart = stringAddr.find("@");
      if(addrStart == std::string::npos){
	fprintf(stderr,"directory name %s has incorrect format. Missing \"@\" ",x->path().filename().native().c_str() );
	break; //expect the name to be in x@xxxxxxxx format for example myReg@0x41200000    
      }
      //Convert the found string into binary         
      if(addrStart+1 > stringAddr.size()){
	fprintf(stderr,"directory name %s has incorrect format. Missing size ", x->path().filename().native().c_str() );
	break; //expect the name to be in x@xxxxxxxx format for example myReg@0x41200000    

      }
      stringAddr = stringAddr.substr(addrStart+1);

      //Get the names's address from the path (in hex)            
      address = std::strtoull(stringAddr.c_str() , 0, 16);
      break;
    }
  }
  return address;
};
// A function that takes a uio label and returns the uio number
// DEPRECATED - with the kernel patch the uio name is set by the "linux,uio-name" device-tree property -> ex: "uio_K_C2C_PHY"
// /dev/uio_NAME is a symlink that points to the actual uioN device file:  /dev/uio_NAME -> /dev/uioN
int label2uio_old(std::string ilabel)
{
  size_t const bufferSize = 1024;
  char * buffer = new char[bufferSize];
  memset(buffer,0x0,bufferSize);

  bool foundValidMatch = false;
  uint64_t dtEntryAddr=0, uioEntryAddr=0;
  // search through the file system to see if there is a uio that matches the name
  std::string const uiopath = "/sys/class/uio/";
  std::string const dvtpath_7series = "/proc/device-tree/amba_pl/";
  std::string const dvtpath_USP = "/proc/device-tree/amba_pl@0";
  std::string dvtpath;

  (exists(dvtpath_USP)) ? dvtpath=dvtpath_USP : dvtpath=dvtpath_7series;

  // traverse through the device-tree
  for (directory_iterator itDir(dvtpath); itDir!=directory_iterator(); ++itDir){
    //We only want to open label files, so we search for "/path/to/"+"label", not the file itself
    if (!is_directory(itDir->path())) {
      //This is a file, so not what we want in our search
      continue;
    }
    
    if (!exists(itDir->path().native()+"/label")) {
      //This directory does not contain a file named "label"
      continue;
    }

    //path has a file named label in it.
    //open file and read its contents into buffer;
    if(!ReadFileToBuffer(itDir->path().native()+"/label",buffer,bufferSize)){
      //bad read
      continue;
    }else{
      dtEntryAddr=SearchDeviceTree(itDir->path().string(),ilabel);
      if(dtEntryAddr != 0){
	//we found the correct entry
	break;
      }
    }
  }

  //check if we found anything  
  //Check if we found a device with the correct name
  if(dtEntryAddr==0) {
    std::cout<<"Cannot find a device that matches label "<<(ilabel).c_str()<<" device not opened!" << std::endl;
    return -1;
  }


  // Traverse through the /sys/class/uio directory
  for (directory_iterator itDir(uiopath); itDir!=directory_iterator(); ++itDir){
    //same kind of search as above, just looking for a uio dir with maps/map0/addr and maps/map0/size
    if (!is_directory(itDir->path())) {
      continue;
    }
    if (!exists(itDir->path()/"maps/map0/addr")) {
      continue;
    }
    if (!exists(itDir->path()/"maps/map0/size")) {
      continue;
    }

    //process address of UIO entry
    if(!ReadFileToBuffer((itDir->path()/"maps/map0/addr").native(),buffer,bufferSize)){
      //bad read
      continue;
    }
    uioEntryAddr = std::strtoul(buffer, 0, 16);

    // see if the UIO address matches the device tree address
    if (dtEntryAddr == uioEntryAddr){
      if(!ReadFileToBuffer((itDir->path().native()+"/maps/map0/size"),buffer,bufferSize)){
	//bad read
	continue;
      }
      
      //the size was in number of bytes, convert into number of uint32
      //size_t size=std::strtoul( buffer, 0, 16)/4;  
      strcpy(buffer,itDir->path().filename().native().c_str());
      foundValidMatch = true;
      break;
    }
  }

  //Did we find a 
  if (!foundValidMatch){
    return -1;
  }
  
  char* endptr;
  int uionumber = strtol(buffer+3,&endptr,10);
  if (uionumber <0){
    return uionumber;
  }
  return uionumber;
}

// new, simple uionumber finder based on kernel patch
int label2uio(std::string ilabel)
{
  std::string prefix = "/dev/";
  std::string uioname = "uio_" + ilabel;
  std::string deviceFile;
  int uionumber;
  bool found = false;

  char* UIO_DEBUG = getenv("UIO_DEBUG");
  if (NULL != UIO_DEBUG) {
    printf("searching for /dev/uio_%s symlink\n", ilabel.c_str());
  }

  for (directory_iterator itUIO(prefix); itUIO != directory_iterator(); ++itUIO) {
    if ((is_directory(itUIO->path())) || (itUIO->path().string().find(uioname)==std::string::npos)) {
      continue;
    }
    else {
      // found /dev/uio_name, resolve symlink and get the UIO number
      if (is_symlink(itUIO->path())) {
        // deviceFile will be a string of form "uioN"
        deviceFile = read_symlink(itUIO->path()).string();
        if (NULL != UIO_DEBUG) {
          printf("resolved symlink: /dev/%s -> /dev/%s\n", uioname.c_str(), deviceFile.c_str());
        }
        found = true;
      }
      else {
        if (NULL != UIO_DEBUG) {
          printf("unable to resolve symlink /dev/%s -> /dev/uioN, using legacy method\n", uioname.c_str());
        }
        return -1;
      }
    }
  }
  if (!found) {
    if (NULL != UIO_DEBUG) {
      printf("unable find /dev/%s, using legacy method\n", uioname.c_str());
    }
    return -1;
  }

  // get the number from any digits after "uio"
  char* endptr;
  uionumber = strtol(deviceFile.substr(3,std::string::npos).c_str(), &endptr, 10);
  if (uionumber < 0) {
    return uionumber;
  }
  return uionumber;
}

int SPEED_TEST::uio_direct(string reg, uint64_t loops)
{
    uint32_t write_mem;
    uint32_t read_mem;
    double speed;

    uint32_t address= 0x000007F0;
    uint32_t count = 1;
    char* UIO_DEBUG = getenv("UIO_DEBUG");

    size_t delim = reg.find('.');
    std::string device_name = reg.substr(0, delim);

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

    uint32_t * ptr = (uint32_t *) mmap(NULL,sizeof(uint64_t)*(address+count), PROT_READ|PROT_WRITE, MAP_SHARED, fdUIO,0x0);

    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<unsigned int> distrib(0, 0xFFFFFFFF);

    auto begin = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count();
        
    cout << endl << "UIO Direct Speedtest" << endl 
        << std::dec << loops << " loops doing write-read of incrementing 32-bit words to " << reg 
            << endl << endl; 
    if(loops != 0){
      for(uint32_t i = 0; i < loops; ++i) {        
          
        write_mem = distrib(gen);
        ptr[address] = write_mem;
        read_mem = ptr[address];
            
        if (write_mem != read_mem) {
          cout << "R/W error: loop " << i << ", write_mem = " << std::hex << write_mem 
              << ", read_mem = " << read_mem << endl << endl;
          return -1;
        }
        
        if (i < 10) {
          cout << "write_mem = " << std::hex << write_mem << ", read_mem = " << read_mem << endl;
        }
        
        if (i%100000 == 0 && i != 0) {
          end = std::chrono::high_resolution_clock::now();
          duration = std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count();
          speed = 2.*32.*i/duration;
          cout << std::dec << i << " reads done, speed = " << speed <<  " Mbps" << endl;
        }

      }
    }else{
      uint32_t i = 0;
      // infinite loop to end by sigint
      while(GlobalVars::running){
        write_mem = distrib(gen);
        ptr[address] = write_mem;
        read_mem = ptr[address];
        
        if (write_mem != read_mem) {
          cout << "R/W error: loop " << i << ", write_mem = " << std::hex << write_mem 
              << ", read_mem = " << read_mem << endl << endl;
          return -1;
        }
        
        if (i < 10) {
          cout << "write_mem = " << std::hex << write_mem << ", read_mem = " << read_mem << endl;
        }
        
        if (i%100000 == 0 && i != 0) {
          end = std::chrono::high_resolution_clock::now();
          duration = std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count();
          speed = 2.*32.*i/duration;
          cout << std::dec << i << " reads done, speed = " << speed <<  " Mbps" << endl;
        }
        i++;
      }
      loops = i;
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count();

    cout << endl << "Speed test: " << std::dec << loops << " write-reads of " << reg << endl;
    cout << duration << " us total, average : " << duration / loops << " us." << endl;

    speed = 2.*32.*loops/duration;
    cout << "Speed = " << speed << " Mbps" << endl;

    return 0;
}

