#include "uhalspeedtest.hh"
#include <boost/program_options.hpp>
#include<signal.h>
#include<unistd.h>
#define UNUSED(signum) (void)(signum)
namespace po = boost::program_options;

namespace GlobalVars {
  bool running = true;
}

void sig_handler(int signum){
	UNUSED(signum);
	GlobalVars::running = false;
}

int main(int argc, char* argv[])
{
	signal(SIGINT,sig_handler);
	
	int cmd;
	bool stop = true;
	string uio_address_str;
	uint32_t uio_address;
	string connections_file;
	string emp_connections_file;
	string node;
	size_t block_size;
	uint64_t loops = 0;

	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help,h", "produce help message")
		("command,c", po::value<int>(&cmd), "select command to run")
		("stop,s", po::value<bool>(&stop), "set to false to let it run until SIGINT")
		("list_commands,i", "list available commands")
		("connections_file,a", po::value<string>(&connections_file)->default_value("/opt/address_table/connections.xml"), "full path to connections file")
		("emp_connections_file,b", po::value<string>(&emp_connections_file)->default_value("/opt/address_table/emp_connections.xml"), "full path to connections file")
		("block_size,k", po::value<size_t>(&block_size)->default_value(100), "block size for block read/write speedtest")
		("loops,l", po::value<uint64_t>(&loops)->default_value(1000000), "number of loops for speedtest")
		("uio_address_str,u", po::value<string>(&uio_address_str)->default_value("0x000007F0"), "uio address from top node")
		("node,n", po::value<string>(&node)->default_value("PL_MEM.SCRATCH.WORD_00"), "node for speedtests")
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
	uio_address = std::stoul(uio_address_str, nullptr, 16);
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

	if(!stop){
		loops = 0;
	}

	switch(cmd) {
	case 1:
		t->uhalspeedtest(node,loops);
		break;
	case 2:
		t->AXI_C2C_loop_back_test(node,loops);
		break;
	case 3:
		t->uio_direct(node,loops,uio_address);
		break;
	case 4:
		t->uio_direct_mock_map(node,loops,uio_address);
		break;
	case 5:
		t->uio_direct_sigbus(node,loops,uio_address);
		break;
	case 6:
		t->empSpeedTest(node,loops, emp_connections_file);
		break;
	case 7:
		t->empSpeedTestBlock(node,loops,emp_connections_file, block_size);
		break;
	default:
		cout << "Invalid command = " << cmd << ", try again" << endl;
		cout << "Enter './test_stand -l' for a list of commands" << endl;
		return -1;
	}

	return 0;
}
