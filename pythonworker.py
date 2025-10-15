import json
import subprocess
import sys
from pathlib import Path

def build(config_file="build.json"):
    with open(config_file, "r") as f:
        config = json.load(f)

    compiler = config.get("compiler", "g++")

    # Automatically find all .cpp files in source folder if requested
    source_files = config.get("sourceFiles", [])
    auto_include_folder = config.get("autoIncludeFolder")
    if auto_include_folder:
        folder_path = Path(auto_include_folder)
        cpp_files = [str(p) for p in folder_path.rglob("*.cpp")]
        source_files.extend(cpp_files)

    source_files_str = " ".join(source_files)
    include_paths = " ".join(f"-I\"{path}\"" for path in config.get("includePaths", []))
    library_paths = " ".join(f"-L\"{path}\"" for path in config.get("libraryPaths", []))
    libraries = " ".join(f"-l{lib}" for lib in config.get("libraries", []))
    compiler_flags = " ".join(config.get("compilerFlags", []))
    output = config.get("output", "a.exe")

    command = f"{compiler} {source_files_str} {include_paths} {library_paths} {libraries} {compiler_flags} -o {output}"
    print("Compiling with command:\n", command, "\n")

    try:
        result = subprocess.run(
            command,
            check=True,
            shell=True,
            capture_output=True,
            text=True
        )
        print(f"✅ Build successful, output: {output}")
        if result.stdout:
            print(result.stdout)
    except subprocess.CalledProcessError as e:
        print("❌ Build failed.\n")
        if e.stdout:
            print("STDOUT:\n", e.stdout)
        if e.stderr:
            print("STDERR:\n", e.stderr)

if __name__ == "__main__":
    cfg = sys.argv[1] if len(sys.argv) > 1 else "build.json"
    build(cfg)
