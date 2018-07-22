/*
 * testCoordinador.c
 *
 *  Created on: 22 jul. 2018
 *      Author: alesaurio
 */

#include "testCoordinador.h"

void testLSU(void) {
	ALGORITMO_DISTRIBUCION = LSU;

	Instancia inst1 = { .espacio_usado = 10, .disponible = true };
	strcpy(inst1.nombre, "Instancia 1");
	Instancia inst2 = { .espacio_usado = 30, .disponible = true };
	strcpy(inst2.nombre, "Instancia 2");
	Instancia inst3 = { .espacio_usado = 10, .disponible = true };
	strcpy(inst3.nombre, "Instancia 3");
	Instancia inst4 = { .espacio_usado = 1, .disponible = false };
	strcpy(inst4.nombre, "Instancia 4");
	Instancia inst5 = { .espacio_usado = 1, .disponible = true };
	strcpy(inst5.nombre, "Instancia 5");

	log_debug(logger, "Instancia 1 espacio: %i", inst1.espacio_usado);
	log_debug(logger, "Instancia 2 espacio: %i", inst2.espacio_usado);
	log_debug(logger, "Instancia 3 espacio: %i", inst3.espacio_usado);
	log_debug(logger, "Instancia 4 espacio: %i", inst4.espacio_usado);
	log_debug(logger, "Instancia 5 espacio: %i", inst5.espacio_usado);

	agregar_instancia(&instancias, inst1, cantidad_instancias + 1);
	cantidad_instancias++;
	redistribuir_claves();

	agregar_instancia(&instancias, inst2, cantidad_instancias + 1);
	cantidad_instancias++;
	redistribuir_claves();

	Instancia nextInst = getInstanciaSet(NULL);
	log_debug(logger, "Próxima instancia: %s", nextInst.nombre);

	agregar_instancia(&instancias, inst3, cantidad_instancias + 1);
	cantidad_instancias++;
	redistribuir_claves();

	nextInst = getInstanciaSet(NULL);
	log_debug(logger, "Próxima instancia: %s", nextInst.nombre);

	agregar_instancia(&instancias, inst4, cantidad_instancias + 1);
	cantidad_instancias++;
	redistribuir_claves();

	nextInst = getInstanciaSet(NULL);
	log_debug(logger, "Próxima instancia: %s", nextInst.nombre);

	agregar_instancia(&instancias, inst5, cantidad_instancias + 1);
	cantidad_instancias++;
	redistribuir_claves();

	nextInst = getInstanciaSet(NULL);
	log_debug(logger, "Próxima instancia: %s", nextInst.nombre);

	exit(0);
}

void testKE(void) {
	ALGORITMO_DISTRIBUCION = KE;

	Instancia inst1 = { .disponible = true };
	strcpy(inst1.nombre, "Instancia 1");
	Instancia inst2 = { .disponible = true };
	strcpy(inst2.nombre, "Instancia 2");
	Instancia inst3 = { .disponible = true };
	strcpy(inst3.nombre, "Instancia 3");
	Instancia inst4 = { .disponible = false };
	strcpy(inst4.nombre, "Instancia 4");

	agregar_instancia(&instancias, inst1, cantidad_instancias + 1);
	cantidad_instancias++;
	redistribuir_claves();

	agregar_instancia(&instancias, inst2, cantidad_instancias + 1);
	cantidad_instancias++;
	redistribuir_claves();

	Instancia nextInst = getInstanciaSet("futbol:messi");
	log_debug(logger, "Instancia para futbol:messi: %s", nextInst.nombre);

	nextInst = getInstanciaSet("pokemon:ralts");
	log_debug(logger, "Instancia para pokemon:ralts: %s", nextInst.nombre);

	nextInst = getInstanciaSet("tutor:lean");
	log_debug(logger, "Instancia para tutor:lean: %s", nextInst.nombre);

	agregar_instancia(&instancias, inst3, cantidad_instancias + 1);
	cantidad_instancias++;
	redistribuir_claves();

	nextInst = getInstanciaSet("zelda:link");
	log_debug(logger, "Instancia para zelda:link: %s", nextInst.nombre);

	nextInst = getInstanciaSet("comic:flash");
	log_debug(logger, "Instancia para comic:flash: %s", nextInst.nombre);

	nextInst = getInstanciaSet("opera:laBoheme");
	log_debug(logger, "Instancia para opera:laBoheme: %s", nextInst.nombre);

	exit(0);
}

void testEL(void) {
	ALGORITMO_DISTRIBUCION = EL;
	Instancia inst1 = { .disponible = true };
	strcpy(inst1.nombre, "Instancia 1");
	Instancia inst2 = { .disponible = true };
	strcpy(inst2.nombre, "Instancia 2");
	Instancia inst3 = { .disponible = false };
	strcpy(inst3.nombre, "Instancia 3");
	Instancia inst4 = { .disponible = true };
	strcpy(inst4.nombre, "Instancia 4");

	agregar_instancia(&instancias, inst1, cantidad_instancias + 1);
	cantidad_instancias++;
	agregar_instancia(&instancias, inst2, cantidad_instancias + 1);
	cantidad_instancias++;

	Instancia nextInst = getInstanciaSet("asd");
	log_debug(logger, "Próxima instancia: %s", nextInst.nombre);

	nextInst = getInstanciaSet("asd");
	log_debug(logger, "Próxima instancia: %s", nextInst.nombre);

	nextInst = getInstanciaSet("asd");
	log_debug(logger, "Próxima instancia: %s", nextInst.nombre);

	agregar_instancia(&instancias, inst3, cantidad_instancias + 1);
	cantidad_instancias++;

	nextInst = getInstanciaSet("asd");
	log_debug(logger, "Próxima instancia: %s", nextInst.nombre);

	nextInst = getInstanciaSet("asd");
	log_debug(logger, "Próxima instancia: %s", nextInst.nombre);

	nextInst = getInstanciaSet("asd");
	log_debug(logger, "Próxima instancia: %s", nextInst.nombre);

	agregar_instancia(&instancias, inst4, cantidad_instancias + 1);
	cantidad_instancias++;
	redistribuir_claves();

	nextInst = getInstanciaSet("asd");
	log_debug(logger, "Próxima instancia: %s", nextInst.nombre);

	nextInst = getInstanciaSet("asd");
	log_debug(logger, "Próxima instancia: %s", nextInst.nombre);

	loggear("asd");

	exit(0);
}
