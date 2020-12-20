#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

//#define DEBUG_ENTER
//#define DEBUG_EXIT 
//#define DEBUG_STAT 

//Structure définissant les métadonnées d'un block alloué
struct block {
	size_t size;
	size_t used_size;
	struct block *next;
	struct block *previous;
	int free;
};
//Taille des métadonnées
#define META_SIZE sizeof(struct block) // = 40

//Début de la liste des métadonnées (au début vaut NULL car 0 block alloué)
void *first = NULL;

//Vérifier si un nombre est un multiple de 4
int multiple4(size_t n){
	int x = (int)n;
	while(x > 0){//Tant que n n'est pas plus petit que 0
		x = x-4;//On enlève 4 à n
	}
	if(x == 0){//Si n à la fin vaut 0 c'est un multiple de 4
		return 1;//On retourne true
	}
	return 0;//Sinon ce n'est pas un multiple de 4
}

//Aligner la taille si besoin
size_t align(size_t size){
	size_t n = size;
	if(!multiple4(n)){//Si la taille n'est pas un multiple de 4
		//On divise la taille par 4, on ne garde que la partie entière, supérieur, puis on multiplie par 4 : on obtient le plus proche multiple de 4 de la taille, tout en étant plus grand que la taille
		return ((n/4)+1)*4;
		//exemple avec 7 : 7/4 = 1.75, on garde donc 2, 2*4 = 8, on obtient un multiple de 4 plus grand que la taille demandé, le block est donc aligné.
	}
	return n;//La taille est déjà un multiple de 4 donc on renvoit le même nombre
}

//Retrouver l'adresse du bloc sans les métadonnées
struct block *_block_ptr(void *ptr){
	return (struct block*)ptr - 1;
}

//Trouver un bloc vide de la bonne taille
struct block *_find_block(struct block **last, size_t size){
	struct block *current = first;//On débute la recherche au début de la liste
	struct block *optimal = NULL;//Le meilleur block trouvé

	while(current){//Tant qu'on a un block
		if(current->free && current->size >= size){//Si le block est libre et qu'il a une taille suffisante
			if(optimal){//Si on a déjà trouvé un block
				if(optimal->size > current->size){//Si la taille du block optimal n'est plus la plus proche
					optimal = current;//On le remplace par le block actuel
				}
			} else {//Si c'est le premier
				optimal = current;
			}
		}
		*last = current;
		current = current->next;//On continue de parcourir
	}
	//Découpage d'un bloc en deux si possible
	if(optimal && (optimal->size >= (size + META_SIZE + 4))){//Si on a trouvé un bloc et que sa taille est suffisante 
		struct block *next, *new;
		next = optimal->next;//Le block après l'optimal

		new = _block_ptr(optimal) + size;

		new->next = next;//new pointe en l'avant vers la next
		optimal->next = new;//optimal pointe en l'avant vers new

		new->previous = optimal;//new pointe en l'arrière vers optimal
		if(next != NULL){//Si next existe
			next->previous = new;//il pointe en l'arrière vers new
		}

		new->size = optimal->size - size - META_SIZE;
		optimal->size = size;

		new->free = 1;
		new->used_size = 0;
	}
	return optimal;//La boucle s'arrête quand on a trouvé un block (retourne NULL si on a parcouru toute la liste sans succès car next du dernier block vaut NULL)
}

//Allouer un nouveau bloc dans le heap
struct block *_new_block(struct block* last, size_t size){
	struct block *b;
	b = sbrk(0); //On vérifie la position de break
	void *ptr = sbrk(size + META_SIZE);//On alloue la taille du block + la taille des métadonnées
	if(ptr == (void*)-1)//Si sbrk a échoué
		return NULL; //On le signal par un retour NULL
	if(last){//vaut NULL à la première utilisation de malloc
		last->next = b;//Si last ne vaut pas NULL on ajoute la block à la chaine
		b->previous = last;
	}
	//On ajoute les informations du block
	b->size = size;
	b->next = NULL;//Il n'a pas de successeur pour le moment
	b->free = 0;
	b->used_size = size;//Il est utilisé à sa capacité maximale
	return b;
}

