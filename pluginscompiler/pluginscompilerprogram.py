import subprocess
from pathlib import Path
from datetime import datetime
import shutil
import re

# -----------------------------
# Current folder (script folder)
# -----------------------------
current_folder = Path(__file__).parent.resolve()
include_folder = current_folder / "include"  # local include folder
output_folder = current_folder / "compiledplugins"  # folder for DLLs
temp_folder = current_folder / "temp_cpp"  # temporary folder for modified cpp files

# Ensure output and temp folders exist
output_folder.mkdir(parents=True, exist_ok=True)
temp_folder.mkdir(parents=True, exist_ok=True)
print(f"Output folder: {output_folder} (created if it didn't exist)")
print(f"Temporary folder: {temp_folder} (used for preprocessing)")

# Collect all .cpp files in the current folder
cpp_files = list(current_folder.glob("*.cpp"))
if not cpp_files:
    print("No .cpp files found in the current folder!")
    exit(1)

print(f"Found {len(cpp_files)} C++ files to compile.")

# Function to preprocess include paths
def preprocess_cpp(src_path, dest_path):
    with open(src_path, "r", encoding="utf-8") as f:
        content = f.read()

    # Replace ../include/... with local include path
    content = re.sub(r'#include\s+[<"]\.\./include/(.*?)[">]',
                     r'#include "\1"', content)

    with open(dest_path, "w", encoding="utf-8") as f:
        f.write(content)

# Compile each .cpp separately
for cpp_file in cpp_files:
    # Generate a unique DLL name: <original_name>_<timestamp>.dll
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    dll_name = f"{cpp_file.stem}_{timestamp}.dll"
    output_dll = output_folder / dll_name

    # Preprocess the cpp file into temp folder
    temp_cpp_file = temp_folder / cpp_file.name
    preprocess_cpp(cpp_file, temp_cpp_file)

    print(f"\nCompiling {cpp_file.name} -> {dll_name}")

    # GCC/MinGW compile command
    compile_command = [
        "g++",
        "-shared",
        "-o", str(output_dll),           # output DLL
        str(temp_cpp_file),              # preprocessed source file
        f"-I{include_folder}",           # include folder
    ]

    # Run compiler
    result = subprocess.run(compile_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

    # Print compiler output
    print("STDOUT:\n", result.stdout)
    print("STDERR:\n", result.stderr)

    if result.returncode == 0:
        print(f"Successfully compiled: {output_dll}")
    else:
        print(f"Failed to compile: {cpp_file.name}")

# Optional: clean up temp folder
# shutil.rmtree(temp_folder)
