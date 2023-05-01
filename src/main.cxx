#include "uhalspeedtest.hh"
#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, char* argv[])
{
  int cmd;
  string connections_file;
  string node;
  // Declare the supported options.
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help,h", "produce help message")
    ("command,c", po::value<int>(&cmd), "select command to run")
    ("list_commands,l", "list available commands")
    ("connections_file,a", po::value<string>(&connections_file)->default_value("/opt/address_table/connections.xml"), "full path to connections file")
    ("node,n", po::value<string>(&node)->default_value("PL_MEM.SCRATCH.WORD_00"), "node for AXI C2C R/W loop tests")
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
    cout << "   cmd = 1 run I2C read test on MCU" << endl;
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

  UHAL_TEST* t = new UHAL_TEST();
  t->SM = sm;
  
  uint32_t loops;
  
  switch(cmd) {
  case 1:
    loops = 1000000;
    t->uhal_speedtest(node,loops);
    break;
  default:
    cout << "Invalid command = " << cmd << ", try again" << endl;
    cout << "Enter './uhal_speedtest -l' for a list of commands" << endl;
    return -1;
  }

  return 0;
}
