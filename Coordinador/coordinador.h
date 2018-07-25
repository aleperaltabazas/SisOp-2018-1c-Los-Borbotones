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

char abecedario[] = "abcdefghijklmnopqrstuvwxyz";

package_int bloqueo_ok = {
		.packed = 26
};

t_clave_list claves_bloqueadas;
t_clave_list claves_disponibles;
t_blocked_list blocked_ESIs;
t_instancia_list instancias;

int pointer;

parametros_set valor_set;
char * buffer_parametros;

t_log* log_operaciones;

Instancia inst_error = {
		.nombre = "ERROR DE INSTANCIA"
};

bool flag_free_asignada;

pthread_mutex_t sem_socket_operaciones_coordi;

pthread_mutex_t sem_instancias;
pthread_mutex_t sem_listening_socket;

uint32_t id_not_found = -3;
uint32_t desbloqueada_ID = -1;
uint32_t proximo_desbloqueado;
t_desbloqueado_list cola_desbloqueados;

int cantidad_instancias;

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

int chequear_solicitud(int socket_cliente, uint32_t id);
	/*
	 * Descripción: revisa una solicitud de ejecución de un proceso ESI y le indica si puede
	 * 		o no ejecutar.
	 * Argumentos:
	 * 		int socket_cliente: socket del proceso ESI.
	 * 		uint32_t id: ID del ESI.
	 */

void iniciar(char** argv);
	/*
	 * Descripción: carga las configuraciones iniciales del proceso.
	 * Argumentos:
	 * 		char** argv: array que contiene el archivo de configuración.
	 */

void iniciar_semaforos(void);
	/*
	 * Descripción: llama a pthread_mutex_init() para todos los semáforos.
	 * Argumentos:
	 * 		void
	 */

void iniciar_listas(void);
	/*
	 * Descripción: inicializa las cabezas de las listas con NULL.
	 * Argumentos:
	 * 		void
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

void enviar_claves(t_clave_list claves, int sockfd, char* name);
	/*
	 * Descripción: envía las claves de la lista a una instancia a través del sockfd.
	 * Argumentos:
	 * 		t_clave_list claves: lista de claves a enviar.
	 * 		int sockfd: socket por el cual comunicar.
	 * 		char* name: nombre de la instancia
	 */

bool estaCaida(Instancia unaInstancia);
	/*
	 * Descripción: pingea a unaInstancia para ver si esta caída.
	 * Argumentos:
	 * 		Instancia unaInstancia
	 */

void ping(Instancia unaInstancia);
	/*
	 * Descripción: pingea a una instancia. En caso que falle el pingeo, marca el flag de disponibilidad
	 * 		de la instancia como false.
	 * Argumentos:
	 * 		Instancia unaInstancia
	 */

uint32_t waitPing(Instancia unaInstancia);
	/*
	 * Descripción: devuelve el resultado de ping enviado por unaInstancia.
	 * Argumentos:
	 * 		Instancia unaInstancia
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

void conectar(Instancia instancia);
	/*
	 * Descripción: marca el flag de disponibilidad de una instancia como true.
	 * Argumentos:
	 * 		Instancia instancia: la instancia a conectar.
	 */

void desconectar(Instancia instancia);
	/*
	 * Descripción: marca el flag de disponibilidad de una instancia como false.
	 * Argumentos:
	 * 		Instancia instancia: la instancia a desconectar.
	 */

uint32_t decimeID(int sockfd);
	/*
	 * Descripción: intercambia mensajes con otro proceso para recibir su número de identificación
	 * 		en el sistema. En caso que falle la recepción, devuelve -1.
	 * Argumentos:
	 * 		int sockfd: socket por el cual comunicar.
	 */

void liberar_claves(uint32_t id);
	/*
	 * Descripción: libera las claves bloqueadas bajo el id asociado.
	 * Argumentos:
	 * 		uint32_t id: id de bloqueante que desbloquea.
	 */

