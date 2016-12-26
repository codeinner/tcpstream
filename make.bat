mkdir build
pushd build
cmake ./.. && cmake --build . --config Release && ctest --output-on-failure
popd