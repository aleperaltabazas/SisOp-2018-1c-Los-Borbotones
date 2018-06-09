/*
 * planificador.h
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <shared-library.h>

#define ALFA 50

//Estructuras
typedef struct algoritmo {
	enum {
		FIFO, SJF, HRRN,
	} tipo;
	bool desalojo;
} algoritmo;

typedef struct ESI {
	int id;
	int socket;
	int rafaga_estimada;
	int rafaga_real;
	int tiempo_arribo;
} ESI;

typedef struct t_esi_node{
	int index;
	ESI esi;
	struct t_esi_node* sgte;
} t_esi_node;

typedef struct t_esi_list{
	t_esi_node* head;
} t_esi_list;

//Variables locales

int ESI_id;
ESI esi_vacio = {
		.id = 0,
		.socket = 0
};

int tiempo;

t_list * ESIs;
t_list * ESIs_bloqueados;
t_list * ESIs_en_ejecucion;
t_list * ESIs_listos;
t_list * ESIs_finalizados;

ESI executing_ESI;

t_esi_list new_ESIs;

algoritmo algoritmo_planificacion;

pthread_mutex_t semaforo_ejecucion;

pthread_spinlock_t sem_ESIs;
pthread_spinlock_t sem_id;

//Hilos

pthread_t hiloDeConsola;

//Funciones de consola

void* consola(void);
void listarOpciones(void);
void pausarOContinuar(void);
void bloquear(float codigo);
void desbloquear(float codigo);
void listar(void);
void kill(float codigo);
void status(float codigo);
void deadlock(void);
float recibirCodigo(void);
void interpretarYEjecutarCodigo(float comando);

//Funciones de servidor

int manejar_cliente(int listening_socket, int socket_cliente, char* mensaje);
	/*
	 * Descripción: determina qué hacer cuando recibe una nueva conexión a través del
	 * 		socket cliente.
	 * Argumentos:
	 * 		int listening_socket: socket del servidor local.
	 * 		int socket_cliente: socket del cliente.
	 * 		char* mensaje: mensaje para enviar como identificacióna a los nuevos clientes.
	 */

void identificar_cliente(char* mensaje, int socket_cliente);
	/*
	 * Descripción: identifica a un cliente y crea un hilo para el mismo, o llama a salir_con_error().
	 * 		en caso de ser un cliente desconocido.
	 * Argumentos:
	 * 		char* mensaje: el mensaje enviado por el cliente.
	 * 		int socket_cliente: socket del cliente.
	 */

//Funciones de hilos

void* atender_ESI(void* sockfd);
	/*
	 * Descripción: atiende los mensajes enviados de un ESI y le avisa que ejecute
	 * 		cuando se le indique.
	 * Argumentos:
	 * 		void* sockfd: descriptor de socket que luego se castea a int.
	 */

//Funciones

void iniciar(void);
	/*
	 * Descripción: crea el logger y las listas de ESIs, y carga los datos del archivo
	 * 		de configuración en variables globales.
	 * Argumentos:
	 * 		void
	 */

void planificar(void);
	/*
	 * Descripción: decide cuál es el siguiente ESI a ejecutar, dependiendo del algoritmo
	 * 		que se use en el momento.
	 * Argumentos:
	 * 		void
	 */

void desalojar(void);
	/*
	 * Descripción: avisa al hilo que está ejecutando actualmente que su ESI asociado ya no ejecuta más.
	 * Argumentos:
	 * 		void
	 */

void ejecutar(ESI esi_a_ejecutar);
	/*
	 * Descripción: avisa a un proceso ESI que ejecute a través de su socket.
	 * Argumentos:
	 * 		ESI* esi_a_ejecutar: proximo ESI a ejecutar.
	 */

void deserializar_esi(void* esi_copiado, ESI* esi_receptor);
	/*
	 * Descripción: copia un buffer de memoria a un tipo ESI*.
	 * Argumentos:
	 * 		void* esi_copiado: el buffer de memoria donde se contiene la información.
	 * 		ESI* esi_receptor: el recipiente de la información del buffer.
	 */

void cerrar(void);
	/*
	 * Descripción: llama a cerrar_listas() y libera la variable executing_ESI.
	 * Argumentos:
	 * 		void
	 */

void cerrar_listas(void);
	/*
	 * Descripción: cierra las distintas listas de ESIs y libera su memoria.
	 * Argumentos:
	 * 		void
	 */

int asignar_ID(ESI esi);
	/*
	 * Descripción: asigna un ID a un proceso ESI y devuelve el mismo valor como identificador.
	 * Argugmentos:
	 * 		ESI esi: proceso ESI a asignar.
	 */

void kill_ESI(ESI esi);
	/*
	 * Descripción: le indica a un ESI que termine.
	 * Argumentos:
	 * 		ESI esi: esi a terminar.
	 */

ESI first(t_esi_list lista);
	/*
	 * Descripción: devuelve el primer elemento de la lista.
	 * Argumentos:
	 * 		t_esi_list* lista: lista a obtener el elemento.
	 */

ESI shortest(t_esi_list lista);
	/*
	 * Descripción: devuelve el elemento cuyo tiempo de ejecución es menor.
	 * Argumentos:
	 * 		t_esi_list* lista: lista a obtener el elemento.
	 */

ESI highest_RR(t_esi_list lista);
	/*
	 * Descripción: devuelve el elemento cuyo RR es mayor.
	 * Argumentos:
	 * 		t_esi_list* lista: lista a obtener el elemento.
	 */

void agregar_ESI(t_esi_list* lista, ESI esi);
	/*
	 * Descripción: agrega un ESI a la lista.
	 * Argumentos:
	 * 		t_esi_list* lista: lista a agregar el elemento.
	 * 		ESI esi: esi a agregar a la lista.
	 */

bool tiene_mas_RR(ESI primer_ESI, ESI segundo_ESI);
	/*
	 * Descripción: devuelve si el segundo ESI tiene mayor RR que el primero.
	 * Argumentos:
	 * 		ESI primer_ESI: ESI a comparar.
	 * 		ESI segundo_ESI: ESI a comparar.
	 */

bool es_mas_corto(ESI primer_ESI, ESI segundo_ESI);
	/*
	 * Descripción: devuelve si el segundo ESI tiene una duración de ráfaga estimada
	 * 		menor al primero.
	 * Argumentos:
	 * 		ESI primer_ESI: ESI a comparar.
	 * 		ESI segundo_ESI: ESI a comparar.
	 */

int wait_time(ESI esi);
	/*
	 * Descripción: devuelve el tiempo de espera de un ESI.
	 * Argumentos:
	 * 		ESI esi: el ESI a obtener su tiempo de espera.
	 */

int estimated_time(ESI esi);
	/*
	 * Descripción: devuelve la duración estimada de ráfaga de un ESI.
	 * Arguentos:
	 * 		ESI esi: el ESI a obtener su duración de ráfaga.
	 */

void charlar_con_el_coordi(void);
	/*
	 * Descripción: mantiene una conexión con el coordinador y envia y recibe un mensaje cada
	 * 		15 segundos para asegurarse que el otro proceso se encuentre en ejecución.
	 * Argumentos:
	 * 		void
	 */

#endif /* PLANIFICADOR_H_ */
