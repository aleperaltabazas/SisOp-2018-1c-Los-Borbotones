/*
 * lists.c
 *
 *  Created on: 15 jul. 2018
 *      Author: alesaurio
 */

#include "lists.h"

/*
 * ==============================
 * =====        ESIs        =====
 * ==============================
 */

t_esi_node* crear_nodo_ESI(ESI esi) {
	t_esi_node* nodo = (t_esi_node*) malloc(sizeof(t_esi_node));
	nodo->esi = esi;
	nodo->sgte = NULL;

	return nodo;
}

void agregar_ESI(t_esi_list* lista, ESI esi) {
	t_esi_node* nodo = crear_nodo_ESI(esi);

	if (lista->head == NULL) {
		lista->head = nodo;
	} else {
		t_esi_node* puntero = lista->head;
		while (puntero->sgte != NULL) {
			puntero = puntero->sgte;
		}

		puntero->sgte = nodo;
	}

	return;
}

void destruir_nodo_ESI(t_esi_node* nodo) {
	free(nodo);
}

void eliminar_ESI(t_esi_list* lista, ESI esi) {
	if (lista->head != NULL) {
		ESI head = headESIs(*lista);
		if (esi.id == head.id) {
			t_esi_node* eliminado = lista->head;
			lista->head = lista->head->sgte;
			destruir_nodo_ESI(eliminado);
		} else {
			t_esi_node* puntero = lista->head;

			while (puntero->sgte->esi.id != esi.id) {
				if (puntero->esi.id == esi.id) {
					break;
				}

				puntero = puntero->sgte;
			}

			if (puntero == NULL) {
				log_warning(logger, "No se encontró el elemento en la lista.");
				return;
			}

			t_esi_node* eliminado = puntero->sgte;
			puntero->sgte = eliminado->sgte;
			destruir_nodo_ESI(eliminado);
		}
	}
}

ESI headESIs(t_esi_list lista) {
	ESI esi = lista.head->esi;

	return esi;

}

/*
 * ==============================
 * =====     INSTANCIAS     =====
 * ==============================
 */

t_instancia_node* crear_nodo_instancia(Instancia instancia) {
	t_instancia_node* nodo = (t_instancia_node*) malloc(
			sizeof(t_instancia_node));
	nodo->instancia.sockfd = instancia.sockfd;
	strcpy(nodo->instancia.nombre, instancia.nombre);
	nodo->instancia.disponible = instancia.disponible;
	nodo->instancia.espacio_usado = instancia.espacio_usado;
	nodo->instancia.veces_llamado = instancia.veces_llamado;
	nodo->instancia.claves = instancia.claves;
	nodo->sgte = NULL;

	return nodo;
}

void destruir_nodo_instancia(t_instancia_node* nodo) {
	free(nodo);
}

void agregar_instancia(t_instancia_list* lista, Instancia instancia, int index) {

	t_instancia_node* nodo = crear_nodo_instancia(instancia);
	nodo->index = index;
	if (lista->head == NULL) {
		lista->head = nodo;
	} else {
		t_instancia_node* puntero = lista->head;
		while (puntero->sgte != NULL) {
			puntero = puntero->sgte;
		}

		puntero->sgte = nodo;
	}

}

void eliminar_instancia(t_instancia_list* lista, Instancia instancia) {
	if (lista->head != NULL) {
		Instancia head = headInstancias(*lista);

		if (mismoString(instancia.nombre, head.nombre)) {
			t_instancia_node* eliminado = lista->head;
			lista->head = lista->head->sgte;
			destruir_nodo_instancia(eliminado);
		}

		else {
			t_instancia_node* puntero = lista->head;

			while (!mismoString(puntero->instancia.nombre, head.nombre)) {
				puntero = puntero->sgte;
			}

			t_instancia_node* eliminado = puntero->sgte;
			puntero->sgte = eliminado->sgte;
			destruir_nodo_instancia(eliminado);
		}
	}
}

