#!/bin/bash
rm gresult
touch gresult
./predictor --gshare:$1 ../traces/int1 >> gresult 2>&1 
./predictor --gshare:$1 ../traces/int2 >> gresult 2>&1 
./predictor --gshare:$1 ../traces/f1 >> gresult   2>&1 
./predictor --gshare:$1 ../traces/f2 >> gresult   2>&1 
./predictor --gshare:$1 ../traces/mm1 >> gresult  2>&1
./predictor --gshare:$1 ../traces/mm2 >> gresult  2>&1
cat gresult
