/*
 * esi.h
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#ifndef ESI_H_
#define ESI_H_

#include <shared-library.h>
//#include "shared-library.h"

t_list* lineas_parseadas;

//Funciones

void iniciar(char** argv);
t_esi_operacion parsear(char* line);
char* siguiente_linea(FILE* fp);
void esperar_orden_de_parseo(int socket_planificador, int socket_coordinador, FILE* archivo_de_parseo);
void error_de_archivo(char* archivo, int exit_value);
FILE* levantar_archivo(char* archivo);
bool solicitar_permiso(int socket_coordinador);

#endif /* ESI_H_ */
