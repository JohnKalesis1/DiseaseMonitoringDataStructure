///////////////////////////////////////////////////////////
//
// Υλοποίηση του ADT Set μέσω AVL Tree
//
///////////////////////////////////////////////////////////

#include <stdlib.h>
#include <assert.h>
#include "ADTList.h"
#include "Set_Mutated.h"
#include "DiseaseMonitor.h"
#include <string.h>


struct set {
	SetNode root;				
	int size;					
	CompareFunc compare;		
	DestroyFunc destroy_value;	
};

struct set_node {
	SetNode left, right;				
    Pointer value; //η αντιπροσωπευτικη τιμη του κομβου
    int height;				
    int left_count; // αριθμος των στοιχειων που υπαρχουν στο αριστερο υποδεντρο
    int right_count;// αριθμος των στοιχειων που υπαρχουν στο δεξι υποδεντρο
    List id_list;   // η λιστα που περιεχει τα στοιχεια που εχουν ισοδυναμη τιμη Value
};

static int int_max(int a, int b) {
	return (a > b) ? a : b ;
}

static int node_height(SetNode node) {
	if (!node) return 0;
	return node->height;
}

static void node_update_height(SetNode node) {
	node->height = 1 + int_max(node_height(node->left), node_height(node->right));
}

static int node_balance(SetNode node) {
	return node_height(node->left) - node_height(node->right);
}

static SetNode node_rotate_left(SetNode node) {
	SetNode right_node = node->right;
	SetNode left_subtree = right_node->left;
    
    node->right_count=right_node->left_count; //ανανεωνουμε τα αθροισματα
	right_node->left_count=node->left_count+list_size(node->id_list);
	right_node->left = node;
	node->right = left_subtree;

	node_update_height(node);
	node_update_height(right_node);
	
	return right_node;
}

static SetNode node_rotate_right(SetNode node) {
	SetNode left_node = node->left;
	SetNode left_right = left_node->right;

    node->left_count=left_node->right_count; //ανανεωνουμε τα αθροισματα
	left_node->right_count=node->right_count+list_size(node->id_list);
	left_node->right = node;
	node->left = left_right;

	node_update_height(node);
	node_update_height(left_node);
	
	return left_node;
}

static SetNode node_rotate_left_right(SetNode node) {
	node->left = node_rotate_left(node->left);
	return node_rotate_right(node);
}

static SetNode node_rotate_right_left(SetNode node) {
	node->right = node_rotate_right(node->right);
	return node_rotate_left(node);
}

static SetNode node_repair_balance(SetNode node) {
	node_update_height(node);

	int balance = node_balance(node);
	if (balance > 1) {

		if (node_balance(node->left) >= 0) 
			return node_rotate_right(node);
		else 
			return node_rotate_left_right(node);

	} else if (balance < -1) {

		if (node_balance(node->right) <= 0)
			return node_rotate_left(node);
		else
			return node_rotate_right_left(node);
	}


	return node;
}

static SetNode node_create(Pointer value) {
	SetNode node = malloc(sizeof(*node));
	node->left = NULL;
	node->right = NULL;
    node->value=value;  
    node->id_list=list_create(NULL); //δημιουργουμε μια λιστα που θα περιεχει απο δω και περα τα ιδια στοιχεια
	list_insert_next(node->id_list,LIST_BOF,value);
	node->height = 1;		// AVL
	node->left_count=0;
    node->right_count=0;
    return node;
}

static SetNode node_find_min(SetNode node) {
	return node != NULL && node->left != NULL
		? node_find_min(node->left)				
		: node;									
}

static SetNode node_find_max(SetNode node) {
	return node != NULL && node->right != NULL
		? node_find_max(node->right)			
		: node;									
}


static SetNode node_insert(SetNode node, CompareFunc compare, Pointer value, bool* inserted, Pointer* old_value) {

	if (node == NULL) {
		*inserted = true;			
        
		return node_create(value); //αν δεν υπαρχει ιδιος κομβος μονο τοτε δημιουργουμε νεο κομβο
	}


	int compare_res = compare(value, node->value);
	if (compare_res == 0) {

		list_insert_next(node->id_list,LIST_BOF,value); //αν βρουμε ισοδυναμο value , προσθετουμε την δομη του value στη λιστα 
        *inserted = true;
        return node;

	} else if (compare_res < 0) {

        node->left_count++; //το στοιχειο θα τοποθετηθει στο αριστερο υποδεντρο , αρα αυξανουμε τον αριστερο αριθμητη
		node->left = node_insert(node->left, compare, value, inserted, old_value);
	} else {

        node->right_count++;//το στοιχειο θα τοποθετηθει στο δεξι υποδεντρο , αρα αυξανουμε τον δεξιο αριθμητη
		node->right = node_insert(node->right, compare, value, inserted, old_value);

    }

	return node_repair_balance(node);	
}

static SetNode node_remove_min(SetNode node, SetNode* min_node) {
	if (node->left == NULL) {

		*min_node = node;
		return node->right;		

	} else {

		node->left = node_remove_min(node->left, min_node);

		return node_repair_balance(node);	
	}
}

