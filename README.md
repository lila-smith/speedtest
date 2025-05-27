# Apollo SM write/read speed tests
Speed testing the apollo service module by reading and writing to scratch.
The teststand test SM speed using getNode, readRegister, and UIO direct respectively.
The other speedtest uses UIO direct to test read and write/read.
## Build/Installation
To build the teststand, use:
```
git clone git@github.com:ablaizot/speedtest.git
make
```
The other speedtest compiles using 
```
g++ -std=c++11 -Wall -rdynamic -lboost_system -lboost_filesystem speedtest.cxx -o speedtest
```

## Command Module Tests

Powerup the command module using BUTool.
```
BUTool.exe -a --cmd cmpwrup
```
Unload the previous firmware if loaded and load in the new firmware. Load the latest tarball from the block_ram branch the it-dtc FW [repo](https://gitlab.cern.ch/cms-tracker-phase2-data-processing/BE_firmware/inner-tracker-dtc/inner-tracker-dtc/-/tree/block_ram?ref_type=heads) with the following commands. 

```
uio_send.py list-devices
uio_send.py unload-tar -u {uidF1}
uio_send.py unload-tar -u {uidF2}
uio_send.py program-fpga -f {F1_tarball}.tgz
uio_send.py program-fpga -f {F1_tarball}.tgz
```

## Running

### Allowed Commands

Syntax: ./test_stand -c cmd
   cmd = 1 uhal (getNode) speedtest
   cmd = 2 uhal (getRegister) speedtest"
   cmd = 3 UIO Direct speedtest
   cmd = 4 UIO Direct mock map speedtest
   cmd = 5 UIO Direct bus error speedtest
   cmd = 6 EMP (Single Read/write) speedtest
   cmd = 7 EMP (Block Read/write) speedtest

Example command

Run the test stand with the following command:
`./test_stand -c 7 -n payload.block_ram1.MEM --block_size 16384 -l 100`

The maximum block size is 262144, though some FW will have smaller BRAMS, allowing only for smaller transfers.

```
Allowed options:
  -h [ --help ]                         produce help message
  -c [ --command ] arg                  select command to run
  -s [ --stop ] arg (=1)                set to false to let it run until SIGINT
  -i [ --list_commands ]                list available commands
  -a [ --connections_file ] arg (=/opt/address_table/connections.xml)
                                        full path to connections file
  -b [ --emp_connections_file ] arg (=/opt/address_table/emp_connections.xml)
                                        full path to connections file
  -f [ --fpga ] arg (=1)                fpga number
  -k [ --block_size ] arg (=100)        block size for block read/write
                                        speedtest
  -l [ --loops ] arg (=1000000)         number of loops for speedtest
  -u [ --uio_address_str ] arg (=0x000007F0)
                                        uio address from top node
  -n [ --node ] arg (=PL_MEM.SCRATCH.WORD_00)
                                        node for speedtests
  -w [ --write_only ] arg (=0)          set to true to only write to node
```
## Sweep
Run `nohup python3 script.py &` to sweep the different BRAMS with different block sizes

## Bit Error Test

To run in the background until stopped:
`./test_stand -c 7 -n payload.block_ram1.MEM --block_size 65536 -s false > error_test.log &`

## Expected Output

### SM Speed Test
```
$ ./test_stand -c 2 -s false
Using .xml connection file...
Warning: Input is a connection file but no device entry specified, using default entry name: test.0
Created new ApolloSM

uhal speedtest
0 loops doing write-read of random 32-bit words to PL_MEM.SCRATCH.WORD_00

write_mem = 51cf271e, read_mem = 51cf271e
write_mem = 3194470d, read_mem = 3194470d
write_mem = 790ad471, read_mem = 790ad471
write_mem = df6bf17c, read_mem = df6bf17c
write_mem = bf720a67, read_mem = bf720a67
write_mem = f96a47e5, read_mem = f96a47e5
write_mem = a3d9cca1, read_mem = a3d9cca1
write_mem = 855183a7, read_mem = 855183a7
write_mem = 354610ef, read_mem = 354610ef
write_mem = 5fe28f62, read_mem = 5fe28f62
100000 reads done, speed = 2.18672 Mbps
200000 reads done, speed = 2.18243 Mbps
^C
Speed test: 219960 write-reads of PL_MEM.SCRATCH.WORD_00
6451885 us total, average : 29 us.
Speed = 2.18191 Mbps
```

### CM Speed Test

```
# ./test_stand -c 7 -n payload.block_ram1.MEM --block_size 65536 -l 100
Using .xml connection file...
Warning: Input is a connection file but no device entry specified, using default entry name: test.0
Created new ApolloSM

empSpeedTestBlock
100 loops doing write-read of incrementing 32-bit words to payload.block_ram1.MEM

Depth of Block RAM: 65536 32 bit words
Mode:  INCREMENTAL

write_mem = 374f3c6e, read_mem = 374f3c6e
write_mem = c9ff8e68, read_mem = c9ff8e68
write_mem = 29b6197d, read_mem = 29b6197d
write_mem = 20cb323, read_mem = 20cb323
write_mem = bcf251a3, read_mem = bcf251a3
write_mem = e999a5cc, read_mem = e999a5cc
write_mem = 2064ac64, read_mem = 2064ac64
write_mem = 2da01d42, read_mem = 2da01d42
write_mem = f27def20, read_mem = f27def20
write_mem = 88d40413, read_mem = 88d40413
  0x374f3c6e
  0xc9ff8e68
  0x29b6197d
  0x20cb323
  0xbcf251a3
  0xe999a5cc
  0x2064ac64
  0x2da01d42
  0xf27def20
  0x88d40413
  0x1150d8da
  0xcd7e29f1
10 reads done, speed = 7.26239 Mbps
20 reads done, speed = 7.62443 Mbps
30 reads done, speed = 7.75243 Mbps
40 reads done, speed = 7.81809 Mbps
50 reads done, speed = 7.85695 Mbps
60 reads done, speed = 7.88433 Mbps
70 reads done, speed = 7.90343 Mbps
80 reads done, speed = 7.91825 Mbps
90 reads done, speed = 7.92872 Mbps

Speed test: 100 write-reads of payload.block_ram1.MEM with block size of 65536
24952641 us total, average : 249526 us.
Average Speed = 8.01518 Mbps
Total Size of transfer = 25 MB (200 Mb ) over a period of 24.9526s
```