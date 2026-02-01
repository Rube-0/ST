cmake -B build -G Ninja -DCMAKE_MAKE_PROGRAM="D:/030/ninja.exe" --toolchain cmake/gcc-arm-none-eabi.cmake

cmake --build build

openocd -f interface/stlink.cfg -f target/stm32f1x.cfg -c "program build/usart.elf verify reset exit"