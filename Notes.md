# Useful Reading
I followed https://ipbus.web.cern.ch/doc/user/html/software/uhalQuickTutorial.html to implement writing through IPBus.

# Read and write commands

IPBus software documentation (uHal) for can be found here: https://ipbus.web.cern.ch/sw/release/2.7/api/html/d5/d69/classuhal_1_1_node.html#a81ce6469527c785375bf43f42d767d5b. 

The node is the hierarhcy by which we access the registers on the CM and SM.
The ones on the CM are not accessible by BUTool. Instead, we have to go through EMP to get to the registers.

The actual write is implemented here.
https://github.com/ipbus/ipbus-software/blob/master/uhal/uhal/src/common/ProtocolIPbusCore.cpp#L319

