#!/bin/bash
rm result
touch result
./predictor --custom int1 >> result 
./predictor --custom int2 >> result 
./predictor --custom f1 >> result 
./predictor --custom f2 >> result 
./predictor --custom mm1 >> result 
./predictor --custom mm2 >> result 
cat result
