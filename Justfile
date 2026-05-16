cross-cmake:
    . "{{ env("BIN") }}/msvcenv.sh" && \
        PATH="{{ env("BIN") }}:$PATH" CC=cl CXX=cl \
        cmake -B build . \
            -G Ninja \
            -DCMAKE_BUILD_TYPE=Release \
            -DSTATIC_MSVC_RUNTIME=ON \
            -DCMAKE_SYSTEM_NAME=Windows \
            -DCMAKE_TOOLCHAIN_FILE=cmake/msvc-wine.cmake
