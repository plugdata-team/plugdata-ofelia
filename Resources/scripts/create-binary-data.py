import os
import sys
import shutil
import zipfile

build_dir = sys.argv[1]

# Get the path for the output directory
output_dir = os.path.join(build_dir, 'src')

# Create the output directory if it doesn't exist
os.makedirs(output_dir, exist_ok=True)

# Zip the folder at the "./ofelia" path relative to the working directory
zip_filename = os.path.join(build_dir, 'ofelia.zip')
shutil.make_archive(os.path.join(build_dir, 'ofelia'), 'zip', "./ofelia")

# Open the zip file and read the binary data
with open(zip_filename, 'rb') as zip_file:
    data = zip_file.read()

num_chunks = 16
chunk_size = len(data) // num_chunks

# Split the data into chunks and write each chunk to a separate C++ source file in the build directory
for i in range(num_chunks):
    start_index = i * chunk_size
    end_index = start_index + chunk_size if i < num_chunks - 1 else len(data)
    chunk = data[start_index:end_index]
    
    cpp_file_path = os.path.join(output_dir, f'ofelia_binary_data_{i}.cpp')
    
    with open(cpp_file_path, 'w') as cpp_file:
        cpp_file.write('#include <vector>\n\n')
        cpp_file.write('std::vector<unsigned char> zip_data = {\n')
        
        for byte in chunk:
            cpp_file.write(str(byte) + ', ')
        
        cpp_file.write('\n};\n')
