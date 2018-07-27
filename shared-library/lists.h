/*
 * lists.h
 *
 *  Created on: 15 jul. 2018
 *      Author: alesaurio
 */

#ifndef LISTS_H_
#define LISTS_H_

#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include "estructuras.h"
#include "strings.h"

/*
 * ==============================
 * =====        ESIs        =====
 * ==============================
 */

t_esi_node* crear_nodo_ESI(ESI esi);
	/*
	 * Descripción: reserva memoria y crea un nodo t_esi_node* con los datos atributos contenidos en esi.
	 * Argumentos:
	 * 		ESI esi: esi de donde tomar los atributos.
	 */

void destruir_nodo_ESI(t_esi_node* nodo);
	/*
	 * Descripción: libera la memoria contenida en el nodo.
	 * Argumentos:
	 * 		t_esi_node* nodo: el nodo a liberar.
	 */

void agregar_ESI(t_esi_list* lista, ESI esi);
	/*
	 * Descripción: agrega un ESI a la lista.
	 * Argumentos:
	 * 		t_esi_list* lista: lista a agregar el elemento.
	 * 		ESI esi: esi a agregar a la lista.
	 */

void eliminar_ESI(t_esi_list* lista, ESI esi);
	/*
	 * Descripción: elimina un ESI de una lista.
	 * Argumentos:
	 * 		t_esi_list* lista: lista de la cual se eliminará el elemento.
	 * 		ESI esi: elemento a eliminar de la lista.
	 */

ESI headESIs(t_esi_list lista);
	/*
	 * Descripción: devuelve el primer elemento de la lista.
	 * Argumentos:
	 * 		t_esi_list* lista: lista a obtener el elemento.
	 */

/*
 * ==============================
 * =====     INSTANCIAS     =====
 * ==============================
 */

t_instancia_node* crear_nodo_instancia(Instancia instancia);
	/*
	 * Descripción: reserva memoria y crea un t_instancia_node con los atributos contenidos en instancia.
	 * Argumentos:
	 * 		Instancia instancia: instancia de donde tomar los atributos.
	 */

void destruir_nodo_instancia(t_instancia_node* nodo);
	/*
	 * Descripción: libera la memoria contenida en el nodo.
	 * Argumentos:
	 * 		t_instancia_node* nodo: el nodo a liberar.
	 */

void agregar_instancia(t_instancia_list* lista, Instancia instancia, int index);
	/*
	 * Descripción: agrega una instancia a la una lista de instancias.
	 * Argumentos:
	 * 		t_instancia_list* lista: la lista a la cual agregar la instancia.
	 * 		Instancia instancia: la instancia a agregar.
	 */

void eliminar_instancia(t_instancia_list* lista, Instancia instancia);
	/*
	 * Descripción: elimina la instancia de la lista.
	 * Argumentos:
	 * 		t_instancia_list* lista: la lista de la cual remover el elemento.
	 * 		Instancia instancia: la instancia a eliminar.
	 */

Instancia headInstancias(t_instancia_list lista);
	/*
	 * Descripción: devuelve el primer elemento de la lista.
	 * Argumentos:
	 * 		t_instancia_list lista: lista a obtener el elemento.
	 */

/*
 * ==============================
 * =====       CLAVES       =====
 * ==============================
 */

t_clave_node* crear_nodo_clave(char* clave, uint32_t id);
	/*
	 * Descripción: crea un nodo con la clave.
	 * Argumentos:
	 * 		char* clave: la clave a poner en el nodo.
	 */

void destruir_nodo_clave(t_clave_node* nodo);
	/*
	 * Descripción: libera la memoria del nodo.
	 * Argumentos:
	 * 		t_clave_node* nodo: puntero a memoria a liberar.
	 */

void agregar_clave(t_clave_list* lista, char* clave, uint32_t id);
	/*
	 * Descripción: agrega una clave a la lista.
	 * Argumentos:
	 * 		t_clave_list* lista: lista a la cual agregar la clave.
	 * 		char* clave: la clave a agregar.
	 */

