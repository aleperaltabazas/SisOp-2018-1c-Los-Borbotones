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
	t_clave_node* nodo = (t_clave_node*) malloc(
			sizeof(uint32_t) + sizeof(t_clave_node));
	strcpy(nodo->clave, clave);
	nodo->block_id = id;
	nodo->sgte = NULL;

	return nodo;
}

void destruir_nodo_clave(t_clave_node* nodo) {
	free(nodo);
}

void agregar_clave(t_clave_list* lista, char* clave, uint32_t id) {
	if (contiene_la_clave(lista, clave)) {
		return;
	}

	t_clave_node* nodo = crear_nodo_clave(clave, id);

	if (lista->head == NULL) {
		lista->head = nodo;
	} else {
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
	/*if (lista->head != NULL) {
	 char* head = headClaves(*lista);
	 if (strcmp(clave, head) == 0) {
	 t_clave_node* eliminado = lista->head;
	 lista->head = lista->head->sgte;
	 log_warning(logger, "A Puntero: %s, clave: %s", eliminado->clave, clave);
	 destruir_nodo_clave(eliminado);
	 } else {
	 t_clave_node* puntero = lista->head;

	 while (!mismoString(puntero->clave, clave)) {
	 puntero = puntero->sgte;
	 }

	 t_clave_node* eliminado = puntero->sgte;
	 puntero->sgte = eliminado->sgte;
	 log_warning(logger, "B: Puntero: %s, clave: %s", eliminado->clave, clave);
	 destruir_nodo_clave(eliminado);
	 }
	 }*/

	if (lista->head != NULL) {

		char* head = headClaves(*lista);

		if (mismoString(clave, head)) {
			t_clave_node* eliminado = lista->head;
			lista->head = lista->head->sgte;
			log_warning(logger, "A: Puntero: %s, clave: %s", eliminado->clave,
					clave);
			destruir_nodo_clave(eliminado);
		}

		else {

			t_clave_node* puntero = lista->head;
			t_clave_node * aux;

			while (!mismoString(puntero->clave, clave)) {
				aux = puntero;
				puntero = puntero->sgte;
			}

			if (puntero->sgte != NULL) {
				aux->sgte = puntero->sgte;
				log_warning(logger, "B: Puntero: %s, clave: %s", puntero->clave,
						clave);
				destruir_nodo_clave(puntero);
			}

			else {
				aux->sgte = NULL;
				log_warning(logger, "C: Puntero: %s, clave: %s", puntero->clave,
						clave);
				destruir_nodo_clave(puntero);
			}
		}
	}
}

char* headClaves(t_clave_list lista) {
	char* key = lista.head->clave;

	return key;
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

			while (puntero->id != id) {
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

