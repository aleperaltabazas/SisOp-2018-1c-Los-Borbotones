/*
 * strings.c
 *
 *  Created on: 15 jul. 2018
 *      Author: alesaurio
 */

#include "strings.h"

bool mismoString(char* unString, char* otroString) {
	return strcmp(unString, otroString) == 0;
}

void cerrar_cadena(char* cadena) {
	int i = 0;

	while (esParseable(cadena[i])) {
		i++;
	}

	cadena[i] = '\0';
}

bool esParseable(char caracter) {
	return isalnum(caracter) || caracter == ':';
}
