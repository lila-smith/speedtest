#include "uhalspeedtest.hh"
#include <chrono>
#include <random>

namespace emp {

// UHAL_REGISTER_DERIVED_NODE(TestNode);

// void TestNode::write(string reg, unsigned aData) const
// {
//   getNode(reg).write(aData);
//   getClient().dispatch();
// }

// unsigned TestNode::read(string reg) const
// {
//   unsigned aData = getNode(reg).read();
//   getClient().dispatch();
//   return aData;
// }


int SPEED_TEST::empSpeedTest(string reg, uint64_t loops)
{
  uint32_t write_mem;
  uhal::ValWord<uint32_t> read_mem;
  double speed;

  std::random_device rd;  //Will be used to obtain a seed for the random number engine
  std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<unsigned int> distrib(0, 0xFFFFFFFF);

  const std::string lConnectionFilePath = "/opt/address_table/emp_connection.xml";
  const std::string lDeviceId = "F1_IPBUS";
  const std::string lRegisterName = reg;

  auto begin = std::chrono::high_resolution_clock::now();
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count();

  cout << endl << "empSpeedTest" << endl 
       << std::dec << loops << " loops doing write-read of incrementing 32-bit words to " << lRegisterName 
	    << endl << endl;

  uhal::ConnectionManager lConnectionMgr("file://" + lConnectionFilePath);
  uhal::HwInterface lHW = lConnectionMgr.getDevice(lDeviceId);
  const uhal::Node& lNode = lHW.getNode(lRegisterName);
  //uhal::Node const & node = SM->GetNode(reg);
  //SCCICNode empNode = SCCICNode(node);
  //TestNode testNode = TestNode(getNode(reg));

  if(loops != 0){
      for(uint64_t i = 0; i < loops; ++i) {

      write_mem = distrib(gen);
      lNode.write(write_mem);
      lHW.dispatch();
      read_mem = lNode.read();
      lHW.dispatch();

      // write_mem = distrib(gen);
      // testNode.write(reg, write_mem);
      // read_mem = testNode.read(reg);

      // write_mem = distrib(gen);
      // SM->WriteNode(node,write_mem);
      // read_mem = SM->ReadNode(node);

      // write_mem = distrib(gen);
      // empNode.icWrite(0,write_mem,0);
      // read_mem = empNode.icRead(0,0);

      if (write_mem != read_mem.value()) {
        cout << "R/W error: loop " << i << ", write_mem = " << std::hex << write_mem 
      << ", read_mem = " << read_mem << endl << endl;
        return -1;
      }

      if (i < 10) {
        cout << "write_mem = " << std::hex << write_mem << ", read_mem = " << std::hex << read_mem.value() << endl;
      }
        
      if (i%100000 == 0 && i != 0) {
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count();
        speed = 2.*32.*i/duration;
        cout << std::dec << i << " reads done, speed = " << speed <<  " Mbps" << endl;
      }

    }
  }else{
  // infinite loop to end by sigint
    uint64_t i = 0;
    while(GlobalVars::running){

      write_mem = distrib(gen);
      lNode.write(write_mem);
      lHW.dispatch();
      read_mem = lNode.read();
      lHW.dispatch();

      // write_mem = distrib(gen);
      // testNode.write(reg, write_mem);
      // read_mem = testNode.read(reg);

      // write_mem = distrib(gen);
      // SM->WriteNode(node,write_mem);
      // read_mem = SM->ReadNode(node);

      if (write_mem != read_mem.value()) {
        cout << "R/W error: loop " << i << ", write_mem = " << std::hex << write_mem 
      << ", read_mem = " << read_mem << endl << endl;
        return -1;
      }

      if (i < 10) {
        cout << "write_mem = " << std::hex << write_mem << ", read_mem = " << std::hex << read_mem.value() << endl;
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
}

