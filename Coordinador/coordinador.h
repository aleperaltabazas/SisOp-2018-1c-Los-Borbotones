/*
 * coordinador.h
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#ifndef COORDINADOR_H_
#define COORDINADOR_H_

#include <shared-library.h>
//#include "shared-library.h"

//Variables globales

char* PUERTO_COORDINADOR;
algoritmo_distribucion ALGORITMO_DISTRIBUCION;
int CANTIDAD_ENTRADAS;
int TAMANIO_ENTRADAS;
float RETARDO;

int seguir_ejecucion = 1;

package_int bloqueo_ok = {
		.packed = 26
};

t_clave_list claves_bloqueadas;
t_clave_list claves_disponibles;

parametros_set valor_set;
char * buffer_parametros;

//Hilos

pthread_t hilo_ESI;
pthread_t hilo_instancia;
pthread_t hilo_planificador;

//Funciones de servidor

int manejar_cliente(int listening_socket, int socketCliente, package_int id);
	/*
	 * Descripción: determina qu� hacer cuando recibe una nueva conexi�n a trav�s del
	 * 		socket cliente.
	 * Argumentos:
	 * 		int listening_socket: socket del servidor local.
	 * 		int socket_cliente: socket del cliente.
	 * 		char* mensaje: mensaje para enviar como identificaci�na a los nuevos clientes.
	 */

void identificar_cliente(package_int id, int socket_cliente);
	/*
	 * Descripción: identifica a un cliente y crea un hilo para el mismo, o llama a salir_con_error().
	 * 		en caso de ser un cliente desconocido.
	 * Argumentos:
	 * 		char* mensaje: el mensaje enviado por el cliente.
	 * 		int socket_cliente: socket del cliente.
	 */

void coordinar(void);
	/*
	 * Descripción: maneja las conexiones mientras que se siga la ejecución.
	 * Argumentos:
	 * 		void
	 */

//Funciones de hilos

void *atender_ESI(void* un_socket);
	/*
	 * Descripción: atiende los mensajes enviados por un proceso ESI y le asigna los recursos
	 * 		o no si se encuentran disponibles.
	 * Argumentos:
	 * 		void* un_socket: descriptor del socket que luego se castea a int.
	 */

void *atender_Instancia(void* un_socket);
	/*
	 * Descripción: atiende los mensajes enviados por un proces de instancia y le indica
	 * 		los resultados de una ejecución de ESI.
	 * Argumentos:
	 * 		void* un_socket: descriptor del socket que luego se castea a int.
	 */

void *atender_Planificador(void* un_socket);
	/*
	 * Descripción: atiende los mensajes enviados por el planificador.
	 * Argumentos:
	 * 		void* un_socket: descriptor del socket que luego se castea a int.
	 */

//Funciones

int chequear_solicitud(int socket_cliente);
	/*
	 * Descripción: revisa una solicitud de ejecución de un proceso ESI y le indica si puede
	 * 		o no ejecutar.
	 * Argumentos:
	 * 		int socket_cliente: socket del proceso ESI.
	 */

void iniciar(void);
	/*
	 * Descripción: carga las configuraciones iniciales del proceso.
	 * Argumentos:
	 * 		void
	 */

void cargar_configuracion(void);
	/*
	 * Descripción: abre el archivo de configuración y carga los valores en variables globales.
	 * Argumentos:
	 * 		void
	 */

algoritmo_distribucion dame_algoritmo(char* algoritmo_src);
	/*
	 * Descripción: devuelve el algoritmo correspondiente a una cadena.
	 * Argumentos:
	 * 		char* algoritmo_src: cadena a tomar el algoritmo.
	 */

float dame_retardo(int retardo);
	/*
	 * Descripción: devuelve en segundos un retardo.
	 * Argumentos:
	 * 		int retardo: la forma entera (en microsegundos) del retardo.
	 */

void bloquear_clave(int socket_cliente);
	/*
	 * Descripción: recibe mensajes de un cliente para bloquear una clave.
	 * Argumentos:
	 * 		int socket_cliente: socket del cliente.
	 */

void bloquear(char* clave, int id);
	/*
	 * Descripción: bloquea una clave. En caso de que no exista, la crea y la bloquea.
	 * 		Si la clave ya se encuentra bloqueada, no hace nada.
	 * Argumentos:
	 * 		char* clave: clave a bloquear.
	 */

void desbloquear_clave(int socket_cliente);
	/*
	 * Descripción: recibe mensajes de un cliente para desbloquear una clave.
	 * Argumentos:
	 * 		int socket_cliente: socket del cliente.
	 */

void desbloquear(char* clave);
	/*
	 * Descripción: desbloquea una clave. En caso que no exista, la crea y la agrega a disponibles.
	 * 		Si la clave ya existe y está disponible, no hace nada.
	 * Argumentos:
	 * 		char* clave: la clave a desbloquear.
	 */