void send_OK(int sockfd);
	/*
	 * Descripción: envía un mensaje de OK a un proceso.
	 * Argumentos:
	 * 		int sockfd: socket por el cual establecer la comunicación.
	 */

void startSigHandlers(void);
	/*
	 * Descripción: invoca signal() con los signal handlers necesarios para el proceso.
	 * Argumentos:
	 * 		void
	 */

void sigHandler_sigint(int signo);
	/*
	 * Descripción: atrapa la señal SIGINT ante un CTRL+C, logea el último error y cierra el listening_socket.
	 * Argumentos:
	 * 		int signo
	 */

void sigHandler_segfault(int signo);
	/*
	 * Descripción: atrapa la señal SIGSEGV y logea el error que lo causó.
	 * Argumentos:
	 * 		int signo
	 */

void redistribuir_claves(void);
	/*
	 * Descripción: en caso de usar el algoritmo Key Explicit, redistribuye las claves que toma cada
	 * 		instancia, en base al total de instancias disponibles.
	 * Argumentos:
	 * 		void
	 */

void asignarKeyMinMax(Instancia* instancia, int posicion, int totalDeDisponibles);
	/*
	 * Descripción: asigna las claves máximas y mínimas correspondientes a la posición respecto al total
	 * 		de instancias disponibles.
	 * Argumentos:
	 * 		Instancia* instancia: a quien asignar los valores.
	 * 		int posicion: número de instancia.
	 * 		int totalDeDisponibles: número total de instancias disponibles en el sistema.
	 */

int redondearDivision(double x, double y);
	/*
	 * Descripción: devuelve el redondeo de x/y.
	 * Argumentos:
	 * 		double x
	 * 		double y
	 */

double getParteFraccional(double x);
	/*
	 * Descripción: devuelve la parte fraccional de un número.
	 * Argumentos:
	 * 		double x
	 */

int getParteEntera(double x);
	/*
	 * Descripción: devuelve la parte entera de un número.
	 * Argumentos:
	 * 		double x
	 */

bool leCorresponde(Instancia instancia, char caracter);
	/*
	 * Descripción: indica si caracter está entre el valor máximo y mínimo de la instancia.
	 * Argumentos:
	 * 		Instancia instancia
	 * 		char caracter
	 */

bool estaAsignada(char* clave);
	/*
	 * Descripción: indica si una clave está asignada a alguna instancia.
	 * Argumentos:
	 * 		char* clave: la clave a buscar.
	 */

Instancia elQueLaTiene(char* clave);
	/*
	 * Descripción: devuelve la instancia que posee la clave.
	 * Argumentos:
	 * 		char* clave
	 */

void status(int sockfd);
	/*
	 * Descripción: le envía el status de una clave al planificador.
	 * Argumentos:
	 * 		int sockfd: socket por el cual comunicar.
	 */

void actualizarClave(char* clave, char* valor);
	/*
	 * Descripción: actualiza la clave con el valor en la estructura de claves bloqueadas. Si no
	 * 		encuentra la clave, no hace nada.
	 * Argumentos:
	 * 		char* clave: la clave a actualizar.
	 * 		char* valor: el valor a actualizar en la clave.
	 */

char* getInstancia(char* recurso);
	/*
	 * Descripción: devuelve la instancia que posee el recurso. Si el recurso no existe, devuelve
	 * 		un string avisando. Si ninguna instancia tiene el recurso, devuelve la instancia que le
	 * 		tocaría tener el recurso.
	 * Argumentos:
	 * 		char* recurso: recurso a buscar.
	 */

void bloquearSegunClave(int sockfd);
	/*
	 * Descripción: intercambia mensajes con el planificador para bloquear a un ESI tras una determinada clave
	 * Argumentos:
	 * 		int sockfd: socket del planificador.
	 */

Instancia menorEspacio(t_instancia_list lista);
	/*
	 * Descripción: devuelve la instancia cuyo atributo "espacio_usado" es el menor de la lista.
	 * Argumentos:
	 * 		t_instancia_list lista
	 */

