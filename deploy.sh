#/bin/bash
clear
echo "Comienza el deploy"

sleep 1

cd Coordinador/Debug/

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/tp-2018-1c-Los-borbotones/shared-library/Debug

echo "Coordinador listo"

cd ..
cd ..

sleep 1

cd planificador/Debug/

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/tp-2018-1c-Los-borbotones/shared-library/Debug

echo "Planificador listo"

cd ..
cd ..

sleep 1

cd ESI/Debug/

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/tp-2018-1c-Los-borbotones/shared-library/Debug

echo "ESI listo"

cd ..
cd ..

sleep 1

cd Instancias/Debug/

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/tp-2018-1c-Los-borbotones/shared-library/Debug

echo "Instancia lista"

cd ..
cd ..

sleep 1

echo "Deploy completo"