//Allouer un bloc en mémoire en précisant la taille voulue
void *_malloc (size_t size) {
	struct block *b;
	size_t s;
	s = align(size);

	if(s <= 0)//Gestion erreur taille
		return NULL;
	if(!first){//Premier appel à malloc
		b = _new_block(NULL,s);
		if(!b)//Si l'allocation d'un nouveau bloc a échouée
			return NULL;
		first = b;//Puisque c'est le premier appel de malloc, ce block est le début de la chaine
	} else {//N-ième appel
		struct block *last = first;
		b = _find_block(&last, s);//On recherche un block libre
		if(!b){//Si on en a pas trouvé
			b = _new_block(last, s);//On alloue un block
			if(!b)//Si l'allocation a échouée
				return NULL;//On le signale avec un retour NULL
		} else {//Si on a trouvé un block libre
			b->free = 0;//On indique qu'il est utilisé
			b->used_size = s;//On indique la taille utilisée (pas forcément sa taille exact)
		}
	}
	return(b+1);//On retourne b+1 pour ne pas inclure les métadonnées
}

void _free(void *ptr) {
	if(!ptr)//Si on a pas de pointeur
		return;//La fonction free s'arrête
	struct block *b;
	b = _block_ptr(ptr);//On récupère les métadonnées du block
	b->free = 1;//On indique qu'il est inutilisé
	b->used_size = 0;
	struct block *n, *p;
	n = b->next;//Le block après l'actuel
	p = b->previous;//Le block avant l'actuel
	if(n && n->free){//Si le block suivant est libre
		b->next = n->next;//On l'intègre dans l'actuel
		b->size = b->size + n->size + META_SIZE;
	}
	if(p && p->free){//Si le block précécdent est libre
		p->next = b->next;
		p->size = p->size + b->size + META_SIZE;
	}
}

//Réallouer un bloc de mémoire dans le tas en copiant le contenu de l'ancien
void *_realloc(void *ptr, size_t size){
	if(!ptr)//Si le block n'existe pas
		return malloc(size);//On fait un simple malloc
	struct block *b_ptr = _block_ptr(ptr);//On retrouve le block alloué
	if(b_ptr->size >= size)//Si on a assez de place dans le block (du à l'alignement ou l'utilisation d'un block déjà alloué)
		return ptr;
	//Si il y a pas assez de places, on doit appeler malloc
	void *new_ptr;//Le nouveau bloc
	new_ptr = malloc(size);//malloc va nous renvoyer un nouveau block alloué ou un block libre
	if(!new_ptr)//Si malloc a échoué
		return NULL;//On le signal avec un renvoie NULL
	memcpy(new_ptr, ptr, b_ptr->used_size);//On copie dans new_ptr le contenu de ptr qui a la taille de b_ptr->used_size
	free(ptr);//On libère le block
	return new_ptr;//Et on renvoie le nouveau block
}

//Allouer un bloc en mémoire en mettant à 0 ses bits
void *_calloc(size_t nmemb, size_t size){
	size_t s = nmemb * size;
	void *ptr = malloc(s);
	memset(ptr, 0, size);//permet de remplir une zone mémoire, identifiée par son adresse et sa taille, avec une valeur précise
	return ptr;
}

void print_stats(){
	int nb_use = 0, nb_free = 0;//Nombre de blocs utilisés ou non
	size_t size_use = 0, size_free = 0, size_trash = 0;//Taille des blocs utilisés, libres, et la quantité gâchée
	struct block *current = first;//On débute la recherche au début de la liste

	while(current){//Tant qu'on a un block
		if(current->free){//Si il est libre
			nb_free++;
			size_free = size_free + current->size;
		} else {//Si il est utilisé
			nb_use++;
			size_use = size_use + current->used_size;
			size_trash = size_trash + (current->size - current->used_size);
		}
		current = current->next;//On continue de parcourir
	}
	fprintf(stderr,"\n----- CALCUL STATISTIQUES EN COURS -----\n\n");
	fprintf(stderr,"Nombre de block alloué : %d\n",nb_use+nb_free);
	fprintf(stderr,"Nombre de block alloué utilisé : %d\n",nb_use);
	fprintf(stderr,"Nombre de block alloué libre : %d\n",nb_free);
	fprintf(stderr,"Taille totale allouée : %lu\n",size_use+size_free+size_trash);
	fprintf(stderr,"Taille allouée utilisé : %lu\n",size_use);
	fprintf(stderr,"Taille allouée libre : %lu\n",size_free);
	fprintf(stderr,"Taille allouée gaspillée : %lu",size_trash);
	fprintf(stderr,"\n\n----- CALCUL STATISTIQUES TERMINER -----\n");
}

