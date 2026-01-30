set -e

if [ "$JAVA_HOME" == "" ]; then
  echo "You need to set JAVA_HOME to the jdk home folder"
  exit
fi

cmake -S . -B build
cmake --build build --config Release --clean-first
cd build && ctest -V -C Release