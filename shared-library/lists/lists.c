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

			while (puntero->esi.id != esi.id) {
				puntero = puntero->sgte;
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
	nodo->instancia.nombre = instancia.nombre;
	nodo->instancia.disponible = instancia.disponible;
	nodo->instancia.espacio_usado = instancia.disponible;
	nodo->instancia.veces_llamado = instancia.veces_llamado;
	nodo->instancia.claves = instancia.claves;
	nodo->sgte = NULL;

	return nodo;
}

void destruir_nodo_instancia(t_instancia_node* nodo) {
	free(nodo);
}

void agregar_instancia(t_instancia_list* lista, Instancia instancia) {

	t_instancia_node* nodo = crear_nodo_instancia(instancia);

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
		t_instancia_node* head = instancia_head(*lista);

		if (mismoString(instancia.nombre, head->instancia.nombre)) {
			t_instancia_node* eliminado = lista->head;
			lista->head = lista->head->sgte;
			destruir_nodo_instancia(eliminado);
		}

		else {
			t_instancia_node* puntero = lista->head;

			while (!mismoString(puntero->instancia.nombre,
					head->instancia.nombre)) {
				puntero = puntero->sgte;
			}

			t_instancia_node* eliminado = puntero->sgte;
			puntero->sgte = eliminado->sgte;
			destruir_nodo_instancia(eliminado);
		}
	}
}

t_instancia_node* instancia_head(t_instancia_list lista) {
	t_instancia_node* instancia = lista.head;

	return instancia;
}

/*
 * ==============================
 * =====       CLAVES       =====
 * ==============================
 */

t_clave_node* crear_nodo_clave(char* clave, uint32_t id) {
	t_clave_node* nodo = (t_clave_node*) malloc(sizeof(t_clave_node));
	nodo->clave = clave;
	nodo->block_id = id;
	nodo->sgte = NULL;

	return nodo;
}

void destruir_nodo_clave(t_clave_node* nodo) {
	free(nodo);
}

void agregar_clave(t_clave_list* lista, char* clave, uint32_t id) {
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

void eliminar_clave(t_clave_list* lista, char* clave) {
	if (lista->head != NULL) {
		char* head = headClaves(*lista);
		if (strcmp(clave, head) == 0) {
			t_clave_node* eliminado = lista->head;
			lista->head = lista->head->sgte;
			destruir_nodo_clave(eliminado);
		} else {
			t_clave_node* puntero = lista->head;

			while (!strcmp(puntero->clave, clave) != 0) {
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

/*
 * ==============================
 * =====      BLOCKED       =====
 * ==============================
 */

t_blocked_node* crear_nodo_blocked(blocked bloqueado) {
	t_blocked_node* nodo = (t_blocked_node*) malloc(sizeof(t_blocked_node));
	nodo->clave = bloqueado.clave;
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
