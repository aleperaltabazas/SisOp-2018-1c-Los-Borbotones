/*
 * instancias.c
 *
 *  Created on: 21 abr. 2018
 *      Author: utnso
 */

#include "instancias.h"

int main(int argc, char** argv) {

	cantidad_entradas = 10;

	tamanio_entrada = 8;

	iniciar_log("Instancias", "A new Instance joins the brawl!");

	inicializar(cantidad_entradas, tamanio_entrada);

	log_trace(logger, "Posicion de memoria inicial en main: %d \n", *almacenamiento_de_valores);

	int socket_coordinador = conectar_a(IP_COORDINADOR, PUERTO_COORDINADOR, mensajeInstancia);

	//Para desconectarla habria que cambiar este valor simplemente
	disponibilidad_de_conexion = 1;

	while(disponibilidad_de_conexion){
			orden_del_coordinador orden;
			orden = recibir_orden_coordinador(socket_coordinador);
			switch(orden.codigo_operacion){
			case 11: loggear("SET"); set(orden.tamanio_a_enviar, socket_coordinador); break;
			//case 12: loggear("STORE"); store(orden.tamanio_a_enviar, socket_coordinador); break;
			case 13: loggear("Fallo"); break;
			default: loggear("ERROR"); break;
		}
	}

	//caso_de_prueba_5();

	leer_valores_almacenados();

	loggear("Cerrando conexion con servidor y terminando.");

	close(socket_coordinador);

	free(almacenamiento_de_valores);

	return EXIT_SUCCESS;
}

void inicializar(int cantidad_entradas, int tamanio_entrada){

	//Creo una matriz de memoria para almacenar los valores que vengan
	//Aunque en realidad es una fila pero lo pienso como matriz
	almacenamiento_de_valores = malloc(tamanio_entrada * cantidad_entradas);

	//Inicializo todas las posiciones en 0 (osea que estan libres) y el tama�o de todos los valores en 0
	int i;
	for(i = 0; i < cantidad_entradas; i++){
			entradas_disponibles[i] = 0;
			tamanios_de_valor_de_entradas_ocupadas[i] = 0;
	}

	list_create(entradas);

}

orden_del_coordinador recibir_orden_coordinador(int socket_coordinador){

	orden_del_coordinador orden;
	orden_del_coordinador * buffer_orden = malloc(sizeof(orden_del_coordinador));

	loggear("Esperando orden del coordinador...");

	if(recv(socket_coordinador, buffer_orden, sizeof(orden_del_coordinador), 0) < 0){
		loggear("Fallo en la recepcion de la orden");
		orden.codigo_operacion = 13;
		orden.tamanio_a_enviar = 0;
		return orden;
	}

	loggear("Orden recibida!");

	//memcpy(&(orden.codigo_operacion), buffer_orden -> codigo_operacion, sizeof(uint32_t));
	//memcpy(&(orden.tamanio_a_enviar), buffer_orden -> tamanio_a_enviar, sizeof(uint32_t));

	log_trace(logger, "cod. op: %d", buffer_orden -> codigo_operacion);

	log_trace(logger, "tamanio: %d", buffer_orden -> tamanio_a_enviar);

	sleep(10);

	orden.codigo_operacion = buffer_orden -> codigo_operacion;

	orden.tamanio_a_enviar = buffer_orden -> tamanio_a_enviar;

	return orden;
}

void set(uint32_t longitud_parametros, int socket_coordinador){

	parametros_set valores;
	parametros_set * buffer_valores = malloc(longitud_parametros);

	loggear("Esperando valores");

	recv(socket_coordinador, (void*) buffer_valores, longitud_parametros, 0);

	loggear("Valor recibido, deserializando...");

	int offset = 0;

	memcpy(&valores.tamanio_clave, buffer_valores, sizeof(uint32_t));

	loggear("Tamanio clave deserializado");

	offset += sizeof(uint32_t);

	memcpy(&valores.clave, buffer_valores + offset, valores.tamanio_clave);

	loggear("Clave deserializada");

	offset += valores.tamanio_clave;

	memcpy(&valores.tamanio_valor, buffer_valores + offset, sizeof(uint32_t));

	loggear("Tamanio valor deserializado");

	offset += sizeof(uint32_t);

	log_trace(logger, "Tamanio clave: %d tamanio_valor: %d", valores.tamanio_clave, valores.tamanio_valor);

	memcpy(&valores.valor, buffer_valores + offset, valores.tamanio_valor);

	loggear("Valores deserializados!");

	log_trace(logger, "Clave: %s Valor: %s", valores.clave, valores.valor);

	//Veo si ya existe la clave (en cuyo caso trabajo directamente sobre el struct entrada que contenga esa clave)
	//Si no existe tengo que crearla, por lo que me fijo si puedo almacenar la clave (veo si entra)
	//Si puedo almacenar creo un struct entrada con los valores que me dieron y dandole una posicion por la cual acceder

}

