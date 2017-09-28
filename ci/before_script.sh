#!/usr/bin/env bash

set -e
set -o pipefail

if [[ "${CI_TARGET}" == lint ]]; then
  exit
fi

echo CI_TARGET: "${CI_TARGET}"
echo TRAVIS_BUILD_DIR: "${TRAVIS_BUILD_DIR}"
echo BUILD_DIR: "${BUILD_DIR}"
echo CMAKE_FLAGS: "${CMAKE_FLAGS}"
mkdir "${BUILD_DIR}"
cd "${BUILD_DIR}"
cmake "${CMAKE_FLAGS}" ..
