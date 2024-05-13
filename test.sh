#!/bin/bash

make clean
make server
make client

./server &
SID=$!

sleep 1 

total_time=0

# Create a CSV file and add headers
echo "Request Number,Status,Runtime (seconds)" > results.csv

for ((i=1; i<=100; i++)); do
    start=$(date +%s.%N)  # Start time in seconds since the epoch with nanosecond precision
    response=$(curl -s -o /dev/null -w "%{http_code}" http://localhost:8000/get/test.html)  # Send request and capture response code
    if [ $response -eq 200 ]; then
        echo "Request $i: done"
        status="done"
    else
        echo "Request $i: not done"
        status="not done"
    fi
    end=$(date +%s.%N)  # End time in seconds since the epoch with nanosecond precision
    runtime=$(echo "$end - $start" | bc)  # Calculate runtime for each request
    total_time=$(echo "$total_time + $runtime" | bc)  # Add runtime to total time
    echo "$i,$status,$runtime" >> results.csv  # Append request details to CSV file
done


total_requests=100
average_time=$(echo "scale=4; $total_time / $total_requests" | bc)  # Calculate average time per request

# Log statistics to a log file
echo "Total time for 100 requests: $total_time seconds" >> test.log
echo "Average time for a request: $average_time seconds" >> test.log
echo "---------------------------------------------" >> test.log

kill -9 $SID
