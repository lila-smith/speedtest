# Apollo SM write/read speed tests
Speed testing the apollo service module by reading and writing to scratch

## Build/Installation
```
git clone git@github.com:ablaizot/speedtest.git
make
```
## Running
```
Allowed options:
  -h [ --help ]                         produce help message
  -c [ --command ] arg                  select command to run
  -s [ --stop ] arg                     set to false to let it run until SIGINT
  -l [ --list_commands ]                list available commands
  -a [ --connections_file ] arg (=/opt/address_table/connections.xml)
                                        full path to connections file
  -n [ --node ] arg (=PL_MEM.SCRATCH.WORD_00)
                                        node for speedtests

Syntax: ./test_stand -c cmd
   cmd = 1 uhal speedtest
   cmd = 2 run AXI C2C read/write test
   cmd = 3 UIO Direct speedtest
```
## Expected Output

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