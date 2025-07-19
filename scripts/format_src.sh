#!/bin/bash

# directories to be formatted (recursive search)
DIRS="QSliderX/include QSliderX/src tests"
# FORMAT_CMD="clang-format --style=LLVM -i {}"
FORMAT_CMD="clang-format -style=file:scripts/clang_style -i"

for D in ${DIRS}; do
    for F in `find ${D}/. -type f \( -iname \*.hpp -o -iname \*.cpp \)`; do
	echo ${F}
	${FORMAT_CMD} ${F}
    done
done

cmake-format -i CMakeLists.txt
cmake-format -i QSliderX/CMakeLists.txt
cmake-format -i tests/CMakeLists.txt
cmake-format -i tests/test_qsx/CMakeLists.txt