void eliminar_clave(t_clave_list* lista, char* clave);
	/*
	 * Descripción: elimina una clave de la lista.
	 * Argumentos:
	 * 		t_clave_list* lista: lista de la cual eliminar la clave.
	 * 		char* clave: la clave a eliminar.
	 */

char* headClaves(t_clave_list lista);
	/*
	 * Descripción: retorna la primer clave de una lista.
	 * Argumentos:
	 * 		t_clave_list lista: lista de la cual tomar el elemento.
	 */

int contiene_la_clave(t_clave_list * lista, char * clave);
	/*
	 * Descripción: devuelve si una lista contiene la clave.
	 * Argumentos:
	 * 		t_clave_list* lista: la lista de donde buscar.
	 * 		char* clave: la clave a buscar.
	 */

bool emptyClaves(t_clave_list lista);
	/*
	 * Descripción: devuelve si la cabeza de la lista es igual a NULL.
	 * Argumentos:
	 * 		t_clave_list lista
	 */

void eliminar_blockeados(t_blocked_list * lista);

/*
 * ==============================
 * =====       PARSED       =====
 * ==============================
 */

t_parsed_node* crear_nodo_parsed(t_esi_operacion parsed);
	/*
	 * Descripción: reserva memoria y crea un nodo para un t_esi_operacion con los atributos de parsed.
	 * Argumentos:
	 * 		t_esi_operacion parsed: el t_esi_operacion de donde tomar los atributos.
	 */

void destruir_nodo_parsed(t_parsed_node* nodo);
	/*
	 * Descripción: libera la memoria ocupada por un nodo.
	 * Argumentos:
	 * 		t_parsed_node* nodo: puntero del nodo a liberar.
	 */

void agregar_parseo(t_parsed_list* lista, t_esi_operacion parsed);
	/*
	 * Descripción: agrega una línea parseada a una lista del tipo t_parsed_list.
	 * Argumentos:
	 * 		t_parsed_list* lista: lista a la cual agregar el parseo.
	 * 		t_esi_operacion parsed: línea a agregar a la lista.
	 */

void eliminar_parseo(t_parsed_list* lista);
	/*
	 * Descripción: elimina el primer nodo de la lista.
	 * Argumentos:
	 * 		t_parsed_list* lista: lista de la cual eliminar el nodo.
	 */

t_esi_operacion headParsed(t_parsed_list lista);
	/*
	 * Descripción: devuelve el primer elemento de una lista.
	 * Argumentos:
	 * 		t_parsed_list lista: lista de la cual obtener el elemento.
	 */

bool emptyParsed(t_parsed_list* lista);
	/*
	 * Descripción: devuelve si la lista se encuentra vacía.
	 * Argumentos:
	 * 		t_parsed_list* lista: lista a verificar.
	 */

void claveListDestroy(t_clave_list* lista);
	/*
	 * Descripción: elimina todos los elementos de la lista, liberando toda la memoria ocupada por la misma.
	 * Argumentos:
	 * 		t_clave_list* lista
	 */

/*
 * ==============================
 * =====      BLOCKED       =====
 * ==============================
 */

t_blocked_node* crear_nodo_blocked(blocked bloqueado);
	/*
	 * Descripción: crea y retorna un nodo de blocked.
	 * Argumentos:
	 * 		blocked bloqueado: estructura que contiene los datos a volcar en el nodo.
	 */

void destruir_nodo_blocked(t_blocked_node* nodo);
	/*
	 * Descripción: libera la porción de memoria de un nodo.
	 * Argumentos:
	 * 		t_blocked_node* nodo: la memoria a liberar.
	 */

void agregar_blocked(t_blocked_list* lista, blocked bloqueado);
	/*
	 * Descripción: agrega un blocked a una lista de blockeds.
	 * Argumentos:
	 * 		t_blocked_list* lista: la lista a la cual agregar el bloqueado.
	 * 		blocked bloqueado: el bloqueado.
	 */

