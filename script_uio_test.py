import subprocess

# use dictionary to store the different parameters and decrease the loops as the block size goes up

BLOCK_SIZES = [2**i for i in range(3, 19)]
LOOP_NUMBERS = {8: 8192000, 16: 4096000, 32: 2048000, 64: 1024000, 128: 512000, 256: 256000, 512: 128000, 1024: 64000, 2048: 32000, 4096: 16000, 8192: 8000, 16384: 4000, 32768: 2000, 65536: 1000, 131072: 500, 262144: 250}
def generate_loop_numbers(max_key):
    loop_numbers = {}
    for i in range(3, max_key+1):
        loop_numbers[2**i] = 2**(23-i)
    return loop_numbers
# fgpas = [1, 2]  # List of different number of FPGAs
words = [i for i in range(16)]  # List of different BRAMs


def main():
    with open(f'PL_MEM_DMA_direct.log','w+') as f:
        f.write(f"Block_Size, Speed, loops, Transfer Size\n")
        command = f"./test_stand -c 9 -n PL_MEM.SCRATCH.WORD_01 --block_size 4 -l 1000 -w true > output_PL_MEM_DMA.txt"
        subprocess.run(command, shell=True)
        log = open(f"output_PL_MEM_DMA.txt", "r", encoding='utf-8', errors='ignore')
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
        print ("Max block size:", max_block_size)
        for block_size in BLOCK_SIZES:
            # print(len(loop_numbers), loop_numbers.keys())
            command = f"./test_stand -c 9 -n PL_MEM.SCRATCH.WORD_01 --block_size {block_size} -l {LOOP_NUMBERS[block_size]} -w true > output_PL_MEM_DMA.txt"
            subprocess.run(command, shell=True)
            log = open("output_PL_MEM_DMA.txt", "r", encoding='utf-8', errors='ignore')
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
            f.write(f"{block_size}, {speed}, {LOOP_NUMBERS[block_size]}, {transfer_size}\n")
            f.flush()
        f.close()

            
if __name__ == '__main__':   
    main()