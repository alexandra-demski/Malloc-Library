#include "memory.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc,char *argv[]){
	int test = 3;
	int nb_total, nb_use, nb_free;//Valeur récupérée avec une fonction get
	int ntotal, nuse, nfree;//Valeur attendue
	size_t sz_total, sz_use, sz_free, sz_trash;//Valeur récupérée avec une fonction get
	size_t stotal, suse, sfree, strash;//Valeur attendue
	void *a;

	for(int i=0; i!=test; i++){
		if(i==0){
			fprintf(stderr,"\n----------------------- ALLOCATION SIMPLE -----------------------\n\n");
			ntotal = 1;
			nuse = 1;
			nfree = 0;
			stotal = 4;
			suse = 4;
			sfree = 0;
			strash = 0;
			a = malloc(4);
			//print_stats();
		} else if(i==1){
			fprintf(stderr,"\n----------------------- LIBERATION SIMPLE -----------------------\n\n");
			ntotal = 1;
			nuse = 0;
			nfree = 1;
			stotal = 4;
			suse = 0;
			sfree = 4;
			strash = 0;
			free(a);
			//print_stats();
		} else if(i==2){
			fprintf(stderr,"\n----------------------- ALIGNEMENT -----------------------\n\n");
			ntotal = 2;
			nuse = 1;
			nfree = 1;
			stotal = 4+12;
			suse = 12;
			sfree = 4;
			strash = 0;
			a = malloc(10);
			//print_stats();
		}
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

	fprintf(stderr,"\n----------------------- CALLOC -----------------------\n\n");
	int *b;
	b = calloc(4,sizeof(int));
	if(b[0] == b[1] && b[0] == b[2] && b[0] == b[3] && b[0] == 0){
		fprintf(stderr,"VALIDE\n");
	} else {
		fprintf(stderr,"ECHEC\n");
	}

	fprintf(stderr,"\n----------------------- REALLOC -----------------------\n\n");
	int *c;
	c = realloc(b,12);
	if(c[0] == c[1] && c[0] == c[2] && c[0] == c[3] && c[0] == 0){
		fprintf(stderr,"VALIDE\n");
	} else {
		fprintf(stderr,"ECHEC\n");
	}

}
