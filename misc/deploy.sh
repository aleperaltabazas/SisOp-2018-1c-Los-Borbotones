#/bin/bash

clear

cd ../shared-library/Debug/

make
make clean
make all

cd ../../misc/

clear

chmod u+x coordinador.sh
chmod u+x planificador.sh
chmod u+x esilargo.sh
chmod u+x esimulticlave.sh
chmod u+x esi1.sh
chmod u+x instancias.sh

clear

echo "Scripts preparados"
