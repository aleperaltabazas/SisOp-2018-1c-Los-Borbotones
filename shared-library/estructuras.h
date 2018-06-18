/*
 * estructuras.h
 *
 *  Created on: 16 jun. 2018
 *      Author: utnso
 */

#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/config.h>

//	Misc

typedef struct ESI {
	int id;
	int socket;
	int rafaga_estimada;
	int rafaga_real;
	int tiempo_arribo;
	bool ejecutando;
} ESI;

typedef struct entrada {
	char clave[40];
	uint32_t pos_valor;
	uint32_t tamanio_valor;
} entrada;

//	Estructuras con serialización

typedef struct aviso_ESI {
	uint32_t aviso;
	uint32_t id;
}__attribute__((packed)) aviso_ESI;

typedef struct package_int {
	uint32_t packed;
}__attribute__((packed)) package_int;

typedef struct orden_del_coordinador {
	uint32_t codigo_operacion;
	uint32_t tamanio_a_enviar;
}__attribute__((packed)) orden_del_coordinador;

typedef struct parametros_set {
	uint32_t tamanio_clave;
	char * clave;
	uint32_t tamanio_valor;
	char * valor;
}__attribute__((packed)) parametros_set;

typedef struct key {
	char* clave;
	int clave_size;
}__attribute__((packed)) key;

//	Algoritmos

typedef struct algoritmo_planificacion {
	enum {
		FIFO, SJF, HRRN,
	} tipo;
	bool desalojo;
} algoritmo_planificacion;

typedef enum {
	LSU, EL, KE
} algoritmo_distribucion;

typedef enum {
	CIRC, LRU, BSU
} algoritmo_reemplazo;

//	Listas

typedef struct t_esi_node {
	int index;
	ESI esi;
	struct t_esi_node* sgte;
} t_esi_node;

typedef struct t_esi_list {
	t_esi_node* head;
} t_esi_list;

typedef struct t_parsed_node {
	int index;
	t_esi_operacion esi_op;
	struct t_parsed_node* sgte;
} t_parsed_node;

typedef struct t_parsed_list {
	t_parsed_node* head;
} t_parsed_list;

typedef struct t_clave_node {
	char* clave;
	char* valor;
	int block_id;
	struct t_clave_node* sgte;
} t_clave_node;

typedef struct t_clave_list {
	t_clave_node* head;
} t_clave_list;

#endif /* ESTRUCTURAS_H_ */
