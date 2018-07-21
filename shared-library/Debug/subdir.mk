################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../getPokemon.c \
../lists.c \
../shared-library.c \
../strings.c 

OBJS += \
./getPokemon.o \
./lists.o \
./shared-library.o \
./strings.o 

C_DEPS += \
./getPokemon.d \
./lists.d \
./shared-library.d \
./strings.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


