#pragma once // #include το πολύ μία φορά

#include "common_types.h"
#include "ADTList.h"

typedef struct set* Set;


Set set_create(CompareFunc compare, DestroyFunc destroy_value);


int set_size(Set set);


void set_insert(Set set, Pointer value);


bool set_remove(Set set, Pointer value);



DestroyFunc set_set_destroy_value(Set set, DestroyFunc destroy_value);

// Ελευθερώνει όλη τη μνήμη που δεσμεύει το σύνολο.
// Οποιαδήποτε λειτουργία πάνω στο set μετά το destroy είναι μη ορισμένη.

void set_destroy(Set set);



#define SET_BOF (SetNode)0
#define SET_EOF (SetNode)0

typedef struct set_node* SetNode;



SetNode set_first(Set set);



//Επιστρεφει τη λιστα του αμεσως επομενου κομβου που εχει μεγαλυτερο value απο το Limit_Value 

List set_next(Set set, SetNode node,Pointer Limit_Value,SetNode *prev);



List set_node_value(Set set, SetNode node);

//Επιστρεφει τη λιστα η οποια αντιστοιχει στο ορισμα Value ή κενη λιστα αν δεν υπαρχει κομβος με ισοδυναμο Value 

List set_equal_value(Set set,SetNode node,Pointer Value);

//Βρισκει δεδομενου μια ημερομηνια ,την λιστα του κομβου οπου ειτε εχει την ιδια ημερομηνια ή την αμεσως μεγαλυτερη 

List set_find_greater_equal(Set set, SetNode node,Pointer Limit_Date,SetNode *prev);

//επιστρεφει την ριζα του δεντρου 

SetNode set_root(Set set);

//Τοποθετει στην μεταβλητη count το πληθος των στοιχειων που ειναι μεγαλυτερα ή ισα του Value

void get_count_greater_equal(Set set,SetNode node,Pointer Value,int* count);