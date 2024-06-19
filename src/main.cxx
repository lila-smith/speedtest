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
	string connections_file;
	string node;
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help,h", "produce help message")
		("command,c", po::value<int>(&cmd), "select command to run")
		("stop,s", po::value<bool>(&stop), "set to false to let it run until SIGINT")
		("list_commands,l", "list available commands")
		("connections_file,a", po::value<string>(&connections_file)->default_value("/opt/address_table/connections.xml"), "full path to connections file")
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
		cout << "   cmd = 6 EMP (ICRead/write) speedtest" << endl;
		return 1;
	}

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

	SPEED_TEST* t = new SPEED_TEST();
	t->SM = sm;

	uint64_t loops = 0;

	if(stop){
		loops = 1000000;
	}

	switch(cmd) {
	case 1:
		t->uhalspeedtest(node,loops);
		break;
	case 2:
		t->AXI_C2C_loop_back_test(node,loops);
		break;
	case 3:
		t->uio_direct(node,loops);
		break;
	case 4:
		t->uio_direct_mock_map(node,loops);
		break;
	case 5:
		t->uio_direct_sigbus(node,loops);
		break;
	case 6:
		t->empSpeedTest(node,loops);
		break;
	default:
		cout << "Invalid command = " << cmd << ", try again" << endl;
		cout << "Enter './test_stand -l' for a list of commands" << endl;
		return -1;
	}

	return 0;
}
