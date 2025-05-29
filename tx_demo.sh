#!/bin/bash

if [ -f "tx_files/tx_demo.txt" ]; then
  echo "File exists."
  rm tx_files/tx_demo.txt
fi

read -p "Message to transmit: " msg

echo $msg > tx_files/tx_demo.txt

tst/file_tx_test

echo
echo
echo

echo "Transmitted Message:"
echo $msg