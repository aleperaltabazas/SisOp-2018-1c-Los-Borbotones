/*
 * instancias.h
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#ifndef INSTANCIAS_H_
#define INSTANCIAS_H_
#define CANTIDAD_ENTRADAS 10
#define TAMANIO_ENTRADA 8

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

//El valor que se desea almacenar en la instancia
char * valor;

//Para saber si una entrada esta ocupada o no uso un vector (0 => libre)
int entradas_disponibles[CANTIDAD_ENTRADAS];

//Para conocer el tamaño del valor almacenado en la entrada
int tamanios_de_valor_de_entradas_ocupadas[CANTIDAD_ENTRADAS];

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
int almacenar_valor();
char * leer_valor(int posicion);
void leer_valores_almacenados();
int verificar_disponibilidad_entradas_contiguas(int entradas_que_ocupa, int entrada);
void actualizar_entradas(int pos_entrada, int entradas_que_ocupa);
void set(uint32_t longitud_parametros, int socket_coordinador);
void cargar_configuracion(void);
void iniciar(void);
algoritmo_reemplazo dame_algoritmo(char* algoritmo_src);
int recieve_and_deserialize(parametros_set *parametros, int socketCliente);

//Aca guardo todos los structs entrada para poder referenciarlos
t_list * entradas;

//Estructura de las entradas

//Funciones para el coordinador
orden_del_coordinador recibir_orden_coordinador(int socket_coordinador);

//Casos de prueba
void caso_de_prueba_1();
void caso_de_prueba_2();
void caso_de_prueba_3();
void caso_de_prueba_4();
void caso_de_prueba_5();

#endif /* INSTANCIAS_H_ */
