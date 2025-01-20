# Ellipse Contour Generation Benchmark

This repository contains experimental code for generating ellipse contour points intended for use with QGIS. The project explores different algorithms and includes OpenMP parallelization experiments.

## Project Overview

The main goal is to evaluate and compare different algorithms for ellipse contour point generation, with a focus on performance optimization through parallel processing using OpenMP.

## Benchmark Environment

The benchmarks were run on three different platforms:

### Windows
- **Hardware**: AMD Ryzen 9 7900X 12-Core Processor
- **Compiler**: MSVC (Visual Studio 2022)
- **Results**: Located in `/windows` directory
- **File naming**: `ellipse_benchmark_N.csv` where N is the number of threads

### FreeBSD
- **Environment**: Proxmox VM with 12 CPU cores
- **Compilers**: 
  - Clang++ (versions 15-20)
  - GCC (versions 12-15)
- **Results**: Located in `/freebsd` directory
- **File naming**: 
  - `ellipse_benchmark_N_cVERSION.csv` for Clang++ (e.g., `ellipse_benchmark_4_c15.csv` for Clang++ 15)
  - `ellipse_benchmark_N_gVERSION.csv` for GCC (e.g., `ellipse_benchmark_4_g14.csv` for G++ 14)

### Fedora
- **Environment**: Proxmox Container with 24 CPU cores
- **Compilers**:
  - Clang++ (version 19)
  - GCC (version 14)
- **Results**: Located in `/fedora` directory
- **File naming**: Same convention as FreeBSD

## CSV File Structure

Each CSV file contains benchmark results in the following format:

```csv
segments,method,mean,min,max,stddev
4,method1,1.234567890,1.123456789,1.345678901,0.012345678
4,method2,1.234567890,1.123456789,1.345678901,0.012345678
4,method1_omp,1.234567890,1.123456789,1.345678901,0.012345678
4,method2_omp,1.234567890,1.123456789,1.345678901,0.012345678
4,method2_bis,1.234567890,1.123456789,1.345678901,0.012345678
* 4,fastest_method,1.123456789
```

Each file contains multiple test runs with:
- `segments`: Number of segments in the ellipse (from 4 to 524288, doubling each time)
- `method`: Algorithm version used (method1, method2, method1_omp, method2_omp, method2_bis)
- `mean`: Average execution time in microseconds
- `min`: Minimum execution time in microseconds
- `max`: Maximum execution time in microseconds
- `stddev`: Standard deviation of execution times
- Summary line (marked with *) showing the fastest method for each segment count

## Running the Benchmarks

The repository includes scripts for running the benchmarks:
- `benchmark.sh` for FreeBSD and Fedora
- `benchmark.ps1` for Windows

These scripts will automatically run the benchmarks with thread counts from 1 to 16 and generate the appropriate CSV files.

## Directory Structure
```
.
├── freebsd/
│   └── ellipse_benchmark_N_[c|g]VERSION.csv
├── fedora/
│   └── ellipse_benchmark_N_[c|g]VERSION.csv
├── windows/
│   └── ellipse_benchmark_N.csv
├── benchmark.sh
├── benchmark.ps1
└── README.md
```

## Implementation Details

The code implements various algorithms for ellipse contour generation, with OpenMP parallelization applied to evaluate performance scaling across different thread counts. The benchmarks test both single-threaded and multi-threaded performance up to 16 threads.
