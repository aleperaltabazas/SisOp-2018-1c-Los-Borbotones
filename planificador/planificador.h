/*
 * planificador.h
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <shared-library.h>
//#include "shared-library.h"

//Funciones
FILE* levantar_archivo(char** archivo);
void error_de_archivo(char* mensaje, int retorno);
void che_parsea(int socket_cliente, FILE* archivo_a_parsear);

#endif /* PLANIFICADOR_H_ */
