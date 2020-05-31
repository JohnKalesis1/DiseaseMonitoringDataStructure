/////////////////////////////////////////////////////////////////////////////
//
// Υλοποίηση του ADT Map μέσω Hash Table με open addressing (linear probing)
//
/////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>

#include "ADTMap.h"





// Το μέγεθος του Hash Table ιδανικά θέλουμε να είναι πρώτος αριθμός σύμφωνα με την θεωρία.
// Η παρακάτω λίστα περιέχει πρώτους οι οποίοι έχουν αποδεδιγμένα καλή συμπεριφορά ως μεγέθη.
// Κάθε re-hash θα γίνεται βάσει αυτής της λίστας. Αν χρειάζονται παραπάνω απο 1610612741 στοχεία, τότε σε καθε rehash διπλασιάζουμε το μέγεθος.
int prime_sizes[] = {53,  97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317, 196613, 393241,
	786433, 1572869, 3145739, 6291469, 12582917, 25165843, 50331653, 100663319, 201326611, 402653189, 805306457, 1610612741};

// Χρησιμοποιούμε open addressing, οπότε σύμφωνα με την θεωρία, πρέπει πάντα να διατηρούμε
// τον load factor του  hash table μικρότερο ή ίσο του 0.5, για να έχουμε αποδoτικές πράξεις
#define MAX_LOAD_FACTOR 0.9

// Δομή του κάθε κόμβου που έχει το hash table (με το οποίο υλοιποιούμε το map)
struct map_node{
	Pointer key;		// Το κλειδί που χρησιμοποιείται για να hash-αρουμε
	Pointer value;  	// Η τιμή που αντισοιχίζεται στο παραπάνω κλειδί
	MapNode next;	// Μεταβλητή για να μαρκάρουμε την κατάσταση των κόμβων (βλέπε διαγραφή)
	bool state;
};

// Δομή του Map (περιέχει όλες τις πληροφορίες που χρεαζόμαστε για το HashTable)
struct map {
	MapNode array;				// Ο πίνακας που θα χρησιμοποιήσουμε για το map (remember, φτιάχνουμε ένα hash table)
	int capacity;				// Πόσο χώρο έχουμε δεσμεύσει.
	int size;					// Πόσα στοιχεία έχουμε προσθέσει
	CompareFunc compare;		// Συνάρτηση για σύγκρηση δεικτών, που πρέπει να δίνεται απο τον χρήστη
	HashFunc hash_function;		// Συνάρτηση για να παίρνουμε το hash code του κάθε αντικειμένου.
	DestroyFunc destroy_key;	// Συναρτήσεις που καλούνται όταν διαγράφουμε έναν κόμβο απο το map.
	DestroyFunc destroy_value;
};

Map map_create(CompareFunc compare, DestroyFunc destroy_key, DestroyFunc destroy_value) {
	// Δεσμεύουμε κατάλληλα τον χώρο που χρειαζόμαστε για το hash table
	Map map = malloc(sizeof(*map));
	map->capacity = prime_sizes[0];
	map->array = malloc(map->capacity * sizeof(struct map_node));

	// Αρχικοποιούμε τους κόμβους που έχουμε σαν διαθέσιμους.
	for (int i = 0; i < map->capacity; i++)
		map->array[i].state=0;

	map->size = 0;
	map->compare = compare;
	map->destroy_key = destroy_key;
	map->destroy_value = destroy_value;

	return map;
}

// Επιστρέφει τον αριθμό των entries του map σε μία χρονική στιγμή.
int map_size(Map map) {
	return map->size;
}

// Συνάρτηση για την επέκταση του Hash Table σε περίπτωση που ο load factor μεγαλώσει πολύ.
static void rehash(Map map) {
	// Αποθήκευση των παλιών δεδομένων
	int old_capacity = map->capacity;
	MapNode old_array = map->array;

	// Βρίσκουμε τη νέα χωρητικότητα, διασχίζοντας τη λίστα των πρώτων ώστε να βρούμε τον επόμενο. 
	int prime_no = sizeof(prime_sizes) / sizeof(int);	// το μέγεθος του πίνακα
	for (int i = 0; i < prime_no; i++) {					// LCOV_EXCL_LINE
		if (prime_sizes[i] > old_capacity) {
			map->capacity = prime_sizes[i]; 
			break;
		}
	}
	// Αν έχουμε εξαντλήσει όλους τους πρώτους, διπλασιάζουμε
	if (map->capacity == old_capacity)					// LCOV_EXCL_LINE
		map->capacity *= 2;								// LCOV_EXCL_LINE

	// Δημιουργούμε ένα μεγαλύτερο hash table
	map->array = malloc(map->capacity * sizeof(struct map_node));
	for (int i = 0; i < map->capacity; i++)
		map->array[i].state=0;

	map->size=0;
	for (int i = 0; i < old_capacity; i++)  {
		MapNode temp,node=&old_array[i];
		bool first=1;
		if (old_array[i].state==1)  {
			while (node!=NULL)  {
				map_insert(map,node->key,node->value);
				temp=node;
				node=node->next;
				if (first!=1)  {
					free(temp);
				}
				first=0;
			}
		}
	}
	//Αποδεσμεύουμε τον παλιό πίνακα ώστε να μήν έχουμε leaks
	free(old_array);
}

