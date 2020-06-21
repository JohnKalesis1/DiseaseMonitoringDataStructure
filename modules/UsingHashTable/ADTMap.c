#include <stdlib.h>
#include "ADTMap.h"

int prime_sizes[] = {53,  97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317, 196613, 393241,
	786433, 1572869, 3145739, 6291469, 12582917, 25165843, 50331653, 100663319, 201326611, 402653189, 805306457, 1610612741};


#define MAX_LOAD_FACTOR 0.9

struct map_node{
	Pointer key;		
	Pointer value;  	
	MapNode next;	
	bool state; // false για αδεια λιστα,true για λιστα που εχει στοιχεια 
};


struct map {
	MapNode array;				
	int capacity;				
	int size;					
	CompareFunc compare;		
	HashFunc hash_function;		
	DestroyFunc destroy_key;	
	DestroyFunc destroy_value;
};

Map map_create(CompareFunc compare, DestroyFunc destroy_key, DestroyFunc destroy_value) {
	Map map = malloc(sizeof(*map));
	map->capacity = prime_sizes[0];
	map->array = malloc(map->capacity * sizeof(struct map_node));

	for (int i = 0; i < map->capacity; i++)
		map->array[i].state=0;

	map->size = 0;
	map->compare = compare;
	map->destroy_key = destroy_key;
	map->destroy_value = destroy_value;

	return map;
}

int map_size(Map map) {
	return map->size;
}

static void rehash(Map map) {
	int old_capacity = map->capacity;
	MapNode old_array = map->array;

	int prime_no = sizeof(prime_sizes) / sizeof(int);	
	for (int i = 0; i < prime_no; i++) {					
		if (prime_sizes[i] > old_capacity) {
			map->capacity = prime_sizes[i]; 
			break;
		}
	}
	if (map->capacity == old_capacity)					
		map->capacity *= 2;								

	map->array = malloc(map->capacity * sizeof(struct map_node));
	for (int i = 0; i < map->capacity; i++)
		map->array[i].state=0;

	map->size=0;  //εφοσον εχουμε αυξησει το μεγεθος του map , αντιγραφουμε τα στοιχεια
	for (int i = 0; i < old_capacity; i++)  {
		MapNode temp,node=&old_array[i];
		bool first=1;
		if (old_array[i].state==1)  {
			while (node!=NULL)  {
				map_insert(map,node->key,node->value); //insert στο νεο map
				temp=node;
				node=node->next;
				if (first!=1)  { 
					free(temp); //δε κανουμε free το πρωτο στοιχειο της λιστας , διοτι τα πρωτα στοιχεια καθε hash
				}				//θα γινουν free απο τη map_destrοy
				first=0;
			}
		}
	}
	free(old_array);
}



void map_insert(Map map, Pointer key, Pointer value) {
	bool already_in_map = false;
	uint pos=map->hash_function(key) % map->capacity;
	MapNode node = &(map->array[pos]);
	if (node->state==1)  { //σημαινει οτι υπαρχουν ηδη αλλα στοιχεια στο hash
		while (node->next!=NULL)  { //διασχιζουμε τη "λιστα" κια τοποθετουμε στο τελος το στοιχειο  
			if (map->compare(key,node->key)==0)  {
				already_in_map=true; // σημαια οτι υπαρχει ισοδυναμο κλειδι
				break;
			}
			node=node->next;
		}
		if (map->compare(key,node->key)==0)  {
			already_in_map=true;  //περιπτωση να ειναι το τελευταιο
		}
		if (already_in_map==true)  {
			if (map->destroy_value!=NULL)  {
				map->destroy_value(node->value);
			}
			if (map->destroy_key!=NULL)  {
				map->destroy_key(node->key);
			}
			node->key=key;  //αντικατασταση 
			node->value = value; 
		}
		else  { //προσθεση στο τελος 
			map->size++;   
			node->next=malloc(sizeof(struct map_node));
			node->next->state=1; // καθε κομβος με στοιχειο πρεπει να εχει κατασταση true για δουλεψει η remove
			node->next->key= key;
			node->next->next= NULL;
			node->next->value = value;
		}
	}
	else  {
		map->size++;  
		node->state=1; // τοποθετουμε το πρωτο στοιχειο στη κενη "λιστα"
		node->key = key;
		node->next= NULL;
		node->value = value;

	}
	
	float load_factor = (float)map->size / map->capacity;
	if (load_factor > MAX_LOAD_FACTOR)
		rehash(map);
}