Instancia headInstancias(t_instancia_list lista) {
	Instancia instancia = lista.head->instancia;

	return instancia;
}

/*
 * ==============================
 * =====       CLAVES       =====
 * ==============================
 */

t_clave_node* crear_nodo_clave(char* clave, uint32_t id) {

	t_clave_node* nodo = (t_clave_node*) malloc(sizeof(t_clave_node));

	strcpy(nodo->clave, clave);
	strcpy(nodo->valor, "null");
	nodo->block_id = id;
	nodo->sgte = NULL;

	return nodo;
}

void destruir_nodo_clave(t_clave_node* nodo) {
	if (nodo != NULL)
		free(nodo);
}

void agregar_clave(t_clave_list* lista, char* clave, uint32_t id) {

	if (contiene_la_clave(lista, clave)) {
		return;
	}

	t_clave_node* nodo = crear_nodo_clave(clave, id);

	if (lista->head == NULL) {
		lista->head = nodo;
	}

	else {

		t_clave_node* puntero = lista->head;
		while (puntero->sgte != NULL) {
			puntero = puntero->sgte;
		}

		puntero->sgte = nodo;
	}
}

int contiene_la_clave(t_clave_list * lista, char * clave) {
	t_clave_node * auxiliar = lista->head;
	while (auxiliar != NULL) {
		if (mismoString(auxiliar->clave, clave)) {
			return 1;
		}
		auxiliar = auxiliar->sgte;
	}

	return 0;
}

void eliminar_clave(t_clave_list* lista, char* clave) {
	if (lista->head != NULL) {
		char* head = headClaves(*lista);
		if (mismoString(head, clave)) {
			t_clave_node* eliminado = lista->head;
			lista->head = lista->head->sgte;
			destruir_nodo_clave(eliminado);

		} else {
			t_clave_node* puntero = lista->head;

			while (puntero->sgte != NULL) {

				if (mismoString(puntero->sgte->clave, clave)) {
					break;
				}

				puntero = puntero->sgte;
			}

			t_clave_node* eliminado = puntero->sgte;
			puntero->sgte = eliminado->sgte;
			destruir_nodo_clave(eliminado);
		}
	}
}

char* headClaves(t_clave_list lista) {
	char* key = lista.head->clave;

	return key;
}

bool emptyClaves(t_clave_list lista) {
	return lista.head == NULL;
}

void claveListDestroy(t_clave_list* lista) {
	t_clave_node* puntero;

	if (lista->head == NULL) {
		return;
	}

	while (lista->head != NULL) {
		puntero = lista->head;
		lista->head = puntero->sgte;
		free(puntero);
	}

	lista->head = NULL;
}

/*
 * ==============================
 * =====      BLOCKED       =====
 * ==============================
 */

t_blocked_node* crear_nodo_blocked(blocked bloqueado) {
	t_blocked_node* nodo = (t_blocked_node*) malloc(sizeof(t_blocked_node));
	strcpy(nodo->clave, bloqueado.clave);
	nodo->id = bloqueado.id;
	nodo->sgte = NULL;

	return nodo;
}

void destruir_nodo_blocked(t_blocked_node* nodo) {
	free(nodo);
}

void agregar_blocked(t_blocked_list* lista, blocked bloqueado) {
	t_blocked_node* nodo = crear_nodo_blocked(bloqueado);

	if (lista->head == NULL) {
		lista->head = nodo;
	} else {
		t_blocked_node* puntero = lista->head;
		while (puntero->sgte != NULL) {
			puntero = puntero->sgte;
		}

		puntero->sgte = nodo;
	}
}

void eliminar_blocked(t_blocked_list* lista, uint32_t id) {
	if (lista->head != NULL) {
		uint32_t head = headBlockedID(*lista);
		if (id == head) {
			t_blocked_node* eliminado = lista->head;
			lista->head = lista->head->sgte;
			destruir_nodo_blocked(eliminado);
		} else {
			t_blocked_node* puntero = lista->head;

			while (puntero->sgte->id != id) {
				puntero = puntero->sgte;
			}

			t_blocked_node* eliminado = puntero->sgte;
			puntero->sgte = eliminado->sgte;
			destruir_nodo_blocked(eliminado);
		}
	}
}

