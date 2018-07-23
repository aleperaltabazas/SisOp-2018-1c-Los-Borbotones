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
#include <stdio.h>
#include <stdint.h>
#include <parsi/parser.h>

//	Misc

typedef struct GET_Op {
	char clave[80];
	uint32_t id;
} GET_Op;

typedef struct SET_Op {
	char clave[80];
	char valor[80];
	uint32_t id;
} SET_Op;

typedef struct STORE_Op {
	char clave[80];
	uint32_t id;
} STORE_Op;

typedef struct ESI {
	uint32_t id;
	int socket;
	float rafaga_estimada;
	int rafaga_real;
	int tiempo_arribo;
	bool ejecutando;
} ESI;

typedef struct entrada {
	char * clave;
	int tiempo_sin_ser_referenciado;
	int pos_valor;
	int tamanio_valor;
} entrada;

typedef struct blocked {
	char clave[40];
	uint32_t id;
} blocked;

typedef struct operacion {
	enum {
		op_GET, op_SET, op_STORE
	} op_type;
	char clave[40];
	char valor[40];
	uint32_t id;
} operacion;

//	Estructuras con serialización

typedef struct aviso_con_ID {
	uint32_t aviso;
	uint32_t id;
}__attribute__((packed)) aviso_con_ID;

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

typedef struct clave_package {
	char* clave;
	uint32_t clave_long;
	uint32_t total_size;
} clave_package;

typedef struct clave_valor_package {
	char* clave;
	uint32_t clave_long;
	char* valor;
	uint32_t valor_long;
	uint32_t total_size;
} clave_valor_package;

typedef package_int op_response;

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
	char clave[40];
	char valor[40];
	uint32_t block_id;
	struct t_clave_node* sgte;
} t_clave_node;

typedef struct t_clave_list {
	t_clave_node* head;
} t_clave_list;

typedef struct t_blocked_node {
	char clave[40];
	uint32_t id;
	struct t_blocked_node* sgte;
} t_blocked_node;

typedef struct t_blocked_list {
	t_blocked_node* head;
} t_blocked_list;

typedef struct entradas_node {
	entrada una_entrada;
	struct entradas_node * siguiente;
} entradas_node;

typedef struct t_entrada_list {
	entradas_node* head;
} t_entrada_list;

typedef struct Instancia {
	int sockfd;
	int veces_llamado;
	int espacio_usado;
	int id;
	bool disponible;
	char nombre[40];
	char keyMin;
	char keyMax;
	t_clave_list claves;
} Instancia;

typedef struct t_instancia_node {
	Instancia instancia;
	int index;
	struct t_instancia_node* sgte;
} t_instancia_node;

typedef struct t_instancia_list {
	t_instancia_node* head;
} t_instancia_list;

#endif /* ESTRUCTURAS_H_ */