int almacenar_valor(){
	int pos_entrada = 0;
	int offset = 0;
	loggear("Se recibio el valor: ");
	loggear(valor);
	log_trace(logger, "Cantidad de caracteres del valor: %d", strlen(valor));

	int entradas_que_ocupa = (strlen(valor) - 1) / tamanio_entrada + 1;
	log_trace(logger, "El valor entra en %d entradas", entradas_que_ocupa);

	int puedo_almacenar = 0;

	while(pos_entrada < cantidad_entradas){

		 puedo_almacenar = verificar_disponibilidad_entradas_contiguas(entradas_que_ocupa, pos_entrada);

		if(puedo_almacenar){
			log_trace(logger, "El valor entra en la entrada: %d", pos_entrada);
			offset = tamanio_entrada * pos_entrada;

			loggear("Posicion de memoria seleccionada");
			log_trace(logger, "Ubicacion memoria: %d", almacenamiento_de_valores + offset);

			loggear("Copio el valor dentro de esa posicion de memoria");
			memcpy(almacenamiento_de_valores + offset, valor, tamanio_entrada * entradas_que_ocupa);

			loggear("Valor copiado");

			//Actualizo las entradas
			actualizar_entradas(pos_entrada, entradas_que_ocupa);

			return EXIT_SUCCESS;
		}
		//Si no puedo almacenar me fijo en la siguiente

		pos_entrada++;
	}

	loggear("No puedo almacenar el valor");

	return EXIT_FAILURE;
}

int verificar_disponibilidad_entradas_contiguas(int entradas_que_ocupa, int entrada){
	int referencia = entrada;
	while(entrada < referencia + entradas_que_ocupa){
		if(entradas_disponibles[entrada] != 0){
			return 0;
		}
		if(entrada >= cantidad_entradas){
			return 0;
		}
		entrada++;
	}
	log_trace(logger, "Tengo suficientes entradas para almacenar en la entrada: %d", entrada);
	return 1;
}

void actualizar_entradas(int pos_entrada, int entradas_que_ocupa){

	entradas_disponibles[pos_entrada] = 1;

	tamanios_de_valor_de_entradas_ocupadas[pos_entrada] = strlen(valor);

	//Para marcar a las que fueron ocupadas por el valor con mas de una entrada
	int referencia = pos_entrada;
	int siguiente = pos_entrada + 1;
	int limite = referencia + entradas_que_ocupa;

	while(siguiente < limite){
		entradas_disponibles[siguiente] = 2;
		siguiente ++;
	}
}

char * leer_valor(int posicion){
	int tamanio_del_valor_a_leer = tamanios_de_valor_de_entradas_ocupadas[posicion];

	auxiliar = malloc(tamanio_del_valor_a_leer);

	if(entradas_disponibles[posicion] == 0){
		return "NOHAYNADA";
	}

	if(entradas_disponibles[posicion] == 2){
		return "Ocupada";
	}

	log_trace(logger, "Estoy por mostrar la entrada: %d que ocupa: %d", posicion, tamanio_del_valor_a_leer);

	memcpy(auxiliar, almacenamiento_de_valores + posicion * tamanio_entrada, tamanio_del_valor_a_leer);

	//Sobre el final de lo que copie pongo un \0 para que sepa que se termina ahi
	auxiliar[tamanio_del_valor_a_leer] = '\0';

	return auxiliar;
}

void leer_valores_almacenados(){

	loggear("Se almacenaron los siguientes valores: ");
	int k;
	for(k = 0; k < cantidad_entradas; k++){
		log_trace(logger, "En la entrada %d se encuentra almacenado el valor: %s", k, leer_valor(k));
		free(auxiliar);
	}
}

//Lleno con el valor ejemplo
void caso_de_prueba_1(){
	valor = "EjemploX";
	while(disponibilidad_de_conexion){
		resultado_almacenamiento = almacenar_valor();
		if(resultado_almacenamiento == EXIT_FAILURE){
					disponibilidad_de_conexion = 0;
		}
	}
}

//Lleno con distintos tipos de valores que ocupan una entrada
void caso_de_prueba_2(){
	int t = 0;
		while(disponibilidad_de_conexion){
			//Recibimos el valor
			if(t == 0){
				valor = "Ejemplo1";
			}
			else if(t == 1){
				valor = "Ejemplo2";
			}
			else if(t == 8){
				valor = "Algo";
			}
			else{
				valor = "Default";
			}
			resultado_almacenamiento = almacenar_valor();
			if(resultado_almacenamiento == EXIT_FAILURE){
				disponibilidad_de_conexion = 0;
			}
			t++;
		}
}

//Valor que ocupa dos entradas
void caso_de_prueba_3(){
	valor = "EjemploEjemplo";
	while(disponibilidad_de_conexion){
		resultado_almacenamiento = almacenar_valor();
		if(resultado_almacenamiento == EXIT_FAILURE){
					disponibilidad_de_conexion = 0;
		}
	}
}

//Un valor que ocupa todas las entradas
void caso_de_prueba_4(){
	valor = "80caracteeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeees";
	while(disponibilidad_de_conexion){
		resultado_almacenamiento = almacenar_valor();
		if(resultado_almacenamiento == EXIT_FAILURE){
					disponibilidad_de_conexion = 0;
		}
	}
}

//Lleno con distintos tipos de valores que ocupan una o mas entradas
void caso_de_prueba_5(){
	int t = 0;
		while(disponibilidad_de_conexion){
			//Recibimos el valor
			if(t == 0){
				valor = "20caractereeeeeeeees";
			}
			else if(t == 1){
				valor = "15caractereeees";
			}
			else if(t == 2){
				valor = "40caractereeeeeeeeeeeeeeeeeeeeeeeeeeeees";
			}
			else{
				valor = "1";
			}
			resultado_almacenamiento = almacenar_valor();
			if(resultado_almacenamiento == EXIT_FAILURE){
				disponibilidad_de_conexion = 0;
			}
			t++;
		}
}