bool emptyBlocked(t_blocked_list* lista) {
	return lista->head == NULL;
}

void eliminar_blockeados(t_blocked_list * lista) {
	if (!emptyBlocked(lista)) {
		t_blocked_node* eliminado = lista->head;
		lista->head = lista->head->sgte;
		destruir_nodo_blocked(eliminado);
	}
}

uint32_t headBlockedID(t_blocked_list lista) {
	t_blocked_node* head = lista.head;

	return head->id;
}

/*
 * ==============================
 * =====       PARSED       =====
 * ==============================
 */

t_parsed_node* crear_nodo_parsed(t_esi_operacion parsed) {
	t_parsed_node* nodo = (t_parsed_node*) malloc(sizeof(t_parsed_node));
	nodo->esi_op = parsed;
	nodo->sgte = NULL;

	return nodo;
}

void destruir_nodo_parsed(t_parsed_node* nodo) {
	destruir_operacion(nodo->esi_op);
	if (nodo != NULL) {
		free(nodo);
	}

}

void agregar_parseo(t_parsed_list* lista, t_esi_operacion parsed) {
	t_parsed_node* nodo = crear_nodo_parsed(parsed);

	if (lista->head == NULL) {
		lista->head = nodo;
	} else {
		t_parsed_node* puntero = lista->head;
		while (puntero->sgte != NULL) {
			puntero = puntero->sgte;
		}

		puntero->sgte = nodo;
	}

	return;
}

void eliminar_parseo(t_parsed_list* lista) {
	if (!emptyParsed(lista)) {
		t_parsed_node* eliminado = lista->head;
		lista->head = lista->head->sgte;
		destruir_nodo_parsed(eliminado);
	}
}

t_esi_operacion headParsed(t_parsed_list lista) {
	t_esi_operacion parsed = lista.head->esi_op;

	return parsed;
}

bool emptyParsed(t_parsed_list* lista) {
	return lista->head == NULL;
}

/*
 * ==============================
 * =====    DESBLOQUEADO    =====
 * ==============================
 */

t_desbloqueado_node* crear_nodo_desbloqueado(uint32_t id) {
	t_desbloqueado_node* nodo = (t_desbloqueado_node*) malloc(
			sizeof(t_desbloqueado_node));
	nodo->id = id;
	nodo->sgte = NULL;

	return nodo;
}

void destruir_nodo_desbloqueado(t_desbloqueado_node* nodo) {
	free(nodo);
}

void agregar_desbloqueado(t_desbloqueado_list* lista, uint32_t id) {
	t_desbloqueado_node* nodo = crear_nodo_desbloqueado(id);

	if (lista->head == NULL) {
		lista->head = nodo;
	} else {
		t_desbloqueado_node* puntero = lista->head;
		while (puntero->sgte != NULL) {
			puntero = puntero->sgte;
		}

		puntero->sgte = nodo;
	}

	return;
}

void eliminar_desbloqueado(t_desbloqueado_list* lista) {
	if (!emptyDesbloqueado(lista)) {
		t_desbloqueado_node* eliminado = lista->head;
		lista->head = lista->head->sgte;
		destruir_nodo_desbloqueado(eliminado);
	}
}

void eliminarDesbloqueadoPorID(t_desbloqueado_list* lista, uint32_t id) {
	if (lista->head != NULL) {
		uint32_t head = headDesbloqueado(*lista);
		if (id == head) {
			t_desbloqueado_node* eliminado = lista->head;
			lista->head = lista->head->sgte;
			destruir_nodo_desbloqueado(eliminado);
		} else {
			t_desbloqueado_node* puntero = lista->head;

			while (puntero->sgte->id != id) {
				puntero = puntero->sgte;
			}

			t_desbloqueado_node* eliminado = puntero->sgte;
			puntero->sgte = eliminado->sgte;
			destruir_nodo_desbloqueado(eliminado);
		}
	}
}

