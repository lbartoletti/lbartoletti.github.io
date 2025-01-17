#!/bin/sh

# Loop through number of threads
i=1
while [ $i -le 16 ]; do
    # Run for GCC
    for version in 12 13 14 15; do
        echo "Testing with g++${version} and ${i} thread(s)"
        ./ellipse_benchmark_g${version} $i > ellipse_benchmark_${i}_g${version}.csv 2>&1
    done

    # Run for Clang
    for version in 15 16 17 18 19 20; do
        echo "Testing with clang++${version} and ${i} thread(s)"
        ./ellipse_benchmark_c${version} $i > ellipse_benchmark_${i}_c${version}.csv 2>&1
    done

    i=$((i + 1))
done