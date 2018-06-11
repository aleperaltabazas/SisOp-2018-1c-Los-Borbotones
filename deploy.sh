#/bin/bash

echo "Ejecutando deploy"

cd /home/utnso/tp-2018-1c-Los-borbotones/Coordinador/Debug

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/tp-2018-1c-Los-borbotones/shared-library/Debug

cd /home/utnso/tp-2018-1c-Los-borbotones/planificador/Debug

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/tp-2018-1c-Los-borbotones/shared-library/Debug

cd /home/utnso/tp-2018-1c-Los-borbotones/ESI/Debug

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/tp-2018-1c-Los-borbotones/shared-library/Debug

cd /home/utnso/tp-2018-1c-Los-borbotones/Instancias/Debug

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/tp-2018-1c-Los-borbotones/shared-library/Debug

echo "Deploy completo"

