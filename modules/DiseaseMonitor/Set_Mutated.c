///////////////////////////////////////////////////////////
//
// Υλοποίηση του ADT Set μέσω AVL Tree
//
///////////////////////////////////////////////////////////

#include <stdlib.h>
#include <assert.h>
#include "ADTList.h"
#include "ADTSet.h"
#include "DiseaseMonitor.h"
#include <string.h>


// Υλοποιούμε τον ADT Set μέσω AVL, οπότε το struct set είναι ένα AVL Δέντρο.
struct set {
	SetNode root;				// η ρίζα, NULL αν είναι κενό δέντρο
	int size;					// μέγεθος, ώστε η set_size να είναι Ο(1)
	CompareFunc compare;		// η διάταξη
	DestroyFunc destroy_value;	// Συνάρτηση που καταστρέφει ένα στοιχείο του set
};

// Ενώ το struct set_node είναι κόμβος ενός AVL Δέντρου Αναζήτησης
struct set_node {
	SetNode left, right;		// Παιδιά			
    Pointer value;
    int height;				// Ύψος που βρίσκεται ο κόμβος στο δέντρο
    int left_count;
    int right_count;
    List id_list;
};


//// Συναρτήσεις που υλοποιούν επιπλέον λειτουργίες του AVL σε σχέση με ένα απλό BST /////////////////////////////////////

// Επιστρέφει τη max τιμή μεταξύ 2 ακεραίων

static int int_max(int a, int b) {
	return (a > b) ? a : b ;
}

// Επιστρέφει το ύψος που βρίσκεται ο κόμβος στο δέντρο

static int node_height(SetNode node) {
	if (!node) return 0;
	return node->height;
}

// Ενημερώνει το ύψος ενός κόμβου

static void node_update_height(SetNode node) {
	node->height = 1 + int_max(node_height(node->left), node_height(node->right));
}

// Επιστρέφει τη διαφορά ύψους μεταξύ αριστερού και δεξιού υπόδεντρου

static int node_balance(SetNode node) {
	return node_height(node->left) - node_height(node->right);
}

// Rotations : Όταν η διαφορά ύψους μεταξύ αριστερού και δεξιού υπόδεντρου είναι
// μεγαλύτερη του 1 το δέντρο δεν είναι πια AVL. Υπάρχουν 4 διαφορετικά
// rotations που εφαρμόζονται ανάλογα με την περίπτωση για να αποκατασταθεί η
// ισορροπία. Η κάθε συνάρτηση παίρνει ως όρισμα τον κόμβο που πρέπει να γίνει
// rotate, και επιστρέφει τη ρίζα του νέου υποδέντρου.

// Single left rotation

static SetNode node_rotate_left(SetNode node) {
	SetNode right_node = node->right;
	SetNode left_subtree = right_node->left;
    
    node->right_count=right_node->left_count;
	right_node->left_count=node->left_count+list_size(node->id_list);
	right_node->left = node;
	node->right = left_subtree;

	node_update_height(node);
	node_update_height(right_node);
	
	return right_node;
}

// Single right rotation

static SetNode node_rotate_right(SetNode node) {
	SetNode left_node = node->left;
	SetNode left_right = left_node->right;

    node->left_count=left_node->right_count;
	left_node->right_count=node->right_count+list_size(node->id_list);
	left_node->right = node;
	node->left = left_right;

	node_update_height(node);
	node_update_height(left_node);
	
	return left_node;
}

// Double left-right rotation

static SetNode node_rotate_left_right(SetNode node) {
	node->left = node_rotate_left(node->left);
	return node_rotate_right(node);
}

// Double right-left rotation

static SetNode node_rotate_right_left(SetNode node) {
	node->right = node_rotate_right(node->right);
	return node_rotate_left(node);
}

// Επισκευή του AVL property αν δεν ισχύει

static SetNode node_repair_balance(SetNode node) {
	node_update_height(node);

	int balance = node_balance(node);
	if (balance > 1) {
		// το αριστερό υπόδεντρο είναι unbalanced
		if (node_balance(node->left) >= 0) 
			return node_rotate_right(node);
		else 
			return node_rotate_left_right(node);

	} else if (balance < -1) {
		// το δεξί υπόδεντρο είναι unbalanced
		if (node_balance(node->right) <= 0)
			return node_rotate_left(node);
		else
			return node_rotate_right_left(node);
	}

	// δεν χρειάστηκε να πραγματοποιηθεί rotation
	return node;
}


