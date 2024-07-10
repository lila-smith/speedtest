import subprocess

# use dictionary to store the different parameters and decrease the loops as the block size goes up

block_sizes = [2**i for i in range(3, 19)]
loop_numbers = {8: 8192000, 16: 4096000, 32: 2048000, 64: 1024000, 128: 512000, 256: 256000, 512: 128000, 1024: 64000, 2048: 32000, 4096: 16000, 8192: 8000, 16384: 4000, 32768: 2000, 65536: 1000, 131072: 500, 262144: 250}
def generate_loop_numbers(max_key):
    loop_numbers = {}
    for i in range(8, max_key+1):
        loop_numbers[2**i] = 2**(23-i)
    return loop_numbers
fgpas = [1, 2]  # List of different number of FPGAs
brams = [1,2]  # List of different BRAMs


def main():
    for fpga in fgpas:
        for bram in brams:
            with open(f'log/F{fpga}_bram{bram}.log','w') as f:
                f.write(f"Block_Size, Speed, loops, Transfer Size\n")
                command = f"./test_stand -c 7 -n payload.block_ram{bram}.MEM --block_size 4 -l 1000 -f {fpga} -w true > log/output_f{fpga}_bram{bram}.txt"
                subprocess.run(command, shell=True)
                log = open(f"log/output_f{fpga}_bram{bram}.txt", "r")
                output = log.readlines()
                log.close()
                max_block_size = 1024
                for line in output:
                    if "Depth of Block RAM:" in line:
                        # Assuming the format is "Depth of Block RAM: 65536 32 bit words"
                        # Split the line into words and select the depth value
                        parts = line.split()
                        max_block_size = parts[4]
                        break  # Assuming there's only one such line, exit the loop after finding it

                loop_numbers = generate_loop_numbers(int(max_block_size))
                print (loop_numbers)
                for block_size in block_sizes:

                    command = f"./test_stand -c 7 -n payload.block_ram{bram}.MEM --block_size {block_size} -l {loop_numbers[block_size]} -f {fpga} -w true > log/output_f{fpga}_bram{bram}_{block_size}.txt"
                    subprocess.run(command, shell=True)
                    log = open(f"log/output_f{fpga}_bram{bram}_{block_size}.txt", "r")
                    output = log.readlines()
                    log.close()
                    #parse output to extract Speed = value
                    speed = 0
                    for line in output:
                        if "Average Speed = " in line:
                            speed = line.split()[3] + " Mpbs"
                            break  
                    for line in output:
                        if "Total Size of transfer = " in line:
                            transfer_size = line.split()[5] + " MB"
                            break 
                    f.write(f"{block_size}, {speed}, {loop_numbers[block_size]}, {transfer_size}\n")
                    f.flush()
                f.close()

            
if __name__ == '__main__':   
    main()