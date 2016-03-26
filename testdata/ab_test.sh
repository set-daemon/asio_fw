#!/bin/bash
ab -n 1000000 -c 4800 -k -p $(pwd)/post_512.dat http://127.0.0.1:18981/
