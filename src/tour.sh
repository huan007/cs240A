#!/bin/bash
rm tresult
touch tresult
./predictor --tournament:$1:$2:$3 ../traces/int1 >> tresult 2>&1 
./predictor --tournament:$1:$2:$3 ../traces/int2 >> tresult 2>&1 
./predictor --tournament:$1:$2:$3 ../traces/f1 >> tresult   2>&1 
./predictor --tournament:$1:$2:$3 ../traces/f2 >> tresult   2>&1 
./predictor --tournament:$1:$2:$3 ../traces/mm1 >> tresult  2>&1
./predictor --tournament:$1:$2:$3 ../traces/mm2 >> tresult  2>&1
cat tresult