bool map_remove(Map map, Pointer key) {
	if (map_find(map,key)!=MAP_EOF)  {
		uint pos= map->hash_function(key) % map->capacity;
		MapNode prev_node=NULL,node=&(map->array[pos]);
		while (node!=NULL)  {
			if (map->compare(node->key,key)==0)  {  //αν το βορυμε το διαγραφουμε
				map->size--;          
				if (prev_node!=NULL)  {
					prev_node->next=node->next;  //συνδεση του προηγουμενου κομβου με τον επομενο 
				}
				else  {  //το στοιχειο που θα αφαιρεσουμε εινια το πρωτο στοιχειο
					if (node->next!=NULL)  { //αν εχει επομενο 
						map->array[pos]=*(node->next);  //συνδεουμε την αφετηρια της "λιστας" με το επομενο
					}
					else  {
						map->array[pos].state=0; //αλλιως σηματοδοτουμε οτι δεν υπαρχουν στοιχεια στο hash 
					}
				}
				if (map->destroy_key != NULL)
					map->destroy_key(node->key);
				if (map->destroy_value != NULL)
					map->destroy_value(node->value);

				if (map->array[pos].state!=0)  {
					free(node);
				}
				return true;
			}
			prev_node=node;
			node=node->next;
		}
	}
	return false;
}


Pointer map_find(Map map, Pointer key) {
	MapNode node = map_find_node(map, key);
	if (node != MAP_EOF)
		return node->value;
	else
		return NULL;
}


DestroyFunc map_set_destroy_key(Map map, DestroyFunc destroy_key) {
	DestroyFunc old = map->destroy_key;
	map->destroy_key = destroy_key;
	return old;
}

DestroyFunc map_set_destroy_value(Map map, DestroyFunc destroy_value) {
	DestroyFunc old = map->destroy_value;
	map->destroy_value = destroy_value;
	return old;
}

void map_destroy(Map map) {
	for (int i = 0; i < map->capacity; i++) {
		MapNode node=&map->array[i];
		if (node->state!=0) {  //διασχιζουμε καθε hash το οποιο εχει στοιχεια
			while (node!=NULL)  {
				if (map->destroy_key != NULL)  //και καλουμε τις destroy
					map->destroy_key(node->key);
				if (map->destroy_value != NULL)
					map->destroy_value(node->value);
			
			node=node->next;
			}
		}
	}

	free(map->array);
	free(map);
}


MapNode map_first(Map map) {
	for (int i = 0; i < map->capacity; i++)  { //ψαχνουμε το πρωτο στοιχειο που να εχει true κατασταση
		MapNode node=&(map->array[i]) ;
		if (  node->state != 0)  {
			return &map->array[i];
		}	
	}
	return MAP_EOF;
}

MapNode map_next(Map map, MapNode node) {
	MapNode next=node->next;
	if (next!=NULL)  {  //αν υπαρχει επομενο στοιχειο με ιδιο hash το επιστρεφουμε 
		return next;
	}
	else  {
		uint pos=(map->hash_function(node->key)+1)%map->capacity; //αλλιως 
		for (int i=1;pos<map->capacity;i++)  {  
			if (map->array[pos].state!=0)  {  //επιστρεφουμε το επομενο hash που εχει στοιχεια
				return &map->array[pos];
			}
			else  {
				pos=pos+1;
			}
		}
	}
	return MAP_EOF; // αν φτασουμε στο τελος του πινακα 
}

Pointer map_node_key(Map map, MapNode node) {
	return node->key;
}

Pointer map_node_value(Map map, MapNode node) {
	return node->value;
}

MapNode map_find_node(Map map, Pointer key) {
	uint pos= map->hash_function(key) % map->capacity;
	MapNode node=&(map->array[pos]);
	if (node->state==1)  {
		while (node!=NULL)  {
			if (map->compare(node->key,key)==0)  {
				return node;
			}
			node=node->next;
		}
	}
	return MAP_EOF;
}

void map_set_hash_function(Map map, HashFunc func) {
	map->hash_function = func;
}

uint hash_string(Pointer value) {
	// djb2 hash function, απλή, γρήγορη, και σε γενικές γραμμές αποδοτική
    uint hash = 5381;
    for (char* s = value; *s != '\0'; s++)
		hash = (hash << 5) + hash + *s;			// hash = (hash * 33) + *s. Το foo << 5 είναι γρηγορότερη εκδοχή του foo * 32.
    return hash;
}

uint hash_int(Pointer value) {
	return *(int*)value;
}

uint hash_pointer(Pointer value) {
	return (size_t)value;				// cast σε sizt_t, που έχει το ίδιο μήκος με έναν pointer
}