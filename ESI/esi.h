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

//Variables locales

t_list* lineas_parseadas;

//Funciones

void iniciar(char** argv);
	/*
	 * Descripción: inicia el log y llama a parsear() para guardar las lineas en la lista de parseadas.
	 * Argumentos:
	 * 		char** argv: vector de char*, que se usa con levantar_archivo() en la posición 1.
	 */

t_esi_operacion parsear(char* line);
	/*
	 * Descripción: parsea una línea y devuelve el resultado en un t_esi_operacion.
	 * Argumentos:
	 * 		char* line: linea a parsear.
	 */

int recibir_ID(int server_socket);
	/*
	 * Descripción: recibe un aviso del servidor y en caso de ser un id, lo almacena en this_id.
	 * Argumentos:
	 * 		int server_socket: socket del cual se hace el recv().
	 */

char* siguiente_linea(FILE* fp);
	/*
	 * Descripción: devuelve la siguiente línea de un archivo.
	 * Argumentos:
	 * 		FILE* fp: archivo del cual se quiere la línea.
	 */

void esperar_orden_de_parseo(int socket_planificador, int socket_coordinador,
		FILE* archivo_de_parseo);

void error_de_archivo(char* archivo, int exit_value);

FILE* levantar_archivo(char* archivo);

bool solicitar_permiso(int socket_coordinador);

void avisar_cierre(int sockfd);

void ready(int sockfd);

void esperar_ejecucion(int sockfd_1, int sockfd_2);

void ejecutar(void);

void asignar_ID(int socket_planificador);

#endif /* ESI_H_ */
