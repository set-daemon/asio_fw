#!/bin/bash
ab -n 1000000 -c 1000 -k -p $(pwd)/post_512.dat http://127.0.0.1:18981/
#ab -n 1 -c 1 -k -p $(pwd)/post_512.dat http://127.0.0.1:18981/
