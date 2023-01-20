#!/bin/bash

# This script provide two mode to measure execution time of fibonacci calculating.
# Mode 0 is the default setting in your system.
# Mode 1 change the following setting:
# - Disable address space layout randomization
# - Set cpu governor to performance
# - Disable turbo mode on intel cpu
# - Execute the program with specific cpu

# Default ASLR  = 2
# Default GOV   = powersave
# Default TURBO = 0
CPUID=4
ORIG_ASLR=`cat /proc/sys/kernel/randomize_va_space`
ORIG_GOV=`cat /sys/devices/system/cpu/cpu$CPUID/cpufreq/scaling_governor`
ORIG_TURBO=`cat /sys/devices/system/cpu/intel_pstate/no_turbo`

echo "mode"
read mode
if [[ ${mode} == "1" ]] ; then
TASKSET = "taskset -c ${CPUID}"
echo "Changing system setting"
sudo bash -c "echo 0 > /proc/sys/kernel/randomize_va_space"
sudo bash -c "echo performance > /sys/devices/system/cpu/cpu$CPUID/cpufreq/scaling_governor"
sudo bash -c "echo 1 > /sys/devices/system/cpu/intel_pstate/no_turbo"
else
echo "Test with default setting"
fi

# Measure the performance of fibdrv
make clean
make
make load
if [[ $mode == "1" ]]; then
sudo taskset -c 4 ./client > out
else
sudo ./client > out
fi
make unload
gnuplot scripts/plot_syscall_overhead.gp


echo "Restoring default setting of system"
sudo bash -c "echo $ORIG_ASLR >  /proc/sys/kernel/randomize_va_space"
sudo bash -c "echo $ORIG_GOV > /sys/devices/system/cpu/cpu$CPUID/cpufreq/scaling_governor"
sudo bash -c "echo $ORIG_TURBO > /sys/devices/system/cpu/intel_pstate/no_turbo"
