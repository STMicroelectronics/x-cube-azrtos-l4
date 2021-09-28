arm-none-eabi-gcc -c -g -mcpu=cortex-m3 -mthumb tx_simulator_startup.S
arm-none-eabi-gcc -c -g -mcpu=cortex-m3 -mthumb cortexm3_crt0.S
arm-none-eabi-gcc -c -g -mcpu=cortex-m3 -mthumb tx_initialize_low_level.S
arm-none-eabi-gcc -c -g -mcpu=cortex-m3 -mthumb -I../../../../common/inc -I../inc sample_threadx.c
arm-none-eabi-ld -A cortex-m3 -ereset_handler -T sample_threadx.ld tx_simulator_startup.o cortexm3_crt0.o tx_initialize_low_level.o sample_threadx.o tx.a  libc.a -o sample_threadx.out -M > sample_threadx.map


