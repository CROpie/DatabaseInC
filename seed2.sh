#!/bin/bash

if [ -f "data.db" ]; then
  rm data.db
fi

if [ -f "index.db" ]; then
  rm index.db
fi

if [ -f "commands.txt" ]; then
  rm commands.txt
fi

word_list=()
while IFS= read -r line; do
  word_list+=("$line")
done < output.txt

# num_entries=1
num_entries=10000

# for ((i=0; i<num_entries; i++)); do
#   rand_index=$(shuf -i 0-$((${#word_list[@]} - 1)) -n 1)
#   echo "insert ${word_list[$rand_index]}" >> commands.txt
# done

# for ((i=0; i<num_entries; i++)); do
#   rand_index0=$(shuf -i 0-$((${#word_list[@]} - 1)) -n 1)
#   rand_index1=$(shuf -i 0-$((${#word_list[@]} - 1)) -n 1)
#   echo "insert ${word_list[$rand_index0]} ${word_list[$rand_index1]}" >> commands.txt
# done

for ((i=0; i<num_entries; i++)); do
  word="${word_list[$((i % ${#word_list[@]}))]}"
  echo "insert $word" >> commands.txt
done

echo "exit" >> commands.txt
# ./build/db < commands.txt
valgrind --track-origins=yes ./build/db < commands.txt
# valgrind --leak-check=full ./build/db < commands.txt
