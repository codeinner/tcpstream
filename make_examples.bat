mkdir build
pushd build
cmake -D BUILD_EXAMPLES=1 ./.. && cmake --build . --config Release && ctest --output-on-failure
popd