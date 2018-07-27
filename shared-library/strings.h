/*
 * strings.h
 *
 *  Created on: 15 jul. 2018
 *      Author: alesaurio
 */

#ifndef STRINGS_H_
#define STRINGS_H_

#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>

char* transfer(char* src, int size);
	/*
	 * Descripción: devuelve un char* con el contenido de src de longitud size.
	 * Argumentos:
	 * 		char* src
	 * 		int size
	 */

bool mismoString(char* unString, char* OtroString);
	/*
	 * Descripción: llama a strcmp() y devuelve si los dos strings son iguales.
	 * Argumentos:
	 * 		char* unString: string a comparar.
	 * 		char* otroString: string a comparar.
	 */

void cerrar_cadena(char* cadena);
	/*
	 * Descripción: agrega \0 al final de una cadena.
	 * Argumentos:
	 * 		char* cadena: el string.
	 */

bool esParseable(char caracter);
	/*
	 * Descripción: devuelve si un caracter es alfa numérico o dos puntos.
	 * Argumentos:
	 * 		char* caracter: el caracter.
	 */

#endif /* STRINGS_H_ */
