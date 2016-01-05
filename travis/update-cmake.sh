CMAKE_BIN="$HOME/cmake/build/install/bin"

if [ ! -d $CMAKE_BIN ]; then

	cd $HOME
	rm -rf cmake
	
	mkdir cmake
	git clone https://github.com/Kitware/CMake.git cmake
	
	cd cmake && mkdir build && cd build
	
	cmake -DCMAKE_INSTALL_PREFIX=$(pwd)/install -DCMAKE_BUILD_TYPE=RELEASE ..
	cmake --build . && cmake --build . --target install
fi

cd $CMAKE_BIN
CMAKEPATH=$(pwd)
export path="$CMAKEPATH:$PATH"