bool existe(char* clave);
	/*
	 * Descripción: devuelve si la clave existe en la lista de claves_bloqueadas o de
	 * 		claves_disponibles.
	 * Argumentos:
	 * 		char* clave: clave a chequear su existencia.
	 */

void crear(char* clave);
	/*
	 * Descripción: agrega una clave a la lista de claves disponibles;
	 * Argumentos:
	 * 		char* clave: la nueva clave.
	 */

bool esta_bloqueada(char* clave);
	/*
	 * Descripción: indica si la clave se encuentra en la lista de claves_bloqueadas.
	 * Argumentos:
	 * 		char* clave: la clave a chequear.
	 */

void eliminar_clave(t_clave_list* lista, char* clave);
	/*
	 * Descripción: elimina una clave de la lista.
	 * Argumentos:
	 * 		t_clave_list* lista: lista de la cual eliminar la clave.
	 * 		char* clave: la clave a eliminar.
	 */

void agregar_clave(t_clave_list* lista, char* clave);
	/*
	 * Descripción: agrega una clave a la lista.
	 * Argumentos:
	 * 		t_clave_list* lista: lista a la cual agregar la clave.
	 * 		char* clave: la clave a agregar.
	 */

t_clave_node* crear_nodo(char* clave);
	/*
	 * Descripción: crea un nodo con la clave.
	 * Argumentos:
	 * 		char* clave: la clave a poner en el nodo.
	 */

void destruir_nodo(t_clave_node* nodo);
	/*
	 * Descripción: libera la memoria del nodo.
	 * Argumentos:
	 * 		t_clave_node* nodo: puntero a memoria a liberar.
	 */

char* first(t_clave_list lista);
	/*
	 * Descripción: retorna la primer clave de una lista.
	 * Argumentos:
	 * 		t_clave_list lista: lista de la cual tomar el elemento.
	 */

void get(int sockfd, int id);
	/*
	 * Descripción: bloquea una clave y se la asigna al id. Si no existe, se crea y se bloquea. Si
	 * 		ya existe y está bloqueada, se bloquea al proceso cliente.
	 * Argumentos:
	 * 		int sockfd: socket del proceso cliente.
	 * 		int id: identificador del proceso cliente.
	 */

void set(int sockfd, int id);
	/*
	 * Descripción: settea un valor en una clave. Si la clave no existe, se bloquea al proceso cliente.
	 * 		Si existe, pero las identificaciones del bloqueante y del proceso cliente no matchean, se
	 * 		bloquea al proceso cliente.
	 * Argumentos:
	 * 		int sockfd: socket del proceso cliente.
	 * 		int id: identificador del proceso cliente.
	 */

void store(int sockfd, int id);
	/*
	 * Descripción: guarda el valor de una clave en una instancia. Si la clave no existe, se bloquea al
	 * 		proceso cliente. Si existe, pero las identificaciones del bloqueante y del proceso cliente
	 * 		no matchean, se	bloquea al proceso cliente.
	 * Argumentos:
	 * 		int sockfd: socket del proceso cliente.
	 * 		int id: identificador del proceso cliente.
	 */

int get_packed(char* clave, int id);
	/*
	 * Descripción: retorna el valor de la operación dependiendo de la existencia de la clave o el resultado
	 * 		de la verificación de las identificaciones.
	 * Argumentos:
	 * 		char* clave: la cadena a verificar.
	 * 		int id: la identificación a comparar.
	 */

int get_clave_id(char* clave);
	/*
	 * Descripción: retorna el id de quien bloqueó la clave. En caso de que la clave no exista, retorna -1.
	 * Argumentos:
	 * 		char* clave: la clave a verificar.
	 */

int settear(char* valor, char* clave, int id);
	/*
	 * Descripción: realiza las verificaciones sobre la clave y el id. En caso negativo, no hace nada y devuelve
	 * 		5. En caso positivo, asigna el valor a la clave y devuelve 20.
	 */

void hacer_store(char* clave);
	/*
	 * Descripción: realiza el guardado de la clave en la instancia.
	 * Argumentos:
	 * 		char* clave: la clave a guardar.
	 */

int dame_response(char* clave, int id);
	/*
	 * Descripción: devuelve el valor de respuesta respecto a la clave que se quiere bloquear y quién
	 * 		desea bloquearla.
	 * Argumentos:
	 * 		char* clave: la clave a bloquear.
	 * 		int id: el id del bloqueante.
	 */

void asignar_parametros_a_enviar();
	/*
 	 * Descripción: obtiene los parametros para mandarlos a la instancia.
 	 */

void enviar_orden_instancia(int tamanio_parametros_set, void* un_socket);
void enviar_valores_set(int tamanio_parametros_set, void* un_socket);


#endif /* COORDINADOR_H_ */
