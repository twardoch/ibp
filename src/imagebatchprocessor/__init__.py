# __init__.py for the imagebatchprocessor package

# Version will be dynamically set by hatch-vcs
try:
    from ._version import __version__
except ImportError:
    # Fallback version if _version.py is not generated yet (e.g., during development)
    __version__ = "0.0.0.dev0"

# You can expose parts of your C++ application here if you create Python bindings.
# For now, it's mainly for packaging the executable.

def main():
    """
    Placeholder for a potential Python entry point.
    This could be used to locate and run the C++ executable.
    """
    import subprocess
    import sys
    from pathlib import Path

    # This assumes the C++ executable is copied into this package directory by hatch_build.py
    package_dir = Path(__file__).parent
    executable_name = "ibp" # Changed from ImageBatchProcessor
    if sys.platform == "win32":
        executable_name += ".exe"

    executable_path = package_dir / executable_name

    if executable_path.exists():
        print(f"Running {executable_name} from: {executable_path}")
        try:
            # Pass along any arguments passed to the script
            subprocess.run([str(executable_path)] + sys.argv[1:], check=True)
        except subprocess.CalledProcessError as e:
            print(f"Error running {executable_name}: {e}", file=sys.stderr)
            sys.exit(e.returncode)
        except FileNotFoundError: # Should not happen if exists() check passed but good practice
            print(f"Error: Executable '{executable_name}' not found at {executable_path}", file=sys.stderr)
            sys.exit(1)
    else:
        print(f"Error: Packaged executable '{executable_name}' not found in '{package_dir}'.", file=sys.stderr)
        print("The package might not have been built correctly, or the C++ application was not found during the build.", file=sys.stderr)
        sys.exit(1)

if __name__ == '__main__':
    # This allows running the module as a script, e.g. python -m imagebatchprocessor
    # However, a console script entry point in pyproject.toml is usually preferred for user execution.
    main()

# Add a console script entry point in pyproject.toml:
# [project.scripts]
# imagebatchprocessor = "imagebatchprocessor:main"
#
# This would allow users to run `imagebatchprocessor` from the command line
# after installing the package.
# I will add this to pyproject.toml next.
