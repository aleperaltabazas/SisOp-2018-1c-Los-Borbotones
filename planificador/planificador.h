/*
 * planificador.h
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include <shared-library.h>

//Variables locales

char* PUERTO_COORDINADOR;
char* PUERTO_PLANIFICADOR;
char* IP_COORDINADOR;
char* IP_PLANIFICADOR;
int ESTIMACION_INICIAL;
float ALFA;
char** CLAVES_BLOQUEADAS;
algoritmo_planificacion ALGORITMO_PLANIFICACION;

ESI esi_vacio = {
		.id = 0,
		.socket = 0,
		.rafaga_real = 0
};

uint32_t ESI_id;
int tiempo;
int ESIs_size;

int socket_coordinador;

aviso_con_ID aviso_bloqueo = {
		.aviso = 25
};

aviso_con_ID aviso_desbloqueo = {
		.aviso = 27
};

ESI executing_ESI;

ESI ESI_error = {
		.id = -2
};

bool seguir_ejecucion = true;
bool ejecutando = false;
bool consola_planificacion = true;
bool display = true;
bool show_debug_commands = false;

t_esi_list ready_ESIs;
t_esi_list blocked_ESIs;
t_esi_list finished_ESIs;

//Semaforos

pthread_mutex_t sem_ESIs_size;
pthread_mutex_t sem_ID;
pthread_mutex_t sem_ESI_ID;
pthread_mutex_t mutex_consola_planificacion;
pthread_mutex_t sem_clock;
pthread_mutex_t sem_planificacion;
pthread_mutex_t sem_ejecucion;
pthread_mutex_t sem_ejecutando;
pthread_mutex_t sem_ready_ESIs;
pthread_mutex_t sem_server_socket;
pthread_mutex_t sem_socket_coordinador;

//Hilos

pthread_t hiloDeConsola;
pthread_t hilo_coordinador;

//Funciones de consola

void* consola(void*);
void listarOpciones(void);
void pausarOContinuar(void);
void bloquear(int codigo);
void desbloquear(int codigo);
void kill_esi(int codigo);
void status(void);
void deadlock(void);
float recibirCodigo(void);
void interpretarYEjecutarCodigo(int comando);
void terminar(void);
void mostrame_clock(void);
void display_console(void);
void dame_datos(void);
void bloquear_clave(void);
void desbloquear_clave(void);
void weed(void);
void finishStatus(void);
void finishListar(void);
void bloquearSegunClave(void);
void listar_bloqueados(void);
void matar(void);
void datos_ESI(void);
void show_debug(void);
void show(void);

//Funciones de servidor

int manejar_cliente(int listening_socket, int socket_cliente, package_int package);
	/*
	 * Descripción: determina qué hacer cuando recibe una nueva conexión a través del
	 * 		socket cliente.
	 * Argumentos:
	 * 		int listening_socket: socket del servidor local.
	 * 		int socket_cliente: socket del cliente.
	 * 		package_int package: identificación para los nuevos clientes.
	 */

void identificar_cliente(package_int id, int socket_cliente);
	/*
	 * Descripción: identifica a un cliente y crea un hilo para el mismo, o llama a salir_con_error().
	 * 		en caso de ser un cliente desconocido.
	 * Argumentos:
	 * 		package_int id: identificación enviada por el cliente.
	 * 		int socket_cliente: socket del cliente.
	 */

void manejar_conexiones(void);
	/*
	 * Descripción: levanta el servidor y recibe las conexiones mientras que se siga la ejecución.
	 * Argumentos:
	 * 		void
	 */

void cerrar(void);
	/*
	 * Descripción: avisa el fin de ejecución al coordinador y cierra el socket.
	 * Argumentos:
	 * 		void
	 */

//Funciones de hilos

void* atender_ESI(void* sockfd);
	/*
	 * Descripción: atiende los mensajes enviados de un ESI y le avisa que ejecute
	 * 		cuando se le indique.
	 * Argumentos:
	 * 		void* sockfd: descriptor de socket que luego se castea a int.
	 */

void* atender_coordinador(void*);
	/*
	 * Descripción: atiende las conversaciones con el coordinador.
	 * Argumentos:
	 * 		void* nada
	 */

//Funciones

void iniciar(char** argv);
	/*
	 * Descripción: crea el logger y las listas de ESIs, y carga los datos del archivo
	 * 		de configuración en variables globales.
	 * Argumentos:
	 * 		char** argv: array que contiene el archivo de configuración.
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
	 * Descripción: desaloja al executing_ESI, poniéndo en su lugar el esi_vacio;
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
	 * 		ESI esi: el ESI a obtener su clock de espera.
	 */

float estimated_time(ESI esi);
	/*
	 * Descripción: devuelve la duración estimada de ráfaga de un ESI.
	 * Argumentos:
	 * 		ESI esi: el ESI a obtener su duración de ráfaga.
	 */

void iniciar_semaforos(void);
	/*
	 * Descripción: inicializa todos los semáforos.
	 * Argumentos:
	 * 		void
	 */

ESI dame_proximo_ESI(void);
	/*
	 * Descripción: devuelve el siguiente ESI de acuerdo al algoritmo.
	 * Argumentos:
	 * 		void
	 */

