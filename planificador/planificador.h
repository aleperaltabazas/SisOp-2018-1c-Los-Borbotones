/*
 * planificador.h
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
//#include <shared-library.h>
#include "shared-library.h"
#include <commons/log.h>
#include <commons/collections/list.h>

void escucharRespuesta(void);
void iniciar_log(void);

#endif /* PLANIFICADOR_H_ */
