/*
 * esi.h
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#ifndef ESI_H_
#define ESI_H_

#include <sys/socket.h>
#include <netdb.h>
#include <commons/log.h>

int conectar_a(char*, char*);
void tira_error(char*);

#endif /* ESI_H_ */
