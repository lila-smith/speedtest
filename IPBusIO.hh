#ifndef __IPBUS_IO_HH__
#define __IPBUS_IO_HH__

#include <uhal/uhal.hpp>

#if UHAL_VER_MAJOR >= 2 && UHAL_VER_MINOR >= 8
#include <unordered_map>
typedef std::unordered_map<std::string, std::string> uMap;
#else
#include <boost/unordered_map.hpp>
typedef boost::unordered_map<std::string, std::string> uMap;
#endif

#include <vector>
#include <string>
#include <stdint.h>

#include <RegisterHelper/RegisterHelperIO.hh>

//This class is an interface between the fundamental software library and what we want to use in our hardware class and the butool
//Right now this uses a dumb block/fifo read instead of the efficient calls built into IPBus.  
//These should be overloaded eventually and done so to work with the API from the BUTool Register Helper
class IPBusIO : public BUTool::RegisterHelperIO {
public:
  IPBusIO(std::shared_ptr<uhal::HwInterface> hw);
  virtual ~IPBusIO(){};

  // Register search
  virtual std::vector<std::string> GetRegsRegex(std::string regex);

  //reads
  virtual uint32_t    ReadAddress       (uint32_t addr);
  virtual uint32_t    ReadRegister      (std::string const & reg);
  virtual std::string ReadString        (std::string const & reg);
  uint32_t ReadNode                     (uhal::Node const & node);

  
  //Misc markups
  virtual uint32_t    GetRegAddress     (std::string const & reg);
  virtual uint32_t    GetRegMask        (std::string const & reg);
  virtual uint32_t    GetRegSize        (std::string const & reg);
  virtual std::string GetRegMode        (std::string const & reg);
  virtual std::string GetRegPermissions (std::string const & reg);
  virtual std::string GetRegDescription (std::string const & reg);
  virtual std::string GetRegDebug       (std::string const & reg);  
  virtual std::unordered_map<std::string,std::string> const & GetRegParameters(std::string const & reg);
  virtual std::string GetRegParameterValue(std::string const & reg, std::string const & name);
  const uMap &        GetParameters     (std::string const & reg);

  
  //numeric, named register, action, and node writes
  virtual void WriteAddress             (uint32_t addr, uint32_t data);
  virtual void WriteRegister            (std::string const & reg, uint32_t data);
  virtual void WriteAction              (std::string const & reg);
  virtual void WriteNode                (uhal::Node const & node, uint32_t data);
			                
  uhal::Node const & GetNode            (std::string const & reg);

  // Helper function to return list of register names with a specified parameter
  std::vector<std::string> GetRegisterNamesFromTable(std::string const & tableName, int statusLevel=1);

protected:
  void SetHWInterface(std::shared_ptr<uhal::HwInterface> _hw);

private:
  IPBusIO(); //do not implement
  //This is a const pointer to a pointer to a HWInterface so that the connection class controls it.
  std::shared_ptr<uhal::HwInterface> hw;
};
#endif
