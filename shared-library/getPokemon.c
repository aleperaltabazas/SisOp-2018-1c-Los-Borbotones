/*
 * getPokemon.c
 *
 *  Created on: 28 jun. 2018
 *      Author: utnso
 */

#include <stdint.h>

char* getPokemon(uint32_t id){
	char* pokemon;

	switch(id){
	case 1:
		pokemon = "Bulbasaur";
		break;
	case 2:
		pokemon = "Ivysaur";
		break;
	case 3:
		pokemon = "Venusaur";
		break;
	case 4:
		pokemon = "Charmander";
		break;
	case 5:
		pokemon = "Charmeleon";
		break;
	case 6:
		pokemon = "Charizard";
		break;
	case 7:
		pokemon = "Squirtle";
		break;
	case 8:
		pokemon = "Wartorle";
		break;
	case 9:
		pokemon = "Blastoise";
		break;
	case 10:
		pokemon = "Caterpie";
		break;
	case 11:
		pokemon = "Metapod";
		break;
	case 12:
		pokemon = "Butterfree";
		break;
	}

	return pokemon;
}
