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
	 * Descripci�n: inicia el log y llama a parsear() para guardar las lineas en la lista de parseadas.
	 * Argumentos:
	 * 		char** argv: vector de char*, que se usa con levantar_archivo() en la posici�n 1.
	 */

t_esi_operacion parsear(char* line);
	/*
	 * Descripci�n: parsea una l�nea y devuelve el resultado en un t_esi_operacion.
	 * Argumentos:
	 * 		char* line: linea a parsear.
	 */

int recibir_ID(int server_socket);
	/*
	 * Descripci�n: recibe un aviso del servidor y en caso de ser un id, lo almacena en this_id.
	 * Argumentos:
	 * 		int server_socket: socket del cual se hace el recv().
	 */

char* siguiente_linea(FILE* fp);
	/*
	 * Descripci�n: devuelve la siguiente l�nea de un archivo.
	 * Argumentos:
	 * 		FILE* fp: archivo del cual se quiere la l�nea.
	 */

void error_de_archivo(char* archivo, int exit_value);
	/*
	 * Descripci�n: informa si hubo un error en la apertura del archivo y sale con error.
	 * Argumentos:
	 * 		char* archivo: el archivo a abrir
	 * 		int exit_value: valor de salida de error (2).
	 */

FILE* levantar_archivo(char* archivo);
	/*
	 * Descripci�n: devuelve un archivo abierto a partir de su nombre.
	 * Argumentos:
	 * 		char* archivo: el archivo a abrir.
	 */

bool solicitar_permiso(int socket_coordinador);
	/*
	 * Descripci�n: devuelve un booleano si puede ejecutar la pr�xima instrucci�n o no.
	 * Argumentos:
	 * 		int socket_coordinador: socket del coordinador, quien le informa si el recurso que
	 * 			solicit� est� disponible
	 */

void avisar_cierre(int sockfd);
	/*
	 * Descripci�n: informa a proceso que �ste va a cerrar.
	 * Argumentos:
	 * 		int sockfd: socket al cu�l se enviar� el mensaje de cierre.
	 */

void ready(int sockfd);
	/*
	 * Descripci�n: le informa al planificador que se encuentra listo para ejecutarse.
	 * Argumentos:
	 * 		int socket_planificador: socket del planificador.
	 */

void esperar_ejecucion(int socket_coordinador, int socket_planificador);
	/*
	 * Descripci�n: espera a recibir una orden de ejecuci�n por parte del planificador y llama a
	 * 		solicitar_permiso() para ejecutar. En caso positivo, llama a ejecutar().
	 * Argumentos:
	 * 		int socket_coordinador: socket del coordinador.
	 * 		int socket_planificador: socket del planificador.
	 */

void ejecutar(void);
	/*
	 * Descripci�n: ejecuta la siguiente instrucci�n como indique la lista de l�neas parseadas,
	 * 		e informa el resultado al coordinador.
	 * Arumgnetos:
	 * 		void
	 */

#endif /* ESI_H_ */
