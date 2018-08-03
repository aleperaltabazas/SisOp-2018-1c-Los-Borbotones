/*
 * strings.c
 *
 *  Created on: 15 jul. 2018
 *      Author: alesaurio
 */

#include "strings.h"

char* transfer(char* src, int size) {
	char* ret = malloc(size);
	memcpy(ret, src, size);
	return ret;
}

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

bool esDePuntuacionOEnie(char caracter) {
	char caracteresDePuntuacionYEnie[] = "!,#~$%&/()=: \"\'¿¡|ñ";

	int i = 0;
	while (caracteresDePuntuacionYEnie[i]) {
		if (caracter == caracteresDePuntuacionYEnie[i]) {
			return true;
		}

		i++;
	}

	return false;
}

bool esParseable(char caracter) {
	return isalnum(caracter) || esDePuntuacionOEnie(caracter);
}
