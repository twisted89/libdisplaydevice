# install dependencies for C++ analysis
set -e

# update pacman
pacman --noconfirm -Suy

# install dependencies
pacman --noconfirm -S \
  base-devel \
  make \
  mingw-w64-x86_64-binutils \
  mingw-w64-x86_64-cmake \
  mingw-w64-x86_64-toolchain

# build
mkdir -p build
cd build || exit 1
cmake -G "MinGW Makefiles" ..
mingw32-make -j"$(nproc)"

# skip autobuild
echo "skip_autobuild=true" >> "$GITHUB_OUTPUT"