//// Συναρτήσεις που είναι (σχεδόν) _ολόιδιες_ με τις αντίστοιχες της BST υλοποίησης ////////////////
//
// Είναι σημαντικό να κατανοήσουμε πρώτα τον κώδικα του BST πριν από αυτόν του AVL.
// Θα μπορούσαμε οργανώνοντας τον κώδικα διαφορετικά να επαναχρησιμοποιήσουμε τις συναρτήσεις αυτές.
//
// Οι διαφορές είναι σημειωμένες με "AVL" σε σχόλιο

// Δημιουργεί και επιστρέφει έναν κόμβο με τιμή value (χωρίς παιδιά)
//
static SetNode node_create(Pointer value) {
	SetNode node = malloc(sizeof(*node));
	node->left = NULL;
	node->right = NULL;
    node->value=value;
    node->id_list=list_create(NULL);
	list_insert_next(node->id_list,LIST_BOF,value);
	node->height = 1;		// AVL
	node->left_count=0;
    node->right_count=0;
    return node;
}

// Επιστρέφει τον κόμβο με τιμή ίση με value στο υποδέντρο με ρίζα node, διαφορετικά NULL


// Επιστρέφει τον μικρότερο κόμβο του υποδέντρου με ρίζα node

static SetNode node_find_min(SetNode node) {
	return node != NULL && node->left != NULL
		? node_find_min(node->left)				// Υπάρχει αριστερό υποδέντρο, η μικρότερη τιμή βρίσκεται εκεί
		: node;									// Αλλιώς η μικρότερη τιμή είναι στο ίδιο το node
}

// Επιστρέφει τον μεγαλύτερο κόμβο του υποδέντρου με ρίζα node

static SetNode node_find_max(SetNode node) {
	return node != NULL && node->right != NULL
		? node_find_max(node->right)			// Υπάρχει δεξί υποδέντρο, η μεγαλύτερη τιμή βρίσκεται εκεί
		: node;									// Αλλιώς η μεγαλύτερη τιμή είναι στο ίδιο το node
}

// Επιστρέφει τον προηγούμενο (στη σειρά διάταξης) του κόμβου target στο υποδέντρο με ρίζα node,
// ή NULL αν ο target είναι ο μικρότερος του υποδέντρου. Το υποδέντρο πρέπει να περιέχει τον κόμβο
// target, οπότε δεν μπορεί να είναι κενό.



// Επιστρέφει τον επόμενο (στη σειρά διάταξης) του κόμβου target στο υποδέντρο με ρίζα node,
// ή NULL αν ο target είναι ο μεγαλύτερος του υποδέντρου. Το υποδέντρο πρέπει να περιέχει τον κόμβο
// target, οπότε δεν μπορεί να είναι κενό.



// Αν υπάρχει κόμβος με τιμή ισοδύναμη της value, αλλάζει την τιμή του σε value, διαφορετικά προσθέτει
// νέο κόμβο με τιμή value. Επιστρέφει τη νέα ρίζα του υποδέντρου, και θέτει το *inserted σε true
// αν έγινε προσθήκη, ή false αν έγινε ενημέρωση.

static SetNode node_insert(SetNode node, CompareFunc compare, Pointer value, bool* inserted, Pointer* old_value) {
	// Αν το υποδέντρο είναι κενό, δημιουργούμε νέο κόμβο ο οποίος γίνεται ρίζα του υποδέντρου
	if (node == NULL) {
		*inserted = true;			// κάναμε προσθήκη
        
		return node_create(value);
	}

	// Το πού θα γίνει η προσθήκη εξαρτάται από τη διάταξη της τιμής
	// value σε σχέση με την τιμή του τρέχοντος κόμβου (node->value)
	//
	int compare_res = compare(value, node->value);
	if (compare_res == 0) {
		// βρήκαμε ισοδύναμη τιμή, κάνουμε update
		list_insert_next(node->id_list,LIST_BOF,value);
        *inserted = true;
        return node;

	} else if (compare_res < 0) {
		// value < node->value, συνεχίζουμε αριστερά.
        node->left_count++;
		node->left = node_insert(node->left, compare, value, inserted, old_value);
	} else {
		// value > node->value, συνεχίζουμε δεξιά
        node->right_count++;
		node->right = node_insert(node->right, compare, value, inserted, old_value);

    }

	return node_repair_balance(node);	// AVL
}

// Αφαιρεί και αποθηκεύει στο min_node τον μικρότερο κόμβο του υποδέντρου με ρίζα node.
// Επιστρέφει τη νέα ρίζα του υποδέντρου.

