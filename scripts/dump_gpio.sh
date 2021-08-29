#!/bin/bash

for pin in {0..32}
do
    val=$(gpioget gpiochip0 $pin)
	echo "$pin = $val"
done
# 12