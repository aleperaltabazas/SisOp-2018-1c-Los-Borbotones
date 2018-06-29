#/bin/bash

cd ../planificador/Debug

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/tp-2018-1c-Los-borbotones/shared-library/Debug

make
make clean
make all

clear

valgrind ./planificador planificador.config

