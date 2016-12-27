mkdir build -p
cd build
cmake ./.. && cmake --build . --config Release && ctest . --output-on-failure
cd ..
