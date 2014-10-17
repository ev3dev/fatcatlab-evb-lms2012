#!/bin/sh

echo ----------------------------------------------------------------------------------

echo EHRPWM0
cat /sys/kernel/debug/omap_mux/mcasp0_aclkx
echo                    ----------------------------
cat /sys/kernel/debug/omap_mux/mcasp0_fsx

echo ----------------------------------------------------------------------------------

echo EHRPWM1
cat /sys/kernel/debug/omap_mux/gpmc_a2
echo                    ----------------------------
cat /sys/kernel/debug/omap_mux/gpmc_a3

echo ----------------------------------------------------------------------------------
