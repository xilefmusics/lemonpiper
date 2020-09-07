#!/bin/bash
killall -q lemonbar
killall -q lemonpiper


./ugly_wrapper.sh | lemonbar -p -g 1920x24+0+0 -F "#ebdbb2" -B "#262626" -U "#FF0000" -f "InputMono"-9 | bash
