#!/bin/bash

if [ -f "data.db" ]; then
  rm data.db
fi

if [ -f "commands.txt" ]; then
  rm commands.txt
fi

for i in {1..10000}; do
  rand=$(cat /dev/urandom | tr -dc 'a-z' | head -c8)
  echo "insert $rand" >> commands.txt
done

echo "exit" >> commands.txt

./build/db < commands.txt
