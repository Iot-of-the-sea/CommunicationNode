#!/bin/bash

if [ -f "tst/test_received/rx_demo.txt" ]; then
  echo "File exists."
  rm tst/test_received/rx_demo.txt
fi

tst/file_rx_test

echo
echo
echo

if [ -f "tst/test_received/rx_demo.txt" ]; then
  echo "SUCCESS: Message received."
  cat tst/test_received/rx_demo.txt
  rm tst/test_received/rx_demo.txt
else
  echo "FAILED: No file received."
fi