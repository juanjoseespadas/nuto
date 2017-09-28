#!/usr/bin/env bash

set -e
set -o pipefail

if [[ "${CI_TARGET}" == lint ]]; then
  exit
fi

mkdir "${BUILD_DIR}"
cd "${BUILD_DIR}"
echo "cmake ${CMAKE_FLAGS} .."
cmake "${CMAKE_FLAGS}" ..
