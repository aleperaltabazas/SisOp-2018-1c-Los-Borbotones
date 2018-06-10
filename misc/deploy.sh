#/bin/bash
clear
echo "Comienza el deploy"

cd /home/utnso/tp-2018-1c-Los-borbotones/Coordinador/Debug/

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/tp-2018-1c-Los-borbotones/shared-library/Debug

echo "Coordinador listo"

cd /home/utnso/tp-2018-1c-Los-borbotones/Planificador/Debug

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/tp-2018-1c-Los-borbotones/shared-library/Debug

echo "Planificador listo"

cd /home/utnso/tp-2018-1c-Los-borbotones/ESI/Debug

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/tp-2018-1c-Los-borbotones/shared-library/Debug

echo "ESI listo"

cd /home/utnso/tp-2018-1c-Los-borbotones/Instancias/Debug/

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/tp-2018-1c-Los-borbotones/shared-library/Debug

echo "Instancia lista"

cd /home/utnso/tp-2018-1c-Los-borbotones

echo "Deploy completo"