void eliminar_blocked(t_blocked_list* lista, uint32_t id);
	/*
	 * Descripción: elimina un nodo de la lista a partir de un id.
	 * Argumentos:
	 * 		t_blocked_list* lista: la lista a modificar.
	 * 		uint32_t id: el id del nodo a eliminar.
	 */

uint32_t headBlockedID(t_blocked_list lista);
	/*
	 * Descripción: devuelve el id del primer elemento de una lista de bloqueados.
	 * Argumentos:
	 * 		t_blocked_list lista: la lista en cuestión.
	 */

t_log * logger;

/*
 * ==============================
 * =====    DESBLOQUEADO    =====
 * ==============================
 */

t_desbloqueado_node* crear_nodo_desbloqueado(uint32_t id);
	/*
	 * Descripción: crea y retorna un nodo de desbloqueado.
	 * Argumentos:
	 * 		uint32_t id: ID a copiar en el nodo.
	 */

void destruir_nodo_desbloqueado(t_desbloqueado_node* nodo);
	/*
	 * Descripción: libera la porción de memoria de un nodo.
	 * Argumentos:
	 * 		t_desbloqueado_node*: el nodo a liberar.
	 */

void agregar_desbloqueado(t_desbloqueado_list* lista, uint32_t id);
	/*
	 * Descripción: agrega un desbloqueado a la lista.
	 * Argumentos:
	 * 		t_desbloqueado_list* lista
	 * 		uint32_t id
	 */

void eliminar_desbloqueado(t_desbloqueado_list* lista);
	/*
	 * Descripción: elimina el primer nodo de la lista.
	 * Argumentos:
	 * 		t_desbloqueado_list* lista
	 */

bool emptyDesbloqueado(t_desbloqueado_list* lista);
	/*
	 * Descripción: devuelve si la cabeza de la lista es NULL.
	 * Argumentos:
	 * 		t_desbloqueado_list* lista
	 */

uint32_t headDesbloqueado(t_desbloqueado_list lista);
	/*
	 * Descripción: devuelve el ID del primer elemento de la lista.
	 * Argumentos:
	 * 		t_desbloqueado_list lista
	 */

/*
 * ==============================
 * =====      DEADLOCK      =====
 * ==============================
 */

t_deadlock_node* crear_nodo_deadlock(deadlock esi);
	/*
	 * Descripción: crea y retorna un nodo de deadlock.
	 * Argumentos:
	 * 		deadlock esi
	 */

void destruir_nodo_deadlock(t_deadlock_node* nodo);
	/*
	 * Descripción: libera la memoria del nodo.
	 * Argumentos:
	 * 		t_deadlock_node* nodo
	 */

void agregar_deadlock(t_deadlock_list* lista, deadlock esi);
	/*
	 * Descripción: agrega un esi a la lista de deadlock.
	 * Argumentos:
	 * 		t_deadlock_list* lista
	 * 		deadlock esi
	 */

void eliminar_deadlock(t_deadlock_list* lista, deadlock esi);
	/*
	 * Descripción: elimina un esi de la lista de deadlock.
	 * Argumentos:
	 * 		t_deadlock_list* lista
	 * 		deadlock esi
	 */

void deadlockListDestroy(t_deadlock_list* lista);
	/*
	 * Descripción: elimina todos los elementos de la lista, liberando toda la memoria ocupada por la misma
	 * Argumentos:
	 * 		t_deadlock_list* lista
	 */

deadlock headDeadlock(t_deadlock_list lista);
	/*
	 * Descripción: devuelve el primer elemento de una lista de deadlock
	 * Argumentos:
	 * 		t_deadlock_list lista
	 */

bool isEmptyDeadlock(t_deadlock_list lista);
	/*
	 * Descripción: indica si la cabeza de la lista es igual a NULL.
	 * Argumentos:
	 * 		t_deadlock_list lista
	 */

int deadlockLength(t_deadlock_list lista);
	/*
	 * Descripción: devuelve la longitud de una lista de deadlock.
	 * Argumentos:
	 * 		t_deadlock_list lista
	 */

/*
 * ==============================
 * =====      ENTRADAS      =====
 * ==============================
 */

#endif /* LISTS_H_ */
