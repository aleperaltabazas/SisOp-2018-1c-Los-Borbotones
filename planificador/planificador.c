/*
 * ========================
 * ===== PLANIFICADOR =====
 * ========================
 */

#include "planificador.h"

#define PACKAGE_SIZE 1024
//Estos tres define van a cambiar, para poder cambiar ip y puerto en runtime (en caso de que esten ocupados) y para poder mandar datos de tamaño no fijo

int i;

ESI test;

int main(int argc, char** argv) {
	iniciar();

	int socket_coordinador = conectar_a(IP_COORDINADOR, PUERTO_COORDINADOR,
			mensajePlanificador);
	int listening_socket = levantar_servidor(PUERTO_PLANIFICADOR);
	int socketCliente;

	while (1) {
		socketCliente = manejar_cliente(listening_socket, socketCliente,
				mensajePlanificador);
	}

	loggear("Cerrando sesion...");

	close(socket_coordinador);
	close(listening_socket);
	close(socketCliente);

	cerrar();

	return EXIT_SUCCESS;
}

void iniciar(void) {
	/*pthread_create(hiloDeConsola, NULL, consola, NULL);
	 pthread_detach(hiloDeConsola);*/
	iniciar_log("Planificador", "Nace el planificador...");

	algoritmo_planificacion.desalojo = false;
	algoritmo_planificacion.tipo = FIFO;

	executing_ESI = malloc(sizeof(executing_ESI));
	executing_ESI->id = -1;

	ESI_id = 1;

	pthread_mutex_init(&semaforo_ejecucion, NULL);

	/*pthread_t hilo_coordinador;
	 pthread_create(&hilo_coordinador, NULL, charlar_con_el_coordi, NULL);
	 pthread_detach(hilo_coordinador);*/

	//Por ahora intento hacer una lista con todos los hilos de ESIs sin discriminarlos para simplificar
	ESIs = list_create();
	ESIs_bloqueados = list_create();
	ESIs_en_ejecucion = list_create();
	ESIs_listos = list_create();
	ESIs_finalizados = list_create();

}

/*void charlar_con_el_coordi(void) {
 int socket_coordinador = conectar_a(IP_COORDINADOR, PUERTO_COORDINADOR,
 mensajePlanificador);

 return;
 }*/

int manejar_cliente(int listening_socket, int socket_cliente, char* mensaje) {

	loggear("Esperando cliente...");

	listen(listening_socket, BACKLOG);

	log_trace(logger, "Esperando...");
	struct sockaddr_in addr; // Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
	socklen_t addrlen = sizeof(addr);

	socket_cliente = accept(listening_socket, (struct sockaddr *) &addr,
			&addrlen);

	loggear("Cliente conectado.");

	loggear("Esperando mensaje del cliente.");

	char package[PACKAGE_SIZE];

	int res = recv(socket_cliente, (void*) package, PACKAGE_SIZE, 0);

	if (res <= 0) {
		loggear("Fallo la conexion con el cliente.");
	}

	loggear("Mensaje recibido exitosamente. Identificando cliente...");
	identificar_cliente((char*) package, socket_cliente);

	loggear("Enviando mensaje al cliente.");

	send(socket_cliente, mensaje, strlen(mensaje) + 1, 0);

	loggear("Mensaje enviado. Cerrando sesion con el cliente actual.");

	return socket_cliente;
}

void identificar_cliente(char* mensaje, int socket_cliente) {
	pthread_t hilo_ESI;

	if (strcmp(mensaje, mensajePlanificador) == 0) {
		loggear(mensajePlanificador);
		loggear("Wait, what the fuck?");
	} else if (strcmp(mensaje, mensajeESI) == 0) {
		loggear(mensajeESI);

		pthread_create(&hilo_ESI, NULL, atender_ESI, (void*) socket_cliente);

		//loggear(mensajeESILista);

		pthread_detach(hilo_ESI);
	} else if (strcmp(mensaje, mensajeInstancia) == 0) {
		loggear(mensajeInstancia);
	} else {
		cerrar();
		salir_con_error("Cliente desconocido, cerrando conexion.",
				socket_cliente);
	}

	return;
}

