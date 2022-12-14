#!/bin/bash
set -e
pushd ../../../../barretenberg/build && make -j4 srs lagrange_base_gen && popd

# Take user inputs and set the default values
num_files=${1:-20}
src=${2:-../../../../barretenberg/srs_db/ignition}
dest=${3:-../../../../barretenberg/srs_db/lagrange}

echo -e "\n--------------------------------------------"
echo "Generate lagrange SRS for $num_files files."
echo "Monomial SRS path:     $src"
echo "Store lagrange SRS at: $dest"
echo -e "--------------------------------------------\n"

# Iterate the loop for generating lagrange transcripts for degree <= 2^{20}
degree=1
while [ $degree -le $num_files ] 
do 
    printf "Generating lagrange transcript for subgroup size "
    echo $((1 << $degree))

    # call the lagrange base processor
    ../../../../barretenberg/build/src/aztec/lagrange_base_gen/lagrange_base_gen $((1 << $degree)) $src $dest

    # increment the value 
    degree=`expr $degree + 1`
done