set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_LIBRARY_ARCHITECTURE armv7e-m)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CPU_FLAGS
    "-mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -ffunction-sections -fdata-sections -specs=nosys.specs"
)
set(COMMON_DEBUG_DIAGNOSTIC_FLAGS
    "-Wall -Wextra -Wformat=2 -Wformat-overflow=2 -Wformat-security -Wformat-signedness -Wunused-parameter -Winit-self -Wcast-align -Wconversion -Wpedantic -Wnull-dereference  -Wduplicated-cond -Wnull-dereference -Wsign-conversion -Wlogical-op -Wdouble-promotion"
)

set(CMAKE_EXECUTABLE_SUFFIX .elf)


# ########## ASM Options ###########
set(CMAKE_EXECUTABLE_SUFFIX_ASM .elf)
set(CMAKE_ASM_COMPILER "arm-none-eabi-gcc")
set(CMAKE_ASM_FLAGS "-x assembler-with-cpp ${CPU_FLAGS} -DDFLOAT_ABI_HARD")
set(CMAKE_ASM_FLAGS_DEBUG "-Os -g3 ${COMMON_DEBUG_DIAGNOSTIC_FLAGS}")
set(CMAKE_ASM_FLAGS_RELEASE "-Os -Wall")

# ########## C Options ###########
set(CMAKE_EXECUTABLE_SUFFIX_C .elf)
set(CMAKE_C_COMPILER "arm-none-eabi-gcc")
set(CMAKE_C_FLAGS "${CPU_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG
    "-Os -g3 ${COMMON_DEBUG_DIAGNOSTIC_FLAGS} -Wimplicit-fallthrough=3 -Wwrite-strings -Wvla"
)
set(CMAKE_C_FLAGS_RELEASE "-Os -Wall")



# ########## C++ Options ###########
set(CMAKE_EXECUTABLE_SUFFIX_CXX .elf)
set(CMAKE_CXX_COMPILER "arm-none-eabi-g++")
set(CMAKE_CXX_FLAGS "${CPU_FLAGS} -fconcepts")
set(CMAKE_CXX_FLAGS_DEBUG
    "-Os -g3 ${COMMON_DEBUG_DIAGNOSTIC_FLAGS} -Wimplicit-fallthrough=5 -Wdelete-non-virtual-dtor -Woverloaded-virtual -Wold-style-cast "
)
set(CMAKE_CXX_FLAGS_RELEASE "-Os -Wall")

set(CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections")


add_compile_options(
  -ffunction-sections -fdata-sections -fno-strict-aliasing -fshort-enums
  -Wno-packed-bitfield-compat -Wno-psabi
)
