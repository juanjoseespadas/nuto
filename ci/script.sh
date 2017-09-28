
#!/usr/bin/env bash

set -e
set -o pipefail

if [[ "${CI_TARGET}" == lint ]]; then
  sudo docker run -ti -v "$(pwd)":/app --workdir=/app coala/base coala
fi

if [[ "${CI_TARGET}" == unit ]]; then
  cd "${BUILD_DIR}"
  make -j2 unit
fi

if [[ "${CI_TARGET}" == test ]]; then
  cd "${BUILD_DIR}"
  make -j2 && ctest --output-on-failure
fi
