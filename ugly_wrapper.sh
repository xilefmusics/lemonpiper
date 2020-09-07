#!/bin/bash
stdbuf -oL ./lemonpiper |
while read -r line; do
  echo "$line"
done
