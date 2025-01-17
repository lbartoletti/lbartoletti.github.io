# Path to executable
$executablePath = ".\EllipseBenchmark.exe"

# Check if file exists
if (-not (Test-Path $executablePath)) {
    Write-Error "EllipseBenchmark.exe was not found in the current directory."
    exit 1
}

# Create results directory if needed
$resultsDir = ".\results"
if (-not (Test-Path $resultsDir)) {
    New-Item -ItemType Directory -Path $resultsDir | Out-Null
}

# Loop to run benchmark with N from 1 to 16
for ($n = 1; $n -le 16; $n++) {
    $outputFile = "ellipse_benchmark_$n.csv"
    Write-Host "Running EllipseBenchmark with N = $n"
    Write-Host "Output to $outputFile"
    Write-Host "----------------------------------------"
    
    try {
        # Run program and redirect output to CSV file
        & $executablePath $n | Out-File -FilePath $outputFile -Encoding UTF8
        Write-Host "Execution completed successfully"
        Write-Host ""  # Empty line for readability
    }
    catch {
        Write-Error "Error while running with N = $n : $_"
    }
}

Write-Host "All benchmarks have been executed."
Write-Host "Results are available in ellipse_benchmark_[1-16].csv files"