int get_nb_total(){
	int nb = 0;
	struct block *current = first;//On débute la recherche au début de la liste
	while(current){//Tant qu'on a un block
		nb++;
		current = current->next;//On continue de parcourir
	}
	return nb;

}

int get_nb_use(){
	int nb = 0;
	struct block *current = first;//On débute la recherche au début de la liste
	while(current){//Tant qu'on a un block
		if(!current->free){//Si il est libre
			nb++;
		}
		current = current->next;//On continue de parcourir
	}
	return nb;
}

int get_nb_free(){
	int nb = 0;
	struct block *current = first;//On débute la recherche au début de la liste
	while(current){//Tant qu'on a un block
		if(current->free){//Si il est libre
			nb++;
		}
		current = current->next;//On continue de parcourir
	}
	return nb;
}

size_t get_size_total(){
	size_t nb = 0;
	struct block *current = first;//On débute la recherche au début de la liste
	while(current){//Tant qu'on a un block
		nb = nb + current->size;
		current = current->next;//On continue de parcourir
	}
	return nb;
}

size_t get_size_use(){
	size_t nb = 0;
	struct block *current = first;//On débute la recherche au début de la liste
	while(current){//Tant qu'on a un block
		if(!current->free){//Si il est libre
			nb = nb + current->used_size;
		}
		current = current->next;//On continue de parcourir
	}
	return nb;
}

size_t get_size_free(){
	size_t nb = 0;
	struct block *current = first;//On débute la recherche au début de la liste
	while(current){//Tant qu'on a un block
		if(current->free){//Si il est libre
			nb = nb + current->size;
		}
		current = current->next;//On continue de parcourir
	}
	return nb;
}

size_t get_size_trash(){
	size_t nb = 0;
	struct block *current = first;//On débute la recherche au début de la liste
	while(current){//Tant qu'on a un block
		if(!current->free){//Si il est libre
			nb = nb + (current->size - current->used_size);
		}
		current = current->next;//On continue de parcourir
	}
	return nb;
}

void *malloc (size_t size) {
	void *p;
	#ifdef DEBUG_ENTER
	fprintf(stderr,"-> enter malloc\n");
	#endif
	p = _malloc(size);
	#ifdef DEBUG_EXIT
	fprintf(stderr,"-> exit malloc\n");
	#endif
	#ifdef DEBUG_STAT
	print_stats();
	#endif
	return p;
}

void free(void *p) {
	#ifdef DEBUG_ENTER
	fprintf(stderr,"-> enter free\n");
	#endif
	_free(p);
	#ifdef DEBUG_EXIT
	fprintf(stderr,"-> exit free\n");
	#endif
	#ifdef DEBUG_STAT
	print_stats();
	#endif
}

void * realloc(void *ptr, size_t size) {
	void *p;
	#ifdef DEBUG_ENTER
	fprintf(stderr,"-> enter realloc\n");
	#endif
	p = _realloc(ptr, size);
	#ifdef DEBUG_EXIT
	fprintf(stderr,"-> exit realloc\n");
	#endif
	#ifdef DEBUG_STAT
	print_stats();
	#endif
	return p;
}

void * calloc(size_t count, size_t size) {
	void *p;
	#ifdef DEBUG_ENTER
	fprintf(stderr,"-> enter calloc\n");
	#endif
	p = _calloc(count, size);
	#ifdef DEBUG_EXIT
	fprintf(stderr,"-> exit calloc\n");
	#endif
	#ifdef DEBUG_STAT
	print_stats();
	#endif
	return p;
}
