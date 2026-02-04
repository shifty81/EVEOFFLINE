#!/bin/bash
# Build test for post-processing effects

echo "Building post-processing test..."
cd cpp_client
mkdir -p build
cd build
cmake .. -DBUILD_TESTS=ON
make test_post_processing
echo "Done! Run with: cd cpp_client/build/bin && ./test_post_processing"