void* atender_ESI(void* buffer) {
	int socket_ESI = (int) buffer;

	aviso_ESI aviso;

	int packageSize = sizeof(aviso);
	char* package = malloc(packageSize);

	loggear("Hilo de ESI inicializado correctamente.");

	int this_id = asignar_ID(socket_ESI);

	ESI esi = { .id = this_id, .socket = socket_ESI };

	while (1) {

		recv(socket_ESI, package, packageSize, 0);

		log_trace(logger, "Mensaje recibidio del ESI numero: %i", this_id);

		deserializar_aviso(&(aviso), &(package));

		loggear("Deserialice bien.");

		if (aviso.aviso == 0) {
			loggear("ESI terminado.");

			//procesar_cierre(socket_ESI);

			list_remove(ESIs, this_id);

			break;
		}

		else if (aviso.aviso == 1) {
<<<<<<< HEAD
			list_add_in_index(ESIs, this_id, (void*) &esi);
			loggear("ESI listo para ejecutar añadido a la cola.");
=======
			esi.tiempo_arribo = tiempo;

			esi.rafaga_estimada = estimated_time(esi);
			esi.rafaga_real = 0;

			agregar_ESI(&new_ESIs, esi);

			log_trace(logger, "ESI número %i preparado.", this_id);
>>>>>>> parent of 0d69e0d... Revert "agrego esqueleto al esi para sus operaciones"

			test = esi;
		}

		else if (aviso.aviso == 5) {
			log_trace(logger, "ESI número %i bloqueado", this_id);
		}

<<<<<<< HEAD
		else if (aviso.aviso == 11) {
			agregar_ESI(&new_ESIs, esi);
			loggear("Hizo GET.");
		}
=======
		planificar();
	}

	return NULL;
}

void kill_ESI(ESI esi) {
	int socket_ESI = esi.socket;
>>>>>>> parent of e60b783... fix'd el tema de la cola de esis

		else if (aviso.aviso == 12) {
			agregar_ESI(&new_ESIs, esi);
			loggear("Hizo SET.");
		}

		else if (aviso.aviso == 13) {
			agregar_ESI(&new_ESIs, esi);
			loggear("Hizo STORE.");
		}

		else {
			log_trace(logger, "%i", aviso.aviso);
		}

		planificar();

	}

	return NULL;
}

int asignar_ID(int socket_ESI) {
	aviso_ESI aviso = { .aviso = 1, .id = ESI_id };

	ESI_id++;

	int packageSize = sizeof(aviso.aviso) + sizeof(aviso.id);
	char* package = malloc(packageSize);

	serializar_aviso(aviso, &package);

	int envio = send(socket_ESI, package, packageSize, 0);

	if (envio < 0) {
		loggear("Fallo el envio de identificacion. Terminando ESI.");
		kill_ESI(socket_ESI);
	}

	free(package);

	return aviso.id;
}

void procesar_cierre(int socket_ESI) {
	loggear("Esperando id de ejecucion del planificador.");

	package_pedido id;

	int packageSize = sizeof(id.pedido);
	char *package = malloc(packageSize);

	int res = recv(socket_ESI, (void*) package, packageSize, 0);

	if (res != 0) {
		loggear("Solicitud confirmada.");
	} else {
		cerrar();
		salir_con_error("Fallo la solicitud.", socket_ESI);
	}

	deserializar_pedido(&(id), &(package));

	int id_as_int = (int) id.pedido;

	list_remove(ESIs, id_as_int);

}

