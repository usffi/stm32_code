@echo off
cd /d %~dp0
"C:\Users\a3331\.platformio\packages\toolchain-gccarmnoneeabi\bin\arm-none-eabi-objdump.exe" -d .pio\build\bluepill_f103c8\firmware.elf > disasm.txt
echo 已生成 disasm.txt
pause
