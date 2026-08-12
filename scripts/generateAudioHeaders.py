import sys

def binary_to_header(input_file, output_file, array_name):
    with open(input_file, 'rb') as f:
        data = f.read()
    
    with open(output_file, 'w') as f:
        f.write(f'unsigned char {array_name}[] = {{\n')
        for i in range(0, len(data), 12):
            chunk = data[i:i+12]
            hex_values = ', '.join(f'0x{b:02x}' for b in chunk)
            f.write(f'    {hex_values},\n')
        f.write(f'}};\n')
        f.write(f'unsigned int {array_name}_len = {len(data)};\n')

if __name__ == "__main__":
    binary_to_header("media/success.mp3", "include/success_audio.h", "success_mp3")
    binary_to_header("media/failure.mp3", "include/failure_audio.h", "failure_mp3")