bool tieneMenosEspacio(Instancia unaInstancia, Instancia otraInstancia);
	/*
	 * Descripción: devuelve si el atributo espacio_usado de unaInstancia es menor que otraInstancia.
	 * Argumentos:
	 * 		Instancia unaInstancia
	 * 		Instancia otraInstancia.
	 */

uint32_t doGet(GET_Op get);
	/*
	 * Descripción: devuelve un valor respecto a un intento de GET.
	 * Argumentos:
	 * 		GET_Op get: estructura que contiene el ID de quien desea hacer la operación
	 * 			y la clave sobre cual hacer dicha operación.
	 */

uint32_t doSet(SET_Op set);
	/*
 	 * Descripción: devuelve un valor respecto a un intento de SET.
 	 * Argumentos:
 	 * 		SET_Op get: estructura que contiene el ID de quien desea hacer la operación,
 	 * 			la clave y valor sobre cual hacer dicha operación.
 	 */

uint32_t doStore(STORE_Op store);
	/*
	 * Descripción: devuelve un valor respecto a un intento de STORE.
	 * Argumentos:
	 * 		STORE_Op get: estructura que contiene el ID de quien desea hacer la operación
	 * 			y la clave sobre cual hacer dicha operación.
	 */

uint32_t getBlockerID(char* clave);
	/*
	 * Descripción: devuelve el ID del ESI que bloqueó la clave.
	 * Argumentos:
	 * 		char* clave
	 */

void revisar_existencia(char* clave);
	/*
	 * Descripción: crea la clave en caso de que no exista. Si ya existe, no hace nada.
	 * Argumentos:
	 * 		char* clave
	 */

uint32_t findBlockerIn(char* clave, t_clave_list lista);
	/*
	 * Descripción: devuelve el del ESI que bloqueó el ID, si se encuentra en la lista. Si no
	 * 		lo encuentra, devuelve id_not_found.
	 * Argumentos:
	 * 		char* clave
	 * 		t_clave_list lista
	 */

void log_get(GET_Op get);
	/*
	 * Descripción: loguea en el log de operaciones una operación GET con los datos de la
	 * 		estructura get.
	 * Argumentos:
	 * 		GET_Op get
	 */

void log_set(SET_Op set);
	/*
	 * Descripción: loguea en el log de operaciones una operación SET con los datos de la
	 * 		estructura set.
	 * Argumentos:
	 * 		SET_Op set
	 */

void log_store(STORE_Op store);
	/*
	 * Descripción: loguea en el log de operaciones una operación STORE con los datos de la
	 * 		estructura store.
	 * Argumentos:
	 * 		STORE_Op store
	 */

void gettearClave(GET_Op get);
	/*
	 * Descripción: bloquea la clave por el ID contenidos en get y loguea en el log de operaciones.
	 * Argumentos
	 * 		GET_Op get
	 */

uint32_t settearClave(SET_Op set, Instancia instancia);
	/*
	 * Descripción: envía a la instancia la clave y su valor para que le haga SET. Si la instancia
	 * 		se encuentra caída, devuelve -1.
	 * Argumentos:
	 * 		Instancia instancia
	 * 		char* clave
	 * 		char* valor
	 */

uint32_t storearClave(STORE_Op store, Instancia instancia);

	/*
	 * Descripción: envía a la instancia la clave de store para guardar. Si la instancia se encuentra caída
	 * 		devuelve -1.
	 * Argumentos:
	 * 		Instancia instancia
	 * 		char* clave
	 */

void enviar_set(SET_Op set, Instancia instancia);
	/*
	 * Descripción: envía la operación de SET a la instancia.
	 * Argumentos:
	 * 		SET_Op set
	 * 		Instancia instancia
	 */

uint32_t recibir_set(Instancia instancia);
	/*
	 * Descripción: devuelve la respuesta enviada por la instancia respecto al set.
	 * 		Además, si no hay error, actualiza las entradas ocupadas por la instancia.
	 * Argumentos:
	 * 		Instancia instancia
	 */

