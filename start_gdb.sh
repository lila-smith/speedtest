#!/usr/bin/env bash
git pull
make clean
make
./test_stand -c 6 -n ttc.master.common.ctrl -l 10000
./test_stand -c 7 -n payload.block_ram.MEM --block_size 10 -l 10000
python3 script.py
#gdb -x gdb_commands.txt --args ./test_stand -c 6 