bool contieneDesbloqueado(t_desbloqueado_list lista, uint32_t id) {
	t_desbloqueado_node* puntero = lista.head;

	while (puntero != NULL) {
		log_error(logger, "ID: %i", id);
		log_error(logger, "Puntero: %i", puntero->id);

		if (puntero->id == id) {
			return true;
		}

		puntero = puntero->sgte;
	}

	return false;
}

bool emptyDesbloqueado(t_desbloqueado_list* lista) {
	return lista->head == NULL;
}

uint32_t headDesbloqueado(t_desbloqueado_list lista) {
	t_desbloqueado_node* head = lista.head;

	return head->id;
}

void show_desbloqueados(t_desbloqueado_list lista) {
	t_desbloqueado_node* puntero = lista.head;

	if (lista.head == NULL) {
		log_error(logger, "NO HAY DESBLOQUEADOS");
	}

	while (puntero != NULL) {
		log_debug(logger, "ID: %i", puntero->id);
		puntero = puntero->sgte;
	}
}

/*
 * ==============================
 * =====      DEADLOCK      =====
 * ==============================
 */

t_deadlock_node* crear_nodo_deadlock(deadlock esi) {
	t_deadlock_node* nodo = (t_deadlock_node*) malloc(sizeof(t_deadlock_node));
	nodo->esi.id = esi.id;
	strcpy(nodo->esi.claveBloqueo, esi.claveBloqueo);
	nodo->esi.clavesTomadas = esi.clavesTomadas;
	nodo->sgte = NULL;

	return nodo;
}

void destruir_nodo_deadlock(t_deadlock_node* node) {
	free(node);
}

void agregar_deadlock(t_deadlock_list* lista, deadlock esi) {
	t_deadlock_node* nodo = crear_nodo_deadlock(esi);

	if (lista->head == NULL) {
		lista->head = nodo;
	} else {
		t_deadlock_node* puntero = lista->head;
		while (puntero->sgte != NULL) {
			puntero = puntero->sgte;
		}

		puntero->sgte = nodo;
	}

	lista->size++;
	return;
}

void eliminar_deadlock(t_deadlock_list* lista, deadlock esi) {
	if (lista->head != NULL) {
		deadlock head = headDeadlock(*lista);
		if (head.id == esi.id) {
			t_deadlock_node* eliminado = lista->head;
			lista->head = lista->head->sgte;
			destruir_nodo_deadlock(eliminado);
		} else {
			t_deadlock_node* puntero = lista->head;

			while (puntero->sgte->esi.id != esi.id) {
				puntero = puntero->sgte;
			}

			t_deadlock_node* eliminado = puntero->sgte;
			puntero->sgte = eliminado->sgte;
			destruir_nodo_deadlock(eliminado);
		}
	}

	lista->size--;
}

void deadlockListDestroy(t_deadlock_list* lista) {
	t_deadlock_node* puntero;

	while (lista->head != NULL) {
		puntero = lista->head;
		lista->head = lista->head->sgte;
		free(puntero);
	}

	lista->head = NULL;
	lista->size = 0;
}

deadlock headDeadlock(t_deadlock_list lista) {
	deadlock esi = lista.head->esi;

	return esi;
}

bool deadlockListContains(t_deadlock_list lista, uint32_t id) {
	t_deadlock_node* puntero = lista.head;

	while (puntero != NULL) {
		if (puntero->esi.id == id) {
			return true;
		}

		puntero = puntero->sgte;
	}

	return false;
}

bool isEmptyDeadlock(t_deadlock_list lista) {
	return lista.head == NULL;
}

int deadlockLength(t_deadlock_list lista) {
	return lista.size;
}
