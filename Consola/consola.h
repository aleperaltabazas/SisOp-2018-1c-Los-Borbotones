/*
 * consola.h
 *
 *  Created on: 5 may. 2018
 *      Author: utnso
 */

#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
//#include <shared-library.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <readline/readline.h>
#include <pthread.h>

t_log * logger;

void listarOpciones(void);
void pausarOContinuar(void);
void bloquear(float codigo);
void desbloquear(float codigo);
void listar (void);
void kill (float codigo);
void status (float codigo);
void deadlock (void);
float recibirCodigo (void);
void interpretarYEjecutarCodigo (float comando);
void * consolita(void);

#endif /* CONSOLA_H_ */
