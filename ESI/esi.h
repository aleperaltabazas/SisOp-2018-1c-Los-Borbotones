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

typedef struct t_parsed_node{
	int index;
	t_esi_operacion esi_op;
	struct t_parsed_node* sgte;
} t_parsed_node;

typedef struct t_parsed_list{
	t_parsed_node* head;
} t_parsed_list;

t_parsed_list parsed_ops;

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

void error_de_archivo(char* archivo, int exit_value);
	/*
	 * Descripción: informa si hubo un error en la apertura del archivo y sale con error.
	 * Argumentos:
	 * 		char* archivo: el archivo a abrir
	 * 		int exit_value: valor de salida de error (2).
	 */

FILE* levantar_archivo(char* archivo);
	/*
	 * Descripción: devuelve un archivo abierto a partir de su nombre.
	 * Argumentos:
	 * 		char* archivo: el archivo a abrir.
	 */

bool solicitar_permiso(int socket_coordinador);
	/*
	 * Descripción: devuelve un booleano si puede ejecutar la próxima instrucción o no.
	 * Argumentos:
	 * 		int socket_coordinador: socket del coordinador, quien le informa si el recurso que
	 * 			solicitó está disponible
	 */

void avisar_cierre(int sockfd);
	/*
	 * Descripción: informa a proceso que éste va a cerrar.
	 * Argumentos:
	 * 		int sockfd: socket al cuál se enviará el mensaje de cierre.
	 */

void ready(int sockfd);
	/*
	 * Descripción: le informa al planificador que se encuentra listo para ejecutarse.
	 * Argumentos:
	 * 		int socket_planificador: socket del planificador.
	 */

void esperar_ejecucion(int socket_coordinador, int socket_planificador);
	/*
	 * Descripción: espera a recibir una orden de ejecución por parte del planificador y llama a
	 * 		solicitar_permiso() para ejecutar. En caso positivo, llama a ejecutar().
	 * Argumentos:
	 * 		int socket_coordinador: socket del coordinador.
	 * 		int socket_planificador: socket del planificador.
	 */

void ejecutar(int socket_planificador, int socket_coordinador);
	/*
	 * Descripción: ejecuta la siguiente instrucción como indique la lista de líneas parseadas,
	 * 		e informa el resultado al coordinador.
	 * Arumgnetos:
	 * 		int socket_planificador: socket del planificador.
	 * 		int socket_coordinador: socket del coordinador.
	 */

#endif /* ESI_H_ */
