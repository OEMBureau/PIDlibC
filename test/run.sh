#!/bin/bash

clang ../pid.c ./relay-output.c -o relay-output 
./relay-output
rm relay-output