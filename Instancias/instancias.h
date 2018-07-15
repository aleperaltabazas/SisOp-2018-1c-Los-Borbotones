/*
 * instancias.h
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#ifndef INSTANCIAS_H_
#define INSTANCIAS_H_

#include <shared-library.h>

char* IP_COORDINADOR;
char* PUERTO_COORDINADOR;
algoritmo_reemplazo ALGORITMO_REEMPLAZO;
char* PUNTO_MONTAJE;
char* NOMBRE;
int DUMP;

//Estos valores vienen definidos por el coordinador
int cantidad_entradas;
int tamanio_entrada;

//Para llevar registro de tiempo de las entradas para el LRU
int reloj_interno;

//El valor que se desea almacenar en la instancia
char * valor;

//Para saber si una entrada esta ocupada o no uso un vector (0 => libre)
char * entradas_disponibles;

//Esta es la posicion inicial de memoria en la que comienzo a guardar los valores (mi base)
char * almacenamiento_de_valores;

//Para leer los valores
char * posicion_de_lectura;
char * auxiliar;

//Para considerar si se cae o no
int disponibilidad_de_conexion;

//Para ver si pude asignar o no
int resultado_almacenamiento;

//Funciones auxiliares
void inicializar(int cantidad_entradas, int tamanio_entrada);
/*
 *Se generan las estructuras administrativas y se reserva memoria en base a los parametros recibidos
 */
void almacenar_valor(int entrada_seleccionada, int entradas_que_ocupa, char * valor);
/*
 * Almacena un valor asociado a una clave en la posicion de memoria correspondiente a la entrada
 */
char * leer_valor(entradas_node * unaEntrada);
/*
 * Se llama a un buffer auxiliar para ver lo almacenado en una entrada particular
 * Siempre debe liberarse el puntero auxiliar al llamarse
 */
void leer_valores_almacenados();
/*
 * Para ver que las pruebas funcionaron (ACTUALIZAR)
 */
int verificar_disponibilidad_entradas_contiguas(int entradas_que_ocupa);
/*
 * En base al vector de entradas disponibles me fijo si tengo la cantidad de 0s (espacios libres)
 * contiguos para almacenar el valor recibido
 */
void actualizar_entradas(int pos_entrada, int entradas_que_ocupa);
/*
 * Actualizo el vector de entradas disponibles al almacenar un valor particular
 */
void set(uint32_t longitud_parametros, int socket_coordinador);
/*
 *Actualiza o crea una entrada con una clave asociada y un valor que se reciben del coordinador
 */
int obtener_entradas_que_ocupa(int tamanio_valor);
/*
 * En base al tamanio de un valor (de caracteres) se calcula cuantas entradas ocupa
 */
void crear_entrada(parametros_set parametros, int entrada_seleccionada, int tamanio_valor, int tamanio_clave);
/*
 * Se genera una entrada (estructura) en base a los parametros recibidos
 */
void cargar_configuracion(char** argv);
/*
 * Se lee lo que esta en el archivo de configuracion para setear las variables correspondientes
 */
void iniciar(char** argv);
/*
 * Se cargan los archivos de configuracion y se inicializan las estructuras administrativas
 */
algoritmo_reemplazo dame_algoritmo(char* algoritmo_src);
/*
 * Se selecciona el algoritmo dentro de las 3 opciones posibles
 */

void setup_montaje(void);
	/*
	 * Descripción: crea el punto de montaje.
	 * Argumentos:
	 * 		void
	 */

FILE* open_file(char* name);
	/*
	 * Descripción: devuelve un file descriptor del archivo creado con name en el punto de montaje.
	 * Argumentos:
	 * 		char* name: nombre del archivo.
	 */

void write_file(char* file_name, char* text);
	/*
	 * Descripción: llama a open_file() para abrir un archivo con file_name y le escribe text.
	 * Argumentos:
	 * 		char* file_name: nombre del archivo.
	 * 		char* valor: valor a escribir en el archivo.
	 */

void send_name(int sockfd);
	/*
	 * Descripción: envía el contenido de la variable NOMBRE al proceso conectado por el sockfd.
	 * 		Llama a enviar_packed() y enviar_cadena(), y cierra el proceso si falla el intercambio
	 * 		de mensajes.
	 * Argumentos:
	 * 		int sockfd: file descriptor del socket por el cual comunicar.
	 */

void revivir(int sockfd);
	/*
	 * Descripción: intercambia los mensajes necesarios con el coordinador para recuperar las claves
	 * 		que tenía en caso que esta instancia se haya caido.
	 * Argumentos:
	 * 		int sockfd: socket por el cual realizar la comunicación.
	 */

void store(uint32_t longitud_parametros, int socket_coordinador);

int recieve_and_deserialize_set(parametros_set *parametros, int socketCliente);
/*
 * Se deserializa dinamicamente los parametros enviados por el coordinador para el caso SET
 */
void desplazar(entradas_node * puntero_entrada, int nueva_posicion);
/*
 * Se actualizan las estructuras administrativas luego de una compactacion
 */
void desplazar_memoria(int posicion_a_desplazarse, int posicion_actual, int entradas_del_valor);
/*
 * Se actualiza el espacio de memoria reservado para almacenar (almacenamiento_de_valores) al compactar
 */
int obtener_primera_entrada_disponible();
/*
 * Se obtiene la primer entrada libre que se utilizara como referencia para compactar
 */

entradas_node * buscar_entrada_en_posicion(int posicion);
int obtener_cantidad_de_entradas_ocupadas();
void agregar_entrada(entrada una_entrada);
void actualizar_entradas_disponibles(int entradas_ocupadas);
int posicion_de_entrada_con_clave(char * clave, int tamanio_clave);
void generar_entrada(parametros_set parametros);
void actualizar_entrada(parametros_set parametros, int posicion_entrada_clave);
void actualizar_valor(int entrada_seleccionada, int entradas_que_ocupa, char * valor);
void destruir_nodo_entrada(entradas_node* nodo);
void compactacion();
char * leer_clave_valor(entradas_node * puntero);

void eliminar_entrada_segun_algoritmo();
entrada obtener_entrada_segun_CIRC();
entrada obtener_entrada_segun_LRU();
entrada obtener_entrada_segun_BSU();
entrada first();
void borrar_entrada(entrada entrada_a_eliminar);
void liberar_entradas_disponibles(int posicion, int tamanio_valor);
int comparar_claves(char * clave, int tamanio_clave, char * clave_a_comparar);
void avanzar_puntero_CIRC();

void recibir_orden_inicial(int socket_coordinador);

//Nodo para recorrer lista
entradas_node * nodo_auxiliar = NULL;

//Aca guardo todos los structs entrada para poder referenciarlos
t_entrada_list entradas_asignadas;

//Planteo el puntero entrada como un simple int que coincide con la posicion de la entrada
int puntero_entrada;

//Funciones para el coordinador
orden_del_coordinador recibir_orden_coordinador(int socket_coordinador);
/*
 * Se recibe la orden del coordinador para saber la funcion a emplear (SET, STORE) y como leer los parametros recibidos
 */

//Casos de prueba
void caso_de_prueba_1();
void caso_de_prueba_2();
void caso_de_prueba_3();
void caso_de_prueba_4();
void caso_de_prueba_5();

#endif /* INSTANCIAS_H_ */
