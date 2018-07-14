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
char* PUERTO_PLANIFICADOR;
char* IP_PLANIFICADOR;

int seguir_ejecucion = 1;

int socket_planificador;

package_int bloqueo_ok = {
		.packed = 26
};

t_clave_list claves_bloqueadas;
t_clave_list claves_disponibles;

t_blocked_list blocked_ESIs;

parametros_set valor_set;
char * buffer_parametros;

t_log* log_operaciones;

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

void *atender_instancia(void* un_socket);
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

void iniciar(char** argv);
	/*
	 * Descripción: carga las configuraciones iniciales del proceso.
	 * Argumentos:
	 * 		char** argv: array que contiene el archivo de configuración.
	 */

void cargar_configuracion(char** argv);
	/*
	 * Descripción: abre el archivo de configuración y carga los valores en variables globales.
	 * Argumentos:
	 * 		char** argv: array que contiene el archivo de configuración.
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

void bloquear(char* clave, uint32_t id);
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

void agregar_clave(t_clave_list* lista, char* clave, uint32_t id);
	/*
	 * Descripción: agrega una clave a la lista.
	 * Argumentos:
	 * 		t_clave_list* lista: lista a la cual agregar la clave.
	 * 		char* clave: la clave a agregar.
	 */

t_clave_node* crear_nodo(char* clave, uint32_t id);
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

int get(int sockfd, uint32_t id);
	/*
	 * Descripción: bloquea una clave y se la asigna al id. Si no existe, se crea y se bloquea. Si
	 * 		ya existe y está bloqueada, se bloquea al proceso cliente. En caso que la operación
	 * 		sea inválida, devuelve -5 para abortar al ESI.
	 * Argumentos:
	 * 		int sockfd: socket del proceso cliente.
	 * 		uint32_t id: identificador del proceso cliente.
	 */

int set(int sockfd, uint32_t id);
	/*
	 * Descripción: settea un valor en una clave. Si la clave no existe, se bloquea al proceso cliente.
	 * 		Si existe, pero las identificaciones del bloqueante y del proceso cliente no matchean, se
	 * 		bloquea al proceso cliente. En caso que la operación
	 * 		sea inválida, devuelve -5 para abortar al ESI.
	 * Argumentos:
	 * 		int sockfd: socket del proceso cliente.
	 * 		uint32_t id: identificador del proceso cliente.
	 */

int store(int sockfd, uint32_t id);
	/*
	 * Descripción: guarda el valor de una clave en una instancia. Si la clave no existe, se bloquea al
	 * 		proceso cliente. Si existe, pero las identificaciones del bloqueante y del proceso cliente
	 * 		no matchean, se	bloquea al proceso cliente. En caso que la operación
	 * 		sea inválida, devuelve -5 para abortar al ESI.
	 * Argumentos:
	 * 		int sockfd: socket del proceso cliente.
	 * 		uint32_t id: identificador del proceso cliente.
	 */

int get_packed(char* clave, uint32_t id);
	/*
	 * Descripción: retorna el valor de la operación dependiendo de la existencia de la clave o el resultado
	 * 		de la verificación de las identificaciones.
	 * Argumentos:
	 * 		char* clave: la cadena a verificar.
	 * 		uint32_t id: la identificación a comparar.
	 */

uint32_t get_clave_id(char* clave);
	/*
	 * Descripción: retorna el id de quien bloqueó la clave. En caso de que la clave no exista, retorna -1.
	 * Argumentos:
	 * 		char* clave: la clave a verificar.
	 */

int settear(char* valor, char* clave, uint32_t id);
	/*
	 * Descripción: realiza las verificaciones sobre la clave y el id. En caso negativo, no hace nada y devuelve
	 * 		5. En caso positivo, asigna el valor a la clave y devuelve 20.
	 * Argumentos:
	 * 		char* valor: valor de la clave.
	 * 		char* clave: la clave en cuestión.
	 * 		uint32_t id: id de quien pide el set.
	 */

void hacer_store(char* clave);
	/*
	 * Descripción: realiza el guardado de la clave en la instancia.
	 * Argumentos:
	 * 		char* clave: la clave a guardar.
	 */

int dame_response(char* clave, uint32_t id);
	/*
	 * Descripción: devuelve el valor de respuesta respecto a la clave que se quiere bloquear y quién
	 * 		desea bloquearla.
	 * Argumentos:
	 * 		char* clave: la clave a bloquear.
	 * 		uint32_t id: el id del bloqueante.
	 */

uint32_t dame_desbloqueado(char* clave, t_blocked_list lista);
	/*
	 * Descripción: devuelve el id del ESI a desbloquear respecto a una clave en una lista,
	 * Argumentos:
	 * 		char* clave: la clave a evaluar.
	 * 		t_blocked_list lista: la lista de la cual buscar el id.
	 *
	 */