void planificar(void) {
<<<<<<< HEAD
	if (executing_ESI->id == -1) {
		//log_trace(logger, "%i", executing_ESI->id);
		executing_ESI = first(ESIs);
=======
	if (executing_ESI.id == -1) {
		executing_ESI = first(new_ESIs);

		eliminar_ESI(&new_ESIs, executing_ESI);

		loggear("ESI eliminado de la cola de la listos.");
>>>>>>> parent of 0d69e0d... Revert "agrego esqueleto al esi para sus operaciones"
		//*executing_ESI = test;

		//log_trace(logger, "%i", executing_ESI->id);
		list_remove(ESIs, 0);
		loggear("ESI elegido.");
		ejecutar(executing_ESI);

		return;
	}

	if (algoritmo_planificacion.desalojo) {
		desalojar();
	}

	switch (algoritmo_planificacion.tipo) {
	case FIFO:
		executing_ESI = first(ESIs);
		//*executing_ESI = test;
		break;
	case SJF:
		executing_ESI = shortest(ESIs);
		break;
	case HRRN:
		executing_ESI = highest_RR(ESIs);
		break;
	default:
		loggear("FALLO EN EL ALGORITMO.");
		break;
	}

<<<<<<< HEAD
	loggear("ESI elegido.");
=======
	eliminar_ESI(&new_ESIs, executing_ESI);

	loggear("ESI eliminado de la cola de la listos.");

	log_trace(logger, "ESI número %i elegido.", executing_ESI.id);
>>>>>>> parent of 0d69e0d... Revert "agrego esqueleto al esi para sus operaciones"

	ejecutar(executing_ESI);

}

void desalojar(void) {
<<<<<<< HEAD
	//list_add(ESIs, (void*) &executing_ESI);
	executing_ESI = esi_vacio;
}

t_esi_node* crear_nodo(ESI esi) {
	t_esi_node* nodo = (t_esi_node*) malloc(sizeof(t_esi_node));
	nodo->esi = esi;
	nodo->sgte = NULL;

	return nodo;
}

void agregar_ESI(t_esi_list* lista, ESI esi) {
	t_esi_node* nodo = crear_nodo(esi);

	if (lista->head == NULL) {
		lista->head = nodo;
	} else {
		t_esi_node* puntero = lista->head;
		while (puntero->sgte != NULL) {
			puntero = puntero->sgte;
		}

		puntero->sgte = nodo;
	}

	return;
}

void destruir_nodo(t_esi_node* nodo) {
	free(nodo);
}

<<<<<<< HEAD
<<<<<<< HEAD
void eliminar_ESI(t_esi_list* lista, ESI esi) {
	if (lista->head != NULL) {
		ESI head = first(*lista);
		if (esi.id == head.id) {
			t_esi_node* eliminado = lista->head;
			lista->head = lista->head->sgte;
			destruir_nodo(eliminado);
		} else {
			t_esi_node* puntero = lista->head;
			while (puntero->esi.id != esi.id) {
				puntero = puntero->sgte;
			}

			t_esi_node* eliminado = puntero->sgte;
			puntero->sgte = eliminado->sgte;
			destruir_nodo(eliminado);
		}
	}
}

=======
>>>>>>> parent of e60b783... fix'd el tema de la cola de esis
ESI first(t_esi_list lista) {
	ESI esi = lista.head->esi;
=======
ESI first(t_esi_list lista) {
	ESI esi = lista.head->esi;

	return esi;

}

ESI shortest(t_esi_list lista) {
	t_esi_node* puntero = lista.head;

	ESI esi = first(lista);

	while (puntero->sgte != NULL) {
		if (es_mas_corto(esi, puntero->esi)) {
			esi = puntero->esi;
		}

		puntero = puntero->sgte;
	}
>>>>>>> parent of e60b783... fix'd el tema de la cola de esis

	return esi;

}

ESI shortest(t_esi_list lista) {
	t_esi_node* puntero = lista.head;
=======
	list_add(ESIs, (void*) &executing_ESI);
	executing_ESI = NULL;
}

ESI* first(t_list* lista) {
	void* elem = list_get(lista, 1);
	ESI* return_ESI = malloc(sizeof(ESI));
>>>>>>> parent of 875678e... agregado t_esi_list y t_esi_nodo porque no puedo hacer andar las listas de las commons. agregue los dos algoritmos

	loggear("Saque bien el buffer");

	deserializar_esi(elem, return_ESI);

<<<<<<< HEAD
	loggear("Copie bien.");
=======
		puntero->esi.rafaga_estimada = estimated_time(puntero->esi);
		puntero = puntero->sgte;
	}
>>>>>>> parent of 0d69e0d... Revert "agrego esqueleto al esi para sus operaciones"

	return return_ESI;
}

ESI* shortest(t_list* lista) {
	//void* elem = list_find(lista, );
	void* elem = list_get(lista, 1);
	ESI* return_ESI = malloc(sizeof(ESI));

	deserializar_esi(&(elem), return_ESI);

	return return_ESI;
}

ESI* highest_RR(t_list* lista) {
	//void* elem = list_find(lista, );
	void* elem = list_get(lista, 1);
	ESI* return_ESI = malloc(sizeof(ESI));

	deserializar_esi(&(elem), return_ESI);

	return return_ESI;
}

void cerrar(void) {
	cerrar_listas();
	free(executing_ESI);
}

void ejecutar(ESI* esi_a_ejecutar) {
	int socket_ESI = executing_ESI->socket;

	loggear("Enviando orden de ejecucion.");
	aviso_ESI orden_ejecucion = { .aviso = 2, .id = esi_a_ejecutar->id };

	int packageSize = sizeof(orden_ejecucion.aviso)
			+ sizeof(orden_ejecucion.id);
	char* message = malloc(packageSize);

	serializar_aviso(orden_ejecucion, &message);

	int envio = send(socket_ESI, message, packageSize, 0);

	if (envio < 0) {
		cerrar();
		log_error(logger, "Fallo el envio. Terminando ESI.");

		kill_ESI(esi_a_ejecutar->socket);

		free(message);
		return;
	}

	loggear("Orden enviada.");

<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
	//list_remove(ESIs, esi_a_ejecutar.id);
=======
	list_remove(ESIs, esi_a_ejecutar.id);
>>>>>>> parent of e60b783... fix'd el tema de la cola de esis

<<<<<<< HEAD
	eliminar_ESI(&new_ESIs, esi_a_ejecutar);

	loggear("ESI eliminado de la cola de la listos.");
=======
	list_remove(ESIs, esi_a_ejecutar->id);
>>>>>>> parent of 875678e... agregado t_esi_list y t_esi_nodo porque no puedo hacer andar las listas de las commons. agregue los dos algoritmos
=======
	list_remove(ESIs, esi_a_ejecutar.id);
>>>>>>> parent of e60b783... fix'd el tema de la cola de esis

=======
>>>>>>> parent of 0d69e0d... Revert "agrego esqueleto al esi para sus operaciones"
	free(message);
}

void deserializar_esi(void* esi_copiado, ESI* esi_receptor) {
	int offset = 0;

	loggear("a");

	memcpy(&esi_receptor->id, esi_copiado, sizeof(esi_receptor->id));

	loggear("b");

	offset = sizeof(esi_receptor->id);

	memcpy(&esi_receptor->socket, esi_copiado + offset,
			sizeof(esi_receptor->socket));
}

void cerrar_listas() {
	//Borro todos los datos de las listas...
	//Habria que liberar la memoria por cada elemento que fue agregado o con el clean ya alcanza?
	list_clean(ESIs);
	list_clean(ESIs_bloqueados);
	list_clean(ESIs_en_ejecucion);
	list_clean(ESIs_listos);
	list_clean(ESIs_finalizados);

	//Libero las cabezas de las listas...
	free(ESIs);
	free(ESIs_bloqueados);
	free(ESIs_en_ejecucion);
	free(ESIs_listos);
	free(ESIs_bloqueados);

}

/*	=====================
 *	===== CONSOLITA =====
 *	=====================
 */

float recibirCodigo() {
	float code = 0;
	scanf("%f", &code);
	return code;
}
void interpretarYEjecutarCodigo(float comando) {
	int opcionElegida;
	float codigoSubsiguiente;
	opcionElegida = comando / 1;
	switch (opcionElegida) {
	case 0:
		pthread_cancel(pthread_self()); //Usar esto lo hace no portable, preguntarle a Lean
		break;
	case 1:
		pausarOContinuar();
		break;
	case 2:
		codigoSubsiguiente = comando - opcionElegida;
		bloquear(codigoSubsiguiente);
		break;
	case 3:
		codigoSubsiguiente = comando - opcionElegida;
		desbloquear(codigoSubsiguiente);
		break;
	case 4:
		listar();
		break;
	case 5:
		codigoSubsiguiente = comando - opcionElegida;
		kill(codigoSubsiguiente);
		break;
	case 6:
		codigoSubsiguiente = comando - opcionElegida;
		status(codigoSubsiguiente);
		break;
	case 7:
		deadlock();
		break;
	default:
		printf(
				"Codigo incorrecto, recuerde que se introduce un codigo de tipo float \n");
		break;
	};
}
void listarOpciones() {
	printf("0 : Cancelar consola \n");
	printf("1 : Pausar o reactivar la planificación \n");
	printf("2.<ESI ID> : Bloquea al ESI elegido \n");
	printf("3.<ESI ID> : Desbloquea al ESI elegido \n");
	printf("4.<Recurso> : Lista procesos esperando dicho recurso \n");
	printf("5.<ESI ID> : Mata al ESI elegido \n");
	printf("6.<ESI ID> : Brinda el estado del ESI elegido \n");
	printf("7 : Lista los ESI en deadlock \n");
	printf("Introduzca la opcion deseada \n");
}
void* consola(void) {
	float comando;
	printf("Bienvenido a la consola interactiva para el planificador \n");
	while (1) {
		listarOpciones();
		comando = recibirCodigo();
		interpretarYEjecutarCodigo(comando);
	}
}
void pausarOContinuar(void) {
	printf("Eligio pausar o continuar \n");
}
void bloquear(float codigo) {
	printf("Eligio bloquear el ESI \n");
}
void desbloquear(float codigo) {
}
void listar(void) {
}
void kill(float codigo) {
}
void status(float codigo) {
}
void deadlock(void) {
}
