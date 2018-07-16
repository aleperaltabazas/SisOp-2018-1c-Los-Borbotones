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

t_instancia_list instancias;
t_instancia_node* pointer;

t_blocked_list blocked_ESIs;

parametros_set valor_set;
char * buffer_parametros;

t_log* log_operaciones;

Instancia inst_error = {
		.nombre = "ERROR DE INSTANCIA"
};

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

int hacer_store(char* clave);
	/*
	 * Descripción: realiza el guardado de la clave en la instancia. Si la instancia que posee la clave
	 * 		no se encuentra conectada, o falla el envío de la clave, se devuelve -1.
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

void liberar_ESI(t_blocked_list* lista, uint32_t id);
	/*
	 * Descripción: quita un elemento de la lista a partir de su id. Si el id es -5, no hace nada.
	 * Argumentos:
	 * 		t_blocked_list* lista: lista a modificar.
	 * 		uint32_t id: elemento a quitar de la lista.
	 */

bool emptyBlocked(t_blocked_list* lista);
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

char* get_name(int sockfd);
	/*
	 * Descripción: retorna el nombre enviado por otro proceso a través del sockfd.
	 * Argumentos:
	 * 		sockfd: el socket por el cual realizar la comunicación.
	 */

void levantar_instancia(char* name, int sockfd);
	/*
	 * Descripción: en base a si una instancia con el nombre se encontraba previamente
	 * 		en el sistema pero se cayó, o bien crea una nueva instancia, o recupera la que
	 * 		se encontraba en el sistema.
	 * Argumentos:
	 * 		char* name: el nombre de la instancia.
	 * 		int sockfd: el socket actual de dicha instancia.
	 */

bool murio(char* name, int sockfd);
	/*
	 * Descripción: revisa si una instancia se encontraba en el sistema, pero se desconectó.
	 * 		Si una instancia con ese nombre ya se encuentra en el sistema, aborta la conexión.
	 * Argumentos:
	 * 		char* name: nombre de la instancia a verificar.
	 * 		int sockfd: socket de la instancia.
	 */

void revivir(char* name, int sockfd);
	/*
	 * Descripción: toma los parámetros que ya se encontraban en el sistema y los agrega a la
	 * 		nueva instancia, y elimina la entrada vieja.
	 * Argumentos:
	 * 		char* name: nombre de la instancia.
	 * 		int sockfd: socket de la instancia.
	 */

void send_orden_no_exit(int op_code, int sockfd);
	/*
	 * Descripción: llama a enviar_orden_instancia() con el primer argumento en 0.
	 * Argumentos:
	 * 		int op_code: código de operación a ejecutar por la instancia.
	 * 		int sockfd: socket por el cual comunicar
	 */

void update(char* name, int sockfd);
	/*
	 * Descripción: actualiza el socket de una instancia en la lista y la marca como disponible.
	 * Argumentos:
	 * 		char* name: nombre de la instancia a actualizar.
	 * 		int sockfd: socket a actualizar.
	 */

t_clave_list get_claves(char* name);
	/*
	 * Descripción: devuelve la lista de las claves de la instancia con el nombre name. En caso que no
	 * 		encuentre dicha instancia, devuelve NULL.
	 * Argumentos:
	 * 		char* name: nombre de la instancia a buscar sus claves.
	 */

void enviar_claves(t_clave_list claves, int sockfd);
	/*
	 * Descripción: envía las claves de la lista a una instancia a través del sockfd.
	 * Argumentos:
	 * 		t_clave_list claves: lista de claves a enviar.
	 * 		int sockfd: socket por el cual comunicar.
	 */

bool ping(Instancia instancia);
	/*
	 * Descripción: pingea a una instancia. En caso que falle el pingeo, marca el flag de disponibilidad
	 * 		de la instancia como false.
	 * Argumentos:
	 * 		Instancia instancia: la instancia a pingear.
	 */

bool recv_ping(int sockfd);
	/*
	 * Descripción: recibe la confirmación del ping realizado a otro proceso.
	 * Argumentos:
	 * 		int sockfd: socket para comunicar.
	 */

Instancia getInstanciaStore(char* clave);
	/*
	 * Descripción: devuelve la instancia que tiene la clave. Si no se encuentra ninguna instancia que
	 * 		tenga la clave, devuelve inst_error
	 * Argumentos:
	 * 		char* clave: la clave a hacer STORE.
	 */

bool tieneLaClave(Instancia instancia, char* clave);
	/*
	 * Descripción: indica si una instancia tiene dicha clave en su lista de claves.
	 * Argumentos:
	 * 		Instancia instancia: la instancia a verificar.
	 * 		char* clave: la clave a buscar.
	 */

void avanzar_puntero(void);
	/*
	 * Descripción: avanza la variable global pointer al siguiente nodo. Si el siguiente fuera NULL,
	 * 		vuelve a la cabeza de la lista.
	 * Argumentos:
	 * 		void
	 */

void actualizarPuntero(void);
	/*
	 * Descripción: reacomoda el puntero al lugar en el que estaba cuando se agrega una instancia nueva.
	 * Argumentos:
	 * 		void
	 */

void actualizarInstancia(Instancia instancia, char* clave);
	/*
	 * Descripción: agrega la clave a la lista de claves de la instancia.
	 * Argumentos:
	 * 		Instancia instancia: la instancia a actualizar.
	 * 		char* clave: la clave a agregar a la lista.
	 */

void desconectar(Instancia instancia);
	/*
	 * Descripción: marca el flag de disponibilidad de una instancia como false.
	 * Argumentos:
	 * 		Instancia instancia: la instancia a desconectar.
	 */

int do_set(char* valor, char* clave);

Instancia getInstanciaSet(char* clave);

void enviar_orden_instancia(int tamanio_parametros_set, void* un_socket, int codigo_de_operacion);
void enviar_valores_set(int tamanio_parametros_set, void* un_socket);
void enviar_ordenes_de_prueba(void* un_socket);
void enviar_ordenes_de_prueba_compactacion(void* un_socket);
void asignar_entradas(int sockfd);

int instancia_id;
Instancia equitativeLoad(void); //Devuelve el socket que corresponde
Instancia leastSpaceUsed(void);
int desempatar (t_instancia_node* a, int b);
Instancia keyExplicit (char * clave);

int instanciasDisponibles(void);

#endif /* COORDINADOR_H_ */
