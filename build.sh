#!/bin/sh
g++ -I/opt/local/include/opencv -I/opt/local/include `pkg-config opencv --libs` $1
