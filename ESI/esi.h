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

//Funciones
void parsear(FILE* archivo_a_parsear);
void esperar_orden_de_parseo(int socket_planificador, int socket_coordinador);
bool solicitar_permiso(int socket_coordinador);

#endif /* ESI_H_ */