static SetNode node_remove(SetNode node, CompareFunc compare, Pointer value, bool* removed, Pointer* old_value) {
	if (node == NULL) {
		assert(node!=NULL);
		*removed = false;//λογω των συνθηκων που θα καλεσουμε την set_remove , ο κομβος που θα παμε να διαγραψουμε 
		return NULL;	 //αναγκαστικα θα υπαρχει ,αλλιως θα μειωθουν οι αριθμητες των πανω κομβων χωρις να εχει αφαιρεθει κατι

	}

	int compare_res = compare(value, node->value);
	if (compare_res == 0) {
		*removed = true;
		*old_value = node->value;
        if (list_size(node->id_list)==1)   { //αν το μεγεθος της λιστα ειναι , τοτε πρεπει να διαγαψουμε τη λιστα και το κομβο
		    if (node->left == NULL) {

		    	SetNode right = node->right;	
                //list_remove_next(node->id_list,LIST_BOF);
                list_destroy(node->id_list);
		    	free(node);
		    	return right;

		    } else if (node->right == NULL) {
		    	
		    	SetNode left = node->left;	
                //list_remove_next(node->id_list,LIST_BOF);
                list_destroy(node->id_list);
		    	free(node);
		    	return left;

		    } else {
		    	
		    	SetNode min_right;
		    	node->right = node_remove_min(node->right, &min_right);

		    	
		    	min_right->left = node->left;
		    	min_right->right = node->right;
                //list_remove_next(node->id_list,LIST_BOF);
                list_destroy(node->id_list);
		    	free(node);

		    	return node_repair_balance(min_right);	
		    }
        }
        else  {
            ListNode lnode=list_first(node->id_list); //αλλιως ψαχνουμε να βρουμε το στοιχειο που πρεπει να διαγραφει βαση του id του ,διατρεχωντας τη λιστα
            if (((Record)list_node_value(node->id_list,lnode))->id==((Record)value)->id)  {
                *removed = true;
		        *old_value = list_node_value(node->id_list,lnode);
				list_remove_next(node->id_list,LIST_BOF);
                return node;
            }
            while (list_next(node->id_list,lnode)!=LIST_EOF)  {
                if (((Record)list_node_value(node->id_list,list_next(node->id_list,lnode)))->id==((Record)value)->id)  {
                    *removed = true;
		            *old_value = list_node_value(node->id_list,list_next(node->id_list,lnode));
					list_remove_next(node->id_list,lnode);
                    return node;
                }
                lnode=list_next(node->id_list,lnode);
            }
			assert(1==0);
        }
            
	}

	if (compare_res < 0)  {
        node->left_count--; //θα αφαιρεθει ενα στοιχειο απο τo αριστερο υποδεντρο (*παρατηρηρση στη περιπτωση κομβου που δεν υπαρχει)
		node->left  = node_remove(node->left,  compare, value, removed, old_value);
    }
	else  {
        node->right_count--; //θα αφαiρεθει ενα στοιχειο απο το δεξι υποδεντρο
		node->right = node_remove(node->right, compare, value, removed, old_value);
    }
	return node_repair_balance(node);	
}

static void node_destroy(SetNode node, DestroyFunc destroy_value) {
	if (node == NULL)
		return;
	
	node_destroy(node->left, destroy_value);
	node_destroy(node->right, destroy_value);

	if (destroy_value != NULL)
		destroy_value(node->id_list);

	free(node);
}

Set set_create(CompareFunc compare, DestroyFunc destroy_value) {
	assert(compare != NULL);	


	Set set = malloc(sizeof(*set));
	set->root = NULL;			
	set->size = 0;
	set->compare = compare;
	set->destroy_value = destroy_value;

	return set;
}

int set_size(Set set) {
	return set->size;
} 

void set_insert(Set set, Pointer value) {
	bool inserted;
	Pointer old_value;
	set->root = node_insert(set->root, set->compare, value, &inserted, &old_value);
	
	if (inserted)
		set->size++;
	else if (set->destroy_value != NULL)
		set->destroy_value(old_value); 
}

bool set_remove(Set set, Pointer value) {
	bool removed;
	Pointer old_value = NULL;
	set->root = node_remove(set->root, set->compare, value, &removed, &old_value);

	if (removed) {
		set->size--;

		if (set->destroy_value != NULL)
			set->destroy_value(old_value);
	}

	return removed;
}

DestroyFunc set_set_destroy_value(Set vec, DestroyFunc destroy_value) {
	DestroyFunc old = vec->destroy_value;
	vec->destroy_value = destroy_value;
	return old;
}

void destroy(Pointer list)  {
	list_destroy((List)list);
	return ;
}
void set_destroy(Set set) {
	node_destroy(set->root,destroy);
	free(set);
}

SetNode set_first(Set set) {
	return node_find_min(set->root);
}

//Διαχζιζουμε το υποδεντρο του κομβου που δινεται ως ορισμα (στη περιπτωση μας εινια παντα η ριζα)
//και ψαχνουμε τον αμεσως επομενο κομβο που εχει τιμη μεγαλυτερη απο Limit_Value 