// Εισαγωγή στο hash table του ζευγαριού (key, item). Αν το key υπάρχει,
// ανανέωση του με ένα νέο value, και η συνάρτηση επιστρέφει true.

void map_insert(Map map, Pointer key, Pointer value) {
	// Σκανάρουμε το Hash Table μέχρι να βρούμε διαθέσιμη θέση για να τοποθετήσουμε το ζευγάρι,
	// ή μέχρι να βρούμε το κλειδί ώστε να το αντικαταστήσουμε.
	bool already_in_map = false;
	uint pos=map->hash_function(key) % map->capacity;
	MapNode node = &(map->array[pos]);
	if (node->state==1)  {
		while (node->next!=NULL)  {
			if (map->compare(key,node->key)==0)  {
				already_in_map=true;
				break;
			}
			node=node->next;
		}
		if (map->compare(key,node->key)==0)  {
			already_in_map=true;
		}
		if (already_in_map==true)  {
			if (map->destroy_value!=NULL)  {
				map->destroy_value(node->value);
			}
			if (map->destroy_key!=NULL)  {
				map->destroy_key(node->key);
			}
			node->key=key;
			node->value = value;
		}
		else  {
			map->size++;
			node->next=malloc(sizeof(struct map_node));
			node->next->state=1;
			node->next->key= key;
			node->next->next= NULL;
			node->next->value = value;
		}
	}
	else  {
		map->size++;
		node->state=1;
		node->key = key;
		node->next= NULL;
		node->value = value;

	}
	
	// Αν με την νέα εισαγωγή ξεπερνάμε το μέγιστο load factor, πρέπει να κάνουμε rehash
	float load_factor = (float)map->size / map->capacity;
	if (load_factor > MAX_LOAD_FACTOR)
		rehash(map);
}

// Διαργραφή απο το Hash Table του κλειδιού με τιμή key
bool map_remove(Map map, Pointer key) {
	if (map_find(map,key)!=MAP_EOF)  {
		uint pos= map->hash_function(key) % map->capacity;
		MapNode prev_node=NULL,node=&(map->array[pos]);
		while (node!=NULL)  {
			if (node->key==key)  {
				if (prev_node!=NULL)  {
					prev_node->next=node->next;
				}
				else  {
					if (node->next!=NULL)  {
						map->array[pos]=*(node->next);
					}
					else  {
						map->array[pos].state=0;
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

// Αναζήτηση στο map, με σκοπό να επιστραφεί το value του κλειδιού που περνάμε σαν όρισμα.

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

// Απελευθέρωση μνήμης που δεσμεύει το map
void map_destroy(Map map) {
	for (int i = 0; i < map->capacity; i++) {
		MapNode /*temp_node,*/node=&map->array[i];
		if (node->state!=0) {
			while (node!=NULL)  {
				if (map->destroy_key != NULL)
					map->destroy_key(node->key);
				if (map->destroy_value != NULL)
					map->destroy_value(node->value);
			
			//temp_node=node;
			node=node->next;
			//free(temp_node);
			}
		}
	}

	free(map->array);
	free(map);
}

/////////////////////// Διάσχιση του map μέσω κόμβων ///////////////////////////

MapNode map_first(Map map) {
	//Ξεκινάμε την επανάληψή μας απο το 1ο στοιχείο, μέχρι να βρούμε κάτι όντως τοποθετημένο
	for (int i = 0; i < map->capacity; i++)  {
		MapNode node=&(map->array[i]) ;
		if (  node->state != 0)  {
			return &map->array[i];
		}	
	}
	return MAP_EOF;
}

MapNode map_next(Map map, MapNode node) {
	MapNode next=node->next;
	if (next!=NULL)  {
		return next;
	}
	else  {
		uint pos=(map->hash_function(node->key)+1)%map->capacity;
		for (int i=1;pos<map->capacity;i++)  {
			if (map->array[pos].state!=0)  {
				return &map->array[pos];
			}
			else  {
				pos=pos+1;
			}
		}
	}
	return MAP_EOF;
}

Pointer map_node_key(Map map, MapNode node) {
	return node->key;
}

Pointer map_node_value(Map map, MapNode node) {
	return node->value;
}

MapNode map_find_node(Map map, Pointer key) {
	// Διασχίζουμε τον πίνακα, ξεκινώντας από τη θέση που κάνει hash το key, και για όσο δε βρίσκουμε EMPTY
	uint pos= map->hash_function(key) % map->capacity;
	MapNode node=&(map->array[pos]);
	if (node->state==1)  {
		while (node!=NULL)  {
			if (node->key==key)  {
				return node;
			}
			node=node->next;
		}
	}
	return MAP_EOF;
}

// Αρχικοποίηση της συνάρτησης κατακερματισμού του συγκεκριμένου map.
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