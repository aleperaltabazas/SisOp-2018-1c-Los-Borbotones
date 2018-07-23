/*
 * testPlanificador.c
 *
 *  Created on: 22 jul. 2018
 *      Author: alesaurio
 */

void testHRRN(void) {
	ALGORITMO_PLANIFICACION.tipo = HRRN;
	tiempo = 30;

	while (new_ESIs.head != NULL) {
		eliminar_ESI(&new_ESIs, new_ESIs.head->esi);
		new_ESIs.head = new_ESIs.head->sgte;
	}

	ESI esi1 = { .id = 1, .rafaga_estimada = 10, .rafaga_real = 4,
			.tiempo_arribo = 0 };
	ESI esi2 = { .id = 2, .rafaga_estimada = 6, .rafaga_real = 0,
			.tiempo_arribo = 0 };
	ESI esi3 = { .id = 3, .rafaga_estimada = 2, .rafaga_real = 0,
			.tiempo_arribo = 30 };

	agregar_ESI(&new_ESIs, esi1);
	agregar_ESI(&new_ESIs, esi2);

	executing_ESI = dame_proximo_ESI();
	log_trace(logger, "Próximo ESI: %i", executing_ESI.id);
	log_trace(logger, "Response ratio: %f", response_ratio(executing_ESI));

	executing_ESI.rafaga_real++;
	executing_ESI.rafaga_real++;
	executing_ESI.rafaga_real++;
	executing_ESI.rafaga_real++;
	tiempo++;
	tiempo++;
	tiempo++;
	tiempo++;

	desalojar();

	executing_ESI = dame_proximo_ESI();
	log_trace(logger, "Próximo ESI: %i", executing_ESI.id);
	log_trace(logger, "Response ratio: %f", response_ratio(executing_ESI));

	executing_ESI.rafaga_real++;
	executing_ESI.rafaga_real++;
	executing_ESI.rafaga_real++;
	executing_ESI.rafaga_real++;
	tiempo++;
	tiempo++;
	tiempo++;

	desalojar();
	agregar_ESI(&new_ESIs, esi3);

	executing_ESI = dame_proximo_ESI();
	log_trace(logger, "Próximo ESI: %i", executing_ESI.id);
	log_trace(logger, "Response ratio: %f", response_ratio(executing_ESI));

	desalojar();

	exit(0);

}

void testSJF(void) {
	ALGORITMO_PLANIFICACION.tipo = SJF;

	ESI esi1 = { .id = 1, .rafaga_estimada = 4, .rafaga_real = 2 };
	ESI esi2 = { .id = 2, .rafaga_estimada = 4, .rafaga_real = 0 };
	ESI esi3 = { .id = 3, .rafaga_estimada = 2, .rafaga_real = 0 };

	agregar_ESI(&new_ESIs, esi1);
	agregar_ESI(&new_ESIs, esi2);

	executing_ESI = dame_proximo_ESI();
	log_debug(logger, "Próximo ESI: %i", executing_ESI.id);

	executing_ESI.rafaga_real++;
	executing_ESI.rafaga_real++;
	executing_ESI.rafaga_real++;
	executing_ESI.rafaga_real++;

	desalojar();

	executing_ESI = dame_proximo_ESI();
	log_debug(logger, "Próximo ESI: %i", executing_ESI.id);

	executing_ESI.rafaga_real++;
	executing_ESI.rafaga_real++;
	executing_ESI.rafaga_real++;
	executing_ESI.rafaga_real++;

	desalojar();
	agregar_ESI(&new_ESIs, esi3);

	executing_ESI = dame_proximo_ESI();
	log_debug(logger, "Próximo ESI: %i", executing_ESI.id);

	desalojar();

	exit(0);
}