void enviar_store(STORE_Op store, Instancia instancia);
	/*
	 * Descripción: envía la operacion de STORE a la instancia.
	 * Argumentos:
	 * 		STORE_Op store
	 * 		Instancia instancia
	 */

uint32_t recibir_store(Instancia instancia);
	/*
	 * Descripción: devuelve la respuesta enviada por la instancia respecto al store.
	 * Argumentos:
	 * 		Instancia instancia
	 */

void avisarDesbloqueo(char* clave);
	/*
	 * Descripción: avisa al planificador que ESI desbloquear respecto a una clave.
	 * Argumentos:
	 * 		char* clave
	 */

void enviar_desbloqueado(int sockfd);
	/*
	 * Descripción: envía el ID de la variable proximo_desbloqueado a través del sockfd.
	 * Argumentos:
	 * 		int sockfd
	 */

uint32_t getDesbloqueado(char* clave);
	/*
	 * Descripción: devuelve el ID del primer ESI bloqueado por la clave. Si no se encuentra, devuelve -1.
	 * Argumentos:
	 * 		char* clave
	 */

void pingAll(void);
	/*
	 * Descripción: envía mensaje de ping a todas las instancias para verificar cuáles siguen disponibles
	 * 		para la distribución de claves en base a las nuevas instancias.
	 * Argumentos:
	 * 		void
	 */

Instancia getInstanciaByName(t_instancia_list lista, char* name);
	/*
	 * Descripción: devuelve la estructura Instancia de la lista asociada a name. Si no se encuentra un
	 * 		elemento en la lista cuyo nombre sea name, devuelve inst_error
	 * Argumentos:
	 * 		t_instancia_list lista
	 * 		char* name
	 */

void actualizarEntradas(Instancia instancia, uint32_t cantidad);
	/*
	 * Descripción: actualiza las entradas ocupadas por una instancia en la lista de instancias.
	 * Argumentos:
	 * 		Instancia instancia
	 * 		uint32_t cantidad
	 */

t_blocked_list listaAuxiliar;

char* getValor(char* recurso);

int do_set(char* valor, char* clave);

Instancia getInstanciaSet(char* clave);

void enviar_orden_instancia(int tamanio_parametros_set, void* un_socket, int codigo_de_operacion);
void enviar_valores_set(int tamanio_parametros_set, void* un_socket);
void enviar_ordenes_de_prueba(void* un_socket);
void enviar_ordenes_de_prueba_compactacion(void* un_socket);
void asignar_entradas(int sockfd);
int obtener_tamanio_parametros_set(SET_Op set);
void asignar_parametros_set(SET_Op set);

int instancia_id;
Instancia equitativeLoad(void); //Devuelve el socket que corresponde
Instancia leastSpaceUsed(void);
int desempatar (t_instancia_node* a, int b);
Instancia keyExplicit (char * clave);
int esperar_confirmacion_de_exito(int un_socket);
void enviar_instancias_a_compactar();

int instanciasDisponibles(void);
void mostrar_listas();

t_blocked_list tienenAlgoRetenido (t_blocked_list lista); //Filtra de los esis bloqueados los que si tienen recursos asignados
bool tieneAlgoRetenido(uint32_t id);
char * pasarACadena(t_blocked_list lista);
t_blocked_list estanEnDL (t_blocked_list lista);
bool puedeLlegarA (t_blocked_node * puntero);
t_clave_node * duenioDe (char * claveBuscada);
void liberar(t_blocked_list * lista);
bool estaEn(t_blocked_list lista, uint32_t id);
void agregar(t_blocked_list lista, t_blocked_node nodo);
void comunicarDeadlock(int socket);

char* getValor(char* clave);
char* getInstancia(char* clave);
char* getBloqueados(char* clave);

#endif /* COORDINADOR_H_ */
