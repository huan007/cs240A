#!/bin/bash
rm gresult
touch gresult
./predictor --gshare:$1 int1 >> gresult 
./predictor --gshare:$1 int2 >> gresult 
./predictor --gshare:$1 f1 >> gresult 
./predictor --gshare:$1 f2 >> gresult 
./predictor --gshare:$1 mm1 >> gresult 
./predictor --gshare:$1 mm2 >> gresult 
cat gresult
