#!/bin/bash
rm result
touch result
./predictor --custom ../traces/int1 >> result 2>&1 
./predictor --custom ../traces/int2 >> result 2>&1 
./predictor --custom ../traces/f1 >> result   2>&1 
./predictor --custom ../traces/f2 >> result   2>&1 
./predictor --custom ../traces/mm1 >> result  2>&1
./predictor --custom ../traces/mm2 >> result  2>&1
cat result
