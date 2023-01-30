#!/bin/bash

# This script will change the follow setting during
# execute the program. And at the end of the script,
# the setting will be set to default.
# - Disable address space layout randomization
# - Set cpu governor to performance
# - Disable turbo mode on intel cpu
# - Execute the program with specific cpu

# Default ASLR  = 2
# Default GOV   = powersave
# Default TURBO = 0
CPUID=15
ORIG_ASLR=`cat /proc/sys/kernel/randomize_va_space`
ORIG_GOV=`cat /sys/devices/system/cpu/cpu$CPUID/cpufreq/scaling_governor`
ORIG_TURBO=`cat /sys/devices/system/cpu/intel_pstate/no_turbo`

echo "Changing system setting"
sudo bash -c "echo 0 > /proc/sys/kernel/randomize_va_space"
sudo bash -c "echo performance > /sys/devices/system/cpu/cpu$CPUID/cpufreq/scaling_governor"
sudo bash -c "echo 1 > /sys/devices/system/cpu/intel_pstate/no_turbo"

# Measure the performance of fibdrv
make clean
make
sudo taskset -c 15 ./bntest 1000 > bn_fib.log
gnuplot plot.gp


echo "Restoring default setting of system"
sudo bash -c "echo $ORIG_ASLR >  /proc/sys/kernel/randomize_va_space"
sudo bash -c "echo $ORIG_GOV > /sys/devices/system/cpu/cpu$CPUID/cpufreq/scaling_governor"
sudo bash -c "echo $ORIG_TURBO > /sys/devices/system/cpu/intel_pstate/no_turbo"
