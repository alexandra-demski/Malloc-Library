#include "memory.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc,char *argv[]){
	int nb_total, nb_use, nb_free;//Valeur récupérée avec une fonction get
	int ntotal, nuse, nfree;//Valeur attendue
	size_t sz_total, sz_use, sz_free, sz_trash;//Valeur récupérée avec une fonction get
	size_t stotal, suse, sfree, strash;//Valeur attendue
	void *a, *b, *c, *d, *e;

	fprintf(stderr,"\n----------------------- FUSION LIBERATION -----------------------\n\n");
	a = malloc(4);
	b = malloc(4);
	c = malloc(4);
	free(a);
	free(c);
	free(b);
	ntotal = 1;
	nuse = 0;
	nfree = 1;
	stotal = 4+4+4+40+40;
	suse = 0;
	sfree = 4+4+4+40+40;
	strash = 0;
	//print_stats();

	nb_total = get_nb_total();
	nb_use = get_nb_use();
	nb_free = get_nb_free();
	sz_total = get_size_total();
	sz_use = get_size_use();
	sz_free = get_size_free();
	sz_trash = get_size_trash();

	if(nb_total == ntotal && nb_use == nuse && nb_free == nfree && sz_total == stotal && sz_use == suse && sz_free == sfree && sz_trash == strash){
		fprintf(stderr,"VALIDE\n");
	} else {
		fprintf(stderr,"ECHEC\n");
	}

}