void bloquear_ESI(char* clave, uint32_t id);
	/*
	 * Descripción: agrega un id asociado a una clave a la lista de bloqueados.
	 * Argumentos:
	 * 		char* clave: la clave que causó el bloqueo.
	 * 		uint32_t id: el id del bloqueado.
	 */

void agregar_blocked(t_blocked_list* lista, blocked bloqueado);
	/*
	 * Descripción: agrega un blocked a una lista de blockeds.
	 * Argumentos:
	 * 		t_blocked_list* lista: la lista a la cual agregar el bloqueado.
	 * 		blocked bloqueado: el bloqueado.
	 */

t_blocked_node* crear_blocked_node(blocked bloqueado);
	/*
	 * Descripción: crea y retorna un nodo de blocked.
	 * Argumentos:
	 * 		blocked bloqueado: estructura que contiene los datos a volcar en el nodo.
	 */

void liberar_ESI(t_blocked_list* lista, uint32_t id);
	/*
	 * Descripción: quita un elemento de la lista a partir de su id. Si el id es -5, no hace nada.
	 * Argumentos:
	 * 		t_blocked_list* lista: lista a modificar.
	 * 		uint32_t id: elemento a quitar de la lista.
	 */

void destruir_blocked_node(t_blocked_node* nodo);
	/*
	 * Descripción: libera la porción de memoria de un nodo.
	 * Argumentos:
	 * 		t_blocked_node* nodo: la memoria a liberar.
	 */

uint32_t head_id(t_blocked_list lista);
	/*
	 * Descripción: devuelve el id del primer elemento de una lista de bloqueados.
	 * Argumentos:
	 * 		t_blocked_list lista: la lista en cuestión.
	 */

void eliminar_blocked(t_blocked_list* lista, uint32_t id);
	/*
	 * Descripción: elimina un nodo de la lista a partir de un id.
	 * Argumentos:
	 * 		t_blocked_list* lista: la lista a modificar.
	 * 		uint32_t id: el id del nodo a eliminar.
	 */

bool esta_vacia(t_blocked_list* lista);
	/*
	 * Descripción: devuelve si una lista está vacía, o sea que su head apunte a NULL.
	 * Argumentos:
	 * 		t_blocked_list* lista: puntero a la lista.
	 */

void asignar_parametros_a_enviar_de_prueba();
	/*
 	 * Descripción: obtiene los parametros para mandarlos a la instancia.
 	 */

void log_op(operacion op);
	/*
	 * Descripción: loguea una operación realizada por un ESI en el log de operaciones, asentando
	 * 		el id de quien realizó la operación, y la clave y, si hubiera, su valor.
	 * Argumentos:
	 * 		operacion op: operacion a guardar.
	 */

void abortar_ESI(int sockfd);
	/*
	 * Descripción: envía un mensaje de fin a un ESI y cierra sockfd. Se llama a
	 * 		terminar_conexion() con el flag retry en false.
	 * Argumentos:
	 * 		sockfd: el socket por el cual enviar el aviso y luego cerrar.
	 */

void do_set(char* valor, char* clave);

int dame_instancia(char* clave);

void enviar_orden_instancia(int tamanio_parametros_set, void* un_socket, int codigo_de_operacion);
void enviar_valores_set(int tamanio_parametros_set, void* un_socket);
void enviar_ordenes_de_prueba(void* un_socket);
void enviar_ordenes_de_prueba_compactacion(void* un_socket);
void asignar_entradas(int sockfd);


typedef struct t_instancia_node{
	int socket;
	int vecesLlamado;
	int espacio_usado;
	int disponible;
	int id;
	struct t_instancia_node* sgte;
} t_instancia_node;

typedef struct t_instancia_list {
	t_instancia_node* head;
} t_instancia_list;

t_instancia_list instancias;

int instancia_id;

void agregameInstancia(int unSocket);
void add (t_instancia_list unaLista, int unSocket);
void * find ();
int instanciasDisponibles(void);
int equitativeLoad(void); //Devuelve el socket que corresponde
int leastSpaceUsed(void);
int desempatar (t_instancia_node* a, int b);
int keyExplicit (char * clave);

t_instancia_node* crear_instancia_node(int sockfd);
void destruir_instancia_node(t_instancia_node* nodo);
void agregar_instancia(t_instancia_list* lista, int sockfd);
t_instancia_node* instancia_head(t_instancia_list lista);
void eliminar_instancia(t_instancia_list* lista, int id);
int instanciasDisponibles(void);



#endif /* COORDINADOR_H_ */
