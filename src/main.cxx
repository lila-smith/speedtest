#include "uhalspeedtest.hh"
#include <boost/program_options.hpp>
#include<signal.h>
#include<unistd.h>
#define UNUSED(signum) (void)(signum)
namespace po = boost::program_options;

namespace GlobalVars {
  bool running = true;
  std::string logFileName = "Invalid.log";
}

void sig_handler(int signum){
	UNUSED(signum);
	GlobalVars::running = false;
}

int main(int argc, char* argv[])
{
	signal(SIGINT,sig_handler);
	TestInfo testInfo;
	int cmd;
	bool stop;
	int fpga;
	string uio_address_str;
	string connections_file;

	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help,h", "produce help message")
		("command,c", po::value<int>(&cmd), "select command to run")
		("stop,s", po::value<bool>(&stop)->default_value(true), "set to false to let it run until SIGINT")
		("list_commands,i", "list available commands")
		("connections_file,a", po::value<string>(&connections_file)->default_value("/opt/address_table/connections.xml"), "full path to connections file")
		("emp_connections_file,b", po::value<string>(&testInfo.emp_connections_file)->default_value("/opt/address_table/emp_connections.xml"), "full path to connections file")
		("fpga,f", po::value<int>(&fpga)->default_value(1), "fpga number")
		("block_size,k", po::value<size_t>(&testInfo.block_size)->default_value(100), "block size for block read/write speedtest")
		("loops,l", po::value<uint64_t>(&testInfo.loops)->default_value(1000000), "number of loops for speedtest")
		("uio_address_str,u", po::value<string>(&uio_address_str)->default_value("0x000007F0"), "uio address from top node")
		("node,n", po::value<string>(&testInfo.reg)->default_value("PL_MEM.SCRATCH.WORD_00"), "node for speedtests")
		("write_only,w", po::value<bool>(&testInfo.write_only)->default_value(false), "set to true to only write to node")
		;
	
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);    

	if (vm.count("help")) {
		cout << desc << endl;
		return 1;
	}

	if (vm.count("list_commands")) {
		cout << "Syntax: ./test_stand -c cmd" << endl;    
		cout << "   cmd = 1 uhal (getNode) speedtest" << endl;
		cout << "   cmd = 2 run AXI C2C (getRegister) speedtest" << endl;
		cout << "   cmd = 3 UIO Direct speedtest" << endl; 
		cout << "   cmd = 4 UIO Direct mock map speedtest" << endl;
		cout << "   cmd = 5 UIO Direct bus error speedtest" << endl;
		cout << "   cmd = 6 EMP (Single Read/write) speedtest" << endl;
		cout << "   cmd = 7 EMP (Block Read/write) speedtest" << endl;
		return 1;
	}
	testInfo.uio_address = std::stoul(uio_address_str, nullptr, 16);
	ApolloSM * sm = NULL;  
	vector<std::string> arg;
	arg.push_back(connections_file);
	sm = new ApolloSM(arg);

	if(NULL == sm) {
		printf("Failed to create new ApolloSM\n");
		return -1;
	}
	else{
		printf("Created new ApolloSM\n");      
	}

	emp::SPEED_TEST* t = new emp::SPEED_TEST();
	t->SM = sm;
	
	testInfo.DeviceId = "F1_IPBUS";
	if(fpga == 2){
		testInfo.DeviceId = "F2_IPBUS";
	}

	if(!stop){
		testInfo.loops = 0;
	}

	t->testInfo = testInfo;
	//Create file for logging with type of test and date
	time_t now = time(0);
	tm *ltm = localtime(&now);
	std::string date = std::to_string(1 + ltm->tm_mon) + "_" + std::to_string(ltm->tm_mday) + "_" + std::to_string(1900 + ltm->tm_year);
	GlobalVars::logFileName = "Invalid" + date + ".log";

	switch(cmd) {
	case 1:
		GlobalVars::logFileName = "uhalWriteNode" + date + ".log";
		t->uhalWriteNode();
		break;
	case 2:
		GlobalVars::logFileName = "uhalWriteRegister" + date + ".log";
		t->uhalWriteRegister();
		break;
	case 3:
		GlobalVars::logFileName = "uio_direct" + date + ".log";
		t->uio_direct();
		break;
	case 4:
		GlobalVars::logFileName = "uio_direct_mock_map" + date + ".log";
		t->uio_direct_mock_map();
		break;
	case 5:
		GlobalVars::logFileName = "uio_direct_sigbus" + date + ".log";
		t->uio_direct_sigbus();
		break;
	case 6:
		GlobalVars::logFileName = "empSpeedTest" + date + ".log";
		t->empSpeedTest();
		break;
	case 7:
		GlobalVars::logFileName = "empSpeedTestBlock" + date + ".log";
		t->empSpeedTestBlock();
		break;
	default:
		cout << "Invalid command = " << cmd << ", try again" << endl;
		cout << "Enter './test_stand -l' for a list of commands" << endl;
		return -1;
	}
	return 0;
}