static SetNode node_remove_min(SetNode node, SetNode* min_node) {
	if (node->left == NULL) {
		// Δεν έχουμε αριστερό υποδέντρο, οπότε ο μικρότερος είναι ο ίδιος ο node
		*min_node = node;
		return node->right;		// νέα ρίζα είναι το δεξιό παιδί

	} else {
		// Εχουμε αριστερό υποδέντρο, οπότε η μικρότερη τιμή είναι εκεί. Συνεχίζουμε αναδρομικά
		// και ενημερώνουμε το node->left με τη νέα ρίζα του υποδέντρου.
		node->left = node_remove_min(node->left, min_node);

		return node_repair_balance(node);	// AVL
	}
}

// Διαγράφει το κόμβο με τιμή ισοδύναμη της value, αν υπάρχει. Επιστρέφει τη νέα ρίζα του
// υποδέντρου, και θέτει το *removed σε true αν έγινε πραγματικά διαγραφή.

static SetNode node_remove(SetNode node, CompareFunc compare, Pointer value, bool* removed, Pointer* old_value) {
	if (node == NULL) {
		*removed = false;		// κενό υποδέντρο, δεν υπάρχει η τιμή
		return NULL;
	}

	int compare_res = compare(value, node->value);
	if (compare_res == 0) {
		// Βρέθηκε ισοδύναμη τιμή στον node, οπότε τον διαγράφουμε. Το πώς θα γίνει αυτό εξαρτάται από το αν έχει παιδιά.
		*removed = true;
		*old_value = node->value;
        if (list_size(node->id_list)==1)   {
		    if (node->left == NULL) {
		    	// Δεν υπάρχει αριστερό υποδέντρο, οπότε διαγράφεται απλά ο κόμβος και νέα ρίζα μπαίνει το δεξί παιδί
		    	SetNode right = node->right;	// αποθήκευση πριν το free!
                list_remove_next(node->id_list,LIST_BOF);
                list_destroy(node->id_list);
		    	free(node);
		    	return right;

		    } else if (node->right == NULL) {
		    	// Δεν υπάρχει δεξί υποδέντρο, οπότε διαγράφεται απλά ο κόμβος και νέα ρίζα μπαίνει το αριστερό παιδί
		    	SetNode left = node->left;		// αποθήκευση πριν το free!
                list_remove_next(node->id_list,LIST_BOF);
                list_destroy(node->id_list);
		    	free(node);
		    	return left;

		    } else {
		    	// Υπάρχουν και τα δύο παιδιά. Αντικαθιστούμε την τιμή του node με την μικρότερη του δεξιού υποδέντρου, η οποία
		    	// αφαιρείται. Η συνάρτηση node_remove_min κάνει ακριβώς αυτή τη δουλειά.

		    	SetNode min_right;
		    	node->right = node_remove_min(node->right, &min_right);

		    	// Σύνδεση του min_right στη θέση του node
		    	min_right->left = node->left;
		    	min_right->right = node->right;
                list_remove_next(node->id_list,LIST_BOF);
                list_destroy(node->id_list);
		    	free(node);

		    	return node_repair_balance(min_right);	// AVL
		    }
        }
        else  {
            ListNode lnode=list_first(node->id_list);
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
        }
            
	}

	// compare_res != 0, συνεχίζουμε στο αριστερό ή δεξί υποδέντρο, η ρίζα δεν αλλάζει.
	if (compare_res < 0)  {
        node->left_count--;
		node->left  = node_remove(node->left,  compare, value, removed, old_value);
    }
	else  {
        node->right_count--;
		node->right = node_remove(node->right, compare, value, removed, old_value);
    }
	return node_repair_balance(node);	// AVL
}

// Καταστρέφει όλο το υποδέντρο με ρίζα node

static void node_destroy(SetNode node, DestroyFunc destroy_value) {
	if (node == NULL)
		return;
	
	// πρώτα destroy τα παιδιά, μετά free το node
	node_destroy(node->left, destroy_value);
	node_destroy(node->right, destroy_value);

	if (destroy_value != NULL)
		destroy_value(node->value);

	free(node);
}


//// Συναρτήσεις του ADT Set. Γενικά πολύ απλές, αφού καλούν τις αντίστοιχες node_* //////////////////////////////////
//
// Επίσης ολόιδιες με αυτές του BST-based Set

