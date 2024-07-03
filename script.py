import subprocess

# use dictionary to store the different parameters and decrease the loops as the block size goes up

block_sizes = [2**i for i in range(1, 19)]
loop_numbers = {2: 1000000, 4: 1000000, 8: 1000000, 16: 1000000, 32: 1000000, 64: 1000000, 128: 100000, 256: 100000, 512: 100000, 1024: 10000, 2048: 10000, 4096: 10000, 8192: 10000, 16384: 1000, 32768: 1000, 65536: 100, 131072: 100, 262144: 100}
fgpas = [1, 2]  # List of different number of FPGAs

# store the output in a dictionary with the key as the FPGA and block size


def main():
    for fpga in fgpas:
        with open(f'log/F{fpga}.log','w') as f:
            f.write(f"Block_Size, Speed, loops, Transfer Size\n")
            for block_size in block_sizes:
                command = f"./test_stand -c 7 -n payload.block_ram.MEM --block_size {block_size} -l {loop_numbers[block_size]} -f {fpga} > log/output_{fpga}_{block_size}.txt"
                subprocess.run(command, shell=True)
                f = open(f"output_{fpga}_{block_size}.txt", "r")
                output = f.readlines()
                f.close()
                #parse output to extract Speed = value
                speed = 0
                for line in output:
                    if "Average Speed = " in line:
                        speed = line.split()[-1]
                        break  
                for line in output:
                    if "Total Size of transfer = " in line:
                        transfer_size = (line.split()[-1]).split("B")[0]
                        break 
                f.write(f"{block_size}, {speed}, {loop_numbers[block_size]}, {transfer_size}\n")
                f.flush()
            f.close()

            
        


if __name__ == '__main__':   
    main()