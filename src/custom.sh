#!/bin/bash
rm result
touch result
./predictor --custom ../traces/int1 >> result 
./predictor --custom ../traces/int2 >> result 
./predictor --custom ../traces/f1 >> result 
./predictor --custom ../traces/f2 >> result 
./predictor --custom ../traces/mm1 >> result 
./predictor --custom ../traces/mm2 >> result 
cat result
