#!/bin/bash -e

# Simple build script wrapper around python, conan, and cmake that simplifies
# the setup and build of polimor.
# 
# It takes up to 3 arguments:
#   --build_type|-b 'Debug' or 'Release' for the build type.
#   --build_dir|-d Path to the directory to build in.
#   --python|-p the python interpreter to use.

# Default settings
BUILD_TYPE="Debug"
BUILD_DIR="../build"
PYTHON_EXE="python3"


# Parse the command line
function usage() {
    echo "Usage: $0 [ --help|-h --build_type|-b <Debug|Release> --build_dir|-d <directory> --python|-p <python>]" 2>&1
    exit 1
}

VALID_ARGS=$(getopt -o b:d:p:h:: --long build_type:,build_dir:,python:,help:: -- "$@")
if [[ $? -ne 0 ]]; then
    exit 1;
fi

eval set -- "$VALID_ARGS"
while [ : ]; do
  case "$1" in
    -b | --build-type)
        BUILD_TYPE="$2"

        # Verify Build is of the right type
        case "$2" in
            Debug)
                ;;
            Release)
                BUILD_TYPE="$2"
                ;;
            *)
                echo "Error: Invalid build type, must be: Debug or Release" 2>&1
                usage
                ;;
        esac

        shift 2
        ;;
    -d | --build_dir)
        BUILD_DIR="$(realpath $2)"
        shift 2
        ;;
    -p | --python)
        PYTHON_EXE="$2"
        shift 2
        ;;
    -h | --help)
        usage
        ;;
    ?)
        usage
        ;;
    :) 
        usage
        ;;
    --)
        shift
        break
        ;;
    *)
        usage
        ;;
  esac
done


BUILD_DIR="$(realpath ${BUILD_DIR})"


echo "$BUILD_TYPE"
echo "$BUILD_DIR"
echo "$PYTHON_EXE"


# Create the build directory
echo "Creating build directory ${BUILD_DIR}/${BUILD_TYPE}..."
mkdir -p "${BUILD_DIR}/${BUILD_TYPE}"

# Run pip
#curl -O -L https://bootstrap.pypa.io/get-pip.py
#${PYTHON_EXE} get-pip.py

# Set up the python venv
echo "Setting up python venv..."
${PYTHON_EXE} -m venv "${BUILD_DIR}/venv"
source "${BUILD_DIR}/venv/bin/activate"

# Install the dependencies
echo "Installing python dependencies.."
pip install jsonschema conan

# Run conan to grab the packages
echo "Running conan to install C++ dependencies.."
export CONAN_HOME="$BUILD_DIR/conan"

# If there is no default conan profile then create one
if [[ -z "$(conan profile list | grep 'default')" ]]; then
    conan profile detect
fi

conan install . --output-folder="${BUILD_DIR}/${BUILD_TYPE}" --build=missing --settings=build_type=${BUILD_TYPE} 

# Change to the build directory
SRC_DIR="$(pwd)"
cd "${BUILD_DIR}/${BUILD_TYPE}"

# Configure cmake
echo "Configuring cmake..."
cmake "${SRC_DIR}" -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} 
 
# Build 
echo "Building..."
make -j
