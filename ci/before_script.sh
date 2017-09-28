#!/usr/bin/env bash

set -e
set -o pipefail

if [[ "${CI_TARGET}" == lint ]]; then
  exit
fi

mkdir "${BUILD_DIR}"
cd "${BUILD_DIR}"
cmake "${CMAKE_FLAGS}" ..
