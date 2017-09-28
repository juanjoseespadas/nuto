#!/usr/bin/env bash

set -e
set -o pipefail

if [[ "${CI_TARGET}" == lint ]]; then
  exit
fi

export PATH=/home/travis/miniconda/bin:$PATH
./scripts/install_numpy.sh
./scripts/install_gmsh.sh
sudo apt-get -qq update
sudo apt-get install -y libeigen3-dev libgcc-6-dev libiomp-dev swig3.0 doxygen python3-dev python3-numpy libboost-system1.55-dev libboost-filesystem1.55-dev libboost-test1.55-dev libboost-mpi1.55-dev libopenblas-dev libmetis-dev libmumps-seq-dev libann-dev libarpack2-dev
sudo apt-get install -y lcov curl texlive-font-utils libglu1-mesa-dev
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-6 99
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-4.0 99
g++ --version
clang++ --version