bool no_hay_ESI(void);
	/*
	 * Descripción: devuelve si el executing_ESI es igual esi_vacio;
	 * Argumentos:
	 * 		void
	 */

int recibir_respuesta(int server_socket);
	/*
	 * Descripción: recibe una respuesta de un servidor, por ok o por error.
	 * Argumentos:
	 * 		int server_socket: socket del servidor.
	 */

void cerrar_ESIs(void);
	/*
	 * Descripción: cierra la conexión con todos los ESIs de las listas.
	 * Argumentos:
	 * 		void
	 */

void cargar_configuracion(char** argv);
	/*
	 * Descripción: carga los datos del archivo de configuracion en variables globales.
	 * Argumentos:
	 * 		char** argv: array que contiene el archivo de configuración.
	 */

void iniciar_hilos(void);
	/*
	 * Descripción: inicia los hilos del coordinador y de la consola.
	 * Argumentos:
	 * 		void
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

algoritmo_planificacion dame_algoritmo(char* algoritmo_src);
	/*
	 * Descripción: devuelve el algoritmo correspondiente respecto a una cadena.
	 * Argumentos:
	 * 		char* algoritmo_src: la cadena a tomar el algoritmo.
	 */

void avisar_bloqueo(int server_socket, char* clave);
	/*
	 * Descripción: avisa a un servidor el bloqueo de una clave determinada.
	 * Argumentos:
	 * 		int server_socket: socket del servidor.
	 * 		char* clave: clave a bloquear.
	 */

void avisar_desbloqueo(int server_socket, char* clave);
	/*
	 * Descripción: avisa a un servidor el desbloqueo de una clave determinada.
	 * Argumentos:
	 * 		int server_socket: socket del servidor.
	 * 		char* clave: clave a desbloquear.
	 */

int recibir_mensaje(int sockfd, int id, ESI esi);
	/*
	 * Descripción: recibe mensajes de un cliente y retorna si se continúa con el hilo o no.
	 * Argumentos:
	 * 		int sockfd: socket del cliente.
	 * 		int id: id del cliente.
	 * 		ESI esi: ESI del cliente.
	 */

bool esta(t_esi_list lista, ESI esi);
	/*
	 * Descripción: devuelve si un ESI se encuentra en una lista determinada.
	 * Argumentos:
	 * 		t_esi_list lista: lista a recorrer.
	 * 		ESI esi: ESI a verificar su existencia.
	 */

void vaciar_ESI(void);
	/*
	 * Descripción: pone el ESI_vacio en el executing_ESI.
	 * Argumentos:
	 * 		void
	 */

float tiempo_real(ESI esi);
	/*
	 * Descripción: devuelve el tiempo real del ESI multiplicado por el Alfa.
	 * Argumentos:
	 * 		ESI esi: el ESI del cual calcular la ráfaga.
	 */

float estimado(ESI esi);
	/*
	 * Descripción: devuelve el tiempo estimado del ESI multplicado por el Alfa.
	 * Argumentos:
	 * 		ESI esi: el ESI del cual calcular la ráfaga.
	 */

void desbloquear_ESI(uint32_t id);
	/*
	 * Descripción: mueve un ESI de la cola de bloqueados a la de listos a partir de su id.
	 * Argumentos:
	 * 		uint32_t id: id del ESI a desbloquear.
	 */

ESI get_ESI(uint32_t id, t_esi_list lista);
	/*
	 * Descripción: devuelve un ESI de una lista determinada a partir de un id.
	 * Argumentos:
	 * 		uint32_t id: el id del ESI a buscar.
	 * 		t_esi_list lista: la lista donde buscar.
	 */

void mostrar(t_esi_node* puntero);
	/*
	 * Descripción: muestra el id de los ESIs de una lista.
	 * Argumentos:
	 * 		t_esi_node* puntero: puntero de la lista.
	 */

void bloqueo_inicial(void);
	/*
	 * Descripción:  bloquea las claves del archivo de configuración.
	 * Argumentos:
	 * 		void
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

ESI findByIDIn(uint32_t id, t_esi_list lista);
	/*
	 * Descripción: busca un con el ID en la lista. Si no lo encuentra, retorna ESI_error.
	 * Argumentos:
	 * 		uint32_t id: ID de ESI a buscar.
	 * 		t_esi_list lista: lista donde buscar.
	 */

void avisarBloqueoESIPorClave(ESI esi, char* clave, int sockfd);
	/*
	 * Descripción: avisa al coordinador para que bloquee a un ESI detrás de la clave.
	 * Argumentos:
	 * 		ESI esi: ESI a bloquear.
	 * 		char* clave: clave por la cual bloquear al ESI.
	 * 		int sockfd: socket del coordinador.
	 */

float response_ratio(ESI esi);
	/*
	 * Descripción: devuelve el RR (response ratio) de un ESI.
	 * Argumentos:
	 * 		ESI esi
	 */

void conseguir_desbloqueado(void);
	/*
	 * Descripción: intercambia mensajes con el coordinador para averiguar el próximo ESI a desbloquear.
	 * Argumentos:
	 * 		void
	 */

#endif /* PLANIFICADOR_H_ */