Set set_create(CompareFunc compare, DestroyFunc destroy_value) {
	assert(compare != NULL);	// LCOV_EXCL_LINE

	// δημιουργούμε το stuct
	Set set = malloc(sizeof(*set));
	set->root = NULL;			// κενό δέντρο
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
	
	// Το size αλλάζει μόνο αν μπει νέος κόμβος. Στα updates κάνουμε destroy την παλιά τιμή
	if (inserted)
		set->size++;
	else if (set->destroy_value != NULL)
		set->destroy_value(old_value); 
}

bool set_remove(Set set, Pointer value) {
	bool removed;
	Pointer old_value = NULL;
	set->root = node_remove(set->root, set->compare, value, &removed, &old_value);

	// Το size αλλάζει μόνο αν πραγματικά αφαιρεθεί ένας κόμβος
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

void set_destroy(Set set) {
	node_destroy(set->root, set->destroy_value);
	free(set);
}

SetNode set_first(Set set) {
	return node_find_min(set->root);
}


List set_next(Set set, SetNode node,Pointer Limit_Value,SetNode *prev) {
	if (node==NULL)  {
        if (*prev!=NULL)  {
            if (set->compare((*prev)->value,Limit_Value)==0)  {
                List list=list_create(NULL);
            	return list;
            }
        }
        else  {
            List list=list_create(NULL);
            return list;
        }
        return (*prev)->id_list;
    }
    if (set->compare(node->value,Limit_Value)>0)  {
        *prev=node;
        return set_next(set,node->left,Limit_Value,prev);
    }
	else  {
		return set_next(set,node->right,Limit_Value,prev);
	}
}

Pointer set_node_value(Set set, SetNode node) {
	return node->id_list;
}

List set_find_greater_equal(Set set, SetNode node,Pointer Limit_Date,SetNode *prev) {
	if (node==NULL)  {
        if (*prev!=NULL)  {
            return (*prev)->id_list;
        }
        else  {
			List list=list_create(NULL);
            return list;
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



// Συναρτήσεις που δεν υπάρχουν στο public interface αλλά χρησιμοποιούνται στα tests
// Ελέγχουν ότι το δέντρο είναι ένα σωστό AVL.

// LCOV_EXCL_START (δε μας ενδιαφέρει το coverage των test εντολών, και επιπλέον μόνο τα true branches εξετάζονται σε ένα επιτυχημένο test)

bool node_is_avl(SetNode node, CompareFunc compare) {
	if (node == NULL)
		return true;

	// Ελέγχουμε την ιδιότητα:
	// κάθε κόμβος είναι > αριστερό παιδί, > δεξιότερο κόμβο του αριστερού υποδέντρου, < δεξί παιδί, < αριστερότερο κόμβο του δεξιού υποδέντρου.
	// Είναι ισοδύναμη με την BST ιδιότητα (κάθε κόμβος είναι > αριστερό υποδέντρο και < δεξί υποδέντρο) αλλά ευκολότερο να ελεγθεί.
	bool res = true;
	if(node->left != NULL)
		res = res && compare(node->left->value, node->value) < 0 && compare(node_find_max(node->left)->value, node->value) < 0;
	if(node->right != NULL)
		res = res && compare(node->right->value, node->value) > 0 && compare(node_find_min(node->right)->value, node->value) > 0;

	// Το ύψος είναι σωστό
	res = res && node->height == 1 + int_max(node_height(node->left), node_height(node->right));

	// Ο κόμβος έχει την AVL ιδιότητα
	int balance = node_balance(node);
	res = res && balance >= -1 && balance <= 1;

	// Τα υποδέντρα είναι σωστά
	res = res &&
		node_is_avl(node->left, compare) &&
		node_is_avl(node->right, compare);

	return res;
}

bool set_is_proper(Set node) {
	return node_is_avl(node->root, node->compare);
}
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
void get_count_greater_equal(Set set,SetNode node,Pointer value,int* count)  {
    if (node==NULL)  {
		return;
	}
	if (strcmp(((Record)node->value)->date,value)==0)  {
        if (node->left!=NULL)  {
			*count=*count-node->left_count;
		}
		return ;
    }
    else if (strcmp(((Record)node->value)->date,value)<0)  {
        *count=*count-node->left_count-list_size(node->id_list);
        get_count_greater_equal(set,node->right,value,count);
    }
    else  {

        get_count_greater_equal(set,node->left,value,count);
    }
}
SetNode set_root(Set set)  {
    return set->root;
}

// LCOV_EXCL_STOP