#!/usr/bin/env bash
set -eu

# Navigate to script folder
cd "$(dirname "$0")"

CMD=${1:-}

if [ -n "$CMD" ]; then
  if [ "$CMD" = "clean" ]; then
    git clean -ffdx
    exit 0
  else
    echo "Unknown command: $CMD"
    exit 1
  fi
fi

# Determine system.
if [[ "$OSTYPE" == "darwin"* ]]; then
  OS=macos
elif [[ "$OSTYPE" == "linux-gnu" ]]; then
  OS=linux
elif [[ "$OSTYPE" == "linux-musl" ]]; then
  OS=linux
else
  echo "Unknown OS: $OSTYPE"
  exit 1
fi

# Download ignition transcripts.
(cd ./srs_db && ./download_ignition.sh 0)

# Pick native toolchain file.
ARCH=$(uname -m)
if [ "$OS" == "macos" ]; then
  PRESET=default
else
  if [ "$(which clang++-16)" != "" ]; then
    PRESET=clang16
  else
    PRESET=default
  fi
fi

# Remove cmake cache files.
rm -f {build,build-wasm,build-wasm-threads}/CMakeCache.txt

# Function to compare versions
version_greater_equal() {
    IFS='.' read -ra VER1 <<< "$1"
    IFS='.' read -ra VER2 <<< "$2"

    for i in {0..2}; do
        VER1_PART=${VER1[i]:-0}
        VER2_PART=${VER2[i]:-0}

        if [[ "$VER1_PART" -gt "$VER2_PART" ]]; then
            return 0
        elif [[ "$VER1_PART" -lt "$VER2_PART" ]]; then
            return 1
        fi
    done

    # If all components are equal
    return 0
}

# Function to check compiler version
check_compiler_version() {
    local cmake_preset_file="CMakePresets.json"

    # Check if CMakePresets.json file exists, silently return if it does not
    if [ ! -f "$cmake_preset_file" ]; then
        return 0
    fi

    # Extract the compiler command (e.g., 'gcc' or 'clang') from CMakePresets.json
    local CC=$(jq -r --arg PRESET "$PRESET" '.configurePresets[] | select(.name == $PRESET) | .environment.CC' "$cmake_preset_file")
    
    case "$CC" in
        *gcc*|*clang*)
        
        local _minimum_version

        if [[ $CC == *"gcc"* ]]; then
            # Minimum required version to 10 for GCC
            _minimum_version="10" 
        elif [[ $CC == *"clang"* ]]; then
            # Minimum required version to 16 for Clang
            _minimum_version="16" 
        fi
        
        if [ -n "$_minimum_version" ] && command -v $CC > /dev/null; then
            # Get the installed compiler version (e.g., '9.3')
            local _version=$($CC --version | grep -o '[0-9]\+\.[0-9]\+' | head -n1) 
            # Check if installed compiler meets the version requirement
            if ! version_greater_equal "$_version" "$_minimum_version"; then
                echo "$CC version is not sufficient ($_version < $_minimum_version)"
                exit 1
            fi
        elif [ -n "$_minimum_version" ]; then
            echo "$CC is not installed"
            exit 1
        fi
        
        ;;
    esac
}

# Check CMake version
check_cmake_version() {
    local cmake_preset_file="CMakePresets.json"

    # Check if CMakePresets.json file exists, silently return if it does not
    if [ ! -f "$cmake_preset_file" ]; then
        return 0
    fi

    # Check if CMake is installed
    if ! command -v cmake &> /dev/null; then
        echo "CMake is not installed. Please install CMake before proceeding."
        exit 1
    fi

    # Extract required CMake version from CMakePresets.json
    REQUIRED_MAJOR=$(jq -r '.cmakeMinimumRequired.major' "$cmake_preset_file")
    REQUIRED_MINOR=$(jq -r '.cmakeMinimumRequired.minor' "$cmake_preset_file")
    REQUIRED_PATCH=$(jq -r '.cmakeMinimumRequired.patch' "$cmake_preset_file")

    # Get installed CMake version
    INSTALLED_CMAKE_VERSION=$(cmake --version | head -n1 | awk '{print $3}')
    
    # Compare versions using version_greater_equal function
    if ! version_greater_equal "$INSTALLED_CMAKE_VERSION" "$REQUIRED_MAJOR.$REQUIRED_MINOR.$REQUIRED_PATCH"; then
        echo "CMake version is lower than the required version $REQUIRED_MAJOR.$REQUIRED_MINOR.$REQUIRED_PATCH for $cmake_preset_file"
        echo "Please update CMake to at least this version."
        exit 1
    fi
}

# Check if Ninja is installed
check_ninja_installed() {
    # Check if Ninja is installed
    if ! command -v ninja &> /dev/null; then
        echo "Ninja is not installed. Please install Ninja before proceeding."
        exit 1
    fi
}

perform_system_checks() {

    # Call the function to check the compiler version
    check_compiler_version

    # Call the function to check CMake version
    check_cmake_version

    # Call the function to check if Ninja is installed
    check_ninja_installed
    
}

# Call the function to perform all system checks
perform_system_checks

echo "#################################"
echo "# Building with preset: $PRESET"
echo "# When running cmake directly, remember to use: --build --preset $PRESET"
echo "#################################"

# Build native.
cmake --preset $PRESET -DCMAKE_BUILD_TYPE=RelWithAssert
cmake --build --preset $PRESET --target bb

if [ ! -d ./srs_db/grumpkin ]; then
  # The Grumpkin SRS is generated manually at the moment, only up to a large enough size for tests
  # If tests require more points, the parameter can be increased here.
  (cd ./build && cmake --build . --parallel --target grumpkin_srs_gen && ./bin/grumpkin_srs_gen 8192)
fi

# Install wasi-sdk.
./scripts/install-wasi-sdk.sh

# Build WASM.
cmake --preset wasm
cmake --build --preset wasm

# Build WASM with new threading.
cmake --preset wasm-threads
cmake --build --preset wasm-threads
