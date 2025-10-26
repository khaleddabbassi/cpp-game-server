import json
import subprocess
from pathlib import Path

def build(config_file="build.json"):
    with open(config_file, "r") as f:
        config = json.load(f)

    compiler = config.get("compiler", "g++")

    # Only .cpp in current folder
    source_folder = Path(config.get("autoIncludeFolder", "."))
    cpp_files = [str(f) for f in source_folder.glob("*.cpp")]  # only current folder
    cpp_files += config.get("sourceFiles", [])  # manually listed files

    output_name = config.get("output", "program.exe")

    # Include paths
    include_paths = config.get("includePaths", [])
    include_flags = [f"-I{path}" for path in include_paths]

    # Library paths and libraries
    library_paths = config.get("libraryPaths", [])
    lib_flags = [f"-L{path}" for path in library_paths]

    libraries = config.get("libraries", [])
    link_flags = [f"-l{lib}" for lib in libraries]

    # Compiler flags
    compiler_flags = config.get("compilerFlags", [])

    # Print all relevant info
    print("\n--- BUILD INFO ---")
    print("Compiler:", compiler)
    print("Output:", output_name)
    print("Source files:", cpp_files)
    print("Include paths:", include_paths)
    print("Library paths:", library_paths)
    print("Libraries:", libraries)
    print("Compiler flags:", compiler_flags)
    print("--- END OF INFO ---\n")

    # Final compile command
    compile_cmd = [compiler] + cpp_files + include_flags + lib_flags + link_flags + compiler_flags + ["-o", output_name]

    print("Running compile command:\n", " ".join(compile_cmd))
    
    # Execute compilation and capture return code
    result = subprocess.run(compile_cmd)
    
    # Print execution status
    if result.returncode == 0:
        print("\n✅ Compilation succeeded!")
    else:
        print(f"\n❌ Compilation failed with return code {result.returncode}")

if __name__ == "__main__":
    build()
