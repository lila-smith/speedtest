import subprocess

block_sizes = [5, 10, 15, 20]  # List of different block sizes
fgpas = [1, 2]  # List of different number of FPGAs

def main():
    for fpga in fgpas:
        for block_size in block_sizes:
            command = f"./test_stand -c 7 -n payload.block_ram.MEM --block_size {block_size} -l 10000 -f {fpga}"
            subprocess.run(command, shell=True)

if __name__ == '__main__':   
    main()