List set_next(Set set, SetNode node,Pointer Limit_Value,SetNode *prev) {
	if (node==NULL)  {  //αν φτασουμε σε κενο κομβο ακολουθωντας τους παρακατω κανονες 
        if (*prev==NULL)  {
							 //αν δεν εχουμε βρει προηγουμενο σημαινει οτι δαοτρεχωντας τον δενντρο απο ριζα εως φυλλο
            return NULL;   //δε βρηκαμε κανενα κομβο που να θεωρηθει καταλληλη τιμη , αρα επιστρεφουμε NULL
        }
		else  {
        	return (*prev)->id_list;
		}
    }
    if (set->compare(node->value,Limit_Value)>0)  {
        *prev=node; //βρικαμε καταλληλη τιμη 
        return set_next(set,node->left,Limit_Value,prev); //κοιταμε αριστερα ψαχνωντας μια ακομη καλυτερη καταλληλη (αν υπαρχει)
    }
	else  {
		return set_next(set,node->right,Limit_Value,prev); // η τιμη αυτη δεν ειναι αποδεκτη , ψαχνουμε δεξια για να βρουμε μια μεγαλυτερη 
	}
}

List set_node_value(Set set, SetNode node) {
	return node->id_list;
}

//Ψαχνουμε τον κομβο που η τιμη του εινια ειτε ιδη με το Limit_Date ή ειναι η αμεσως μεγαλυτερη ημερομηνια 
//ιδιος τροπος με τη set_next απλα αν βρουμε ιση ημερομηνια , επιστρεφουμε αυτη
List set_find_greater_equal(Set set, SetNode node,Pointer Limit_Date,SetNode *prev) {
	if (node==NULL)  {
        if (*prev!=NULL)  {
            return (*prev)->id_list;
        }
        else  {
            return NULL;
        }
    }
    if (strcmp(((Record)node->value)->date,Limit_Date)>0)  {
        *prev=node;
        return set_find_greater_equal(set,node->left,Limit_Date,prev);
    }
	else if (strcmp(((Record)node->value)->date,Limit_Date)==0)  {
		return node->id_list;
	}
	else  {
		return set_find_greater_equal(set,node->right,Limit_Date,prev);
	}
}

bool node_is_avl(SetNode node, CompareFunc compare) {
	if (node == NULL)
		return true;


	bool res = true;
	if(node->left != NULL)
		res = res && compare(node->left->value, node->value) < 0 && compare(node_find_max(node->left)->value, node->value) < 0;
	if(node->right != NULL)
		res = res && compare(node->right->value, node->value) > 0 && compare(node_find_min(node->right)->value, node->value) > 0;


	res = res && node->height == 1 + int_max(node_height(node->left), node_height(node->right));


	int balance = node_balance(node);
	res = res && balance >= -1 && balance <= 1;


	res = res &&
		node_is_avl(node->left, compare) &&
		node_is_avl(node->right, compare);

	return res;
}

bool set_is_proper(Set node) {
	return node_is_avl(node->root, node->compare);
}

//Ψαχνει να βρει να υπαρχει ημερομηνια ιση με την Value ,αν ναι επιστρεφει τη λιστα με τις ιδιες ημερομηνιες
//αλλιως κενη λιστα

List set_equal_value(Set set,SetNode node,Pointer Value)  {
	if (node==NULL)  {
		return NULL;
	}
	if (strcmp(((Record)node->value)->date,Value)==0)  {
		return node->id_list;
	}
	else if (strcmp(((Record)node->value)->date,Value)>0)  {
		return set_equal_value(set,node->left,Value);
	}
	else  {
		return set_equal_value(set,node->right,Value);
	}  

}

//Βρισκει σε πολυπλοκοτητα O(logn) τον αριθμο των στοιχειων με μεγαλυτερη ή ιση ημερομηνια απο το Value
//αρχικοποιουμε παντα το count με το μεγεθος του δεντρου

void get_count_greater_equal(Set set,SetNode node,Pointer value,int* count)  {
    if (node==NULL)  {
		return;
	}
	if (strcmp(((Record)node->value)->date,value)==0)  {
        if (node->left!=NULL)  {
			*count=*count-node->left_count; //αν φτασουμε στην ημερομηνια μας τοτε αν εχει αριστερο υποδεντρο αφαιρουμε το πληθος των στοιχεων του 
		}
		return ;
    }
    else if (strcmp(((Record)node->value)->date,value)<0)  {
        *count=*count-node->left_count-list_size(node->id_list); //αν βρουμε στοιχειο μικρεοτερο της ημερομηνιας αφαιρουμε 
        get_count_greater_equal(set,node->right,value,count);  //το πληθος στοιχεων που εχει στο κομβο του και στο αριστερο υποδεντρο
    }                                                         //και κατευθυνομαστε δεξια 
    else  {
		//αν βρουμε μεγαλυτρεο στοιχειο δεν αλλαχουμε κατι 
        get_count_greater_equal(set,node->left,value,count); //και συνεχιζουμε αριστερα για να βρουμε μικροτερο
    }
}
SetNode set_root(Set set)  {
    return set->root;
}

