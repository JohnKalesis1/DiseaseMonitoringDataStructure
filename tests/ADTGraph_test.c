//////////////////////////////////////////////////////////////////
//
// Unit tests για τον ADT Graph.
// Οποιαδήποτε υλοποίηση οφείλει να περνάει όλα τα tests.
//
//////////////////////////////////////////////////////////////////

#include "acutest.h"			// Απλή βιβλιοθήκη για unit testing
#include "ADTList.h"
#include "ADTGraph.h"

int* create_integer(int value)  {
    int* p;
    p=malloc(sizeof(*p));
    *p=value;
    return p;
}
int compare_ints(Pointer a,Pointer b)  {
	return *(int*)a-*(int*)b;
}
// Προς υλοποίηση
void test_create(void)  {
	Graph graph=graph_create(compare_ints,free);
	List list=graph_get_vertices(graph);
	TEST_ASSERT(graph!=NULL);
	TEST_ASSERT(graph_size(graph)==0);
	TEST_ASSERT(list_size(list)==0);
	graph_destroy(graph);
	list_destroy(list);
} 

void test_insert(void)  {
	int N=1000;
	Graph graph=graph_create(compare_ints,free);
	graph_set_hash_function(graph,hash_int);
	for(int i=0;i<N;i++)  {
		graph_insert_vertex(graph,create_integer(i));
		TEST_ASSERT(graph_size(graph)==i+1); //προστηκη ενος στοιχειου καθε φορα , ελεγχος οτι αυξανεται το μεγεθος οτυ γραφου
	}
	List list=graph_get_vertices(graph);
	TEST_ASSERT(list_size(list)==N); //κοιταμε οτι η get_vertices επιστρεφει σωστο αριθμο απο στοιχεια
	ListNode lnode=list_first(list);
	while (lnode!=LIST_EOF)  {// ελεγχουμε οτι στο γραφο δε νυπαρχει στοιχειο που δε θα επρεπε
		TEST_ASSERT(*(int*)list_node_value(list,lnode)<N && *(int*)list_node_value(list,lnode)>=0); 
		lnode=list_next(list,lnode);
	}
	lnode=list_first(list);
	int count=1;
	while (lnode!=LIST_EOF)  {
		Pointer vertex1=list_node_value(list,lnode);
		Pointer vertex2=list_node_value(list,list_next(list,lnode));
		graph_insert_edge(graph,vertex1,vertex2,count); //προσθετουμε μια ακμη αναμεσα στο κ<->κ+1 στοιχειο της λιστας οπου κ=0,2,4,...,998
		count++;
		lnode=list_next(list,list_next(list,lnode));
	}
	lnode=list_first(list);
	count=1;
	while (lnode!=LIST_EOF)  {
		Pointer vertex1=list_node_value(list,lnode);
		Pointer vertex2=list_node_value(list,list_next(list,lnode));// ελεγχουμε οτι εχεο προστεθει ακμη με σωστο βαρος για καθε στοιχειο 
		TEST_ASSERT(graph_get_weight(graph,vertex2,vertex1)==graph_get_weight(graph,vertex1,vertex2) && graph_get_weight(graph,vertex2,vertex1)==count);
		List adj=graph_get_adjacent(graph,vertex1); //παιρνουμε τους γειτονες οτυ καθε vertex 
		TEST_ASSERT(*(int*)list_node_value(list,list_first(adj))==*(int*)vertex2); //ελεγχουμε οτι ειναι αυτος που θα πρεπε
		TEST_ASSERT(list_size(adj)==1);
		count++;
		lnode=list_next(list,list_next(list,lnode));
		list_destroy(adj);
	}
	graph_destroy(graph);
	list_destroy(list);

} 

void test_remove(void)  {
	int N=1000;												//insert and delete ολα τα στοιχεια
	Graph graph=graph_create(compare_ints,free);
	graph_set_hash_function(graph,hash_int);
	for(int i=0;i<N;i++)  {
		graph_insert_vertex(graph,create_integer(i)); //πρεωτα βαζουμε ολα τα στοιχεια
		TEST_ASSERT(graph_size(graph)==i+1);
	}
	List list=graph_get_vertices(graph);
	ListNode lnode=list_first(list);
	for(int i=0;i<N;i++)  {
		graph_remove_vertex(graph,list_node_value(list,lnode)); //μετα τα αφαιρουμε ολα τα στοιχεια 
		TEST_ASSERT(graph_size(graph)==N-i-1); //ελεγχος οτι μειωνεται το μεγεθος
		lnode=list_next(list,lnode);
	}
	list_destroy(list);										//δοκιμη delete_edge  
	for(int i=0;i<N;i++)  {
		graph_insert_vertex(graph,create_integer(i));
		TEST_ASSERT(graph_size(graph)==i+1);
	}
	list=graph_get_vertices(graph);
	lnode=list_first(list);
	int count=1;
	while (lnode!=LIST_EOF)  {
		Pointer vertex1=list_node_value(list,lnode);
		Pointer vertex2=list_node_value(list,list_next(list,lnode));
		graph_insert_edge(graph,vertex1,vertex2,count); //οπως και στο πανω συνδεουμε ανα δυο υα στοιχεια της λιστας
		count++;
		lnode=list_next(list,list_next(list,lnode));
	}
	lnode=list_first(list);
	while (lnode!=LIST_EOF)  {
		Pointer vertex1=list_node_value(list,lnode);
		Pointer vertex2=list_node_value(list,list_next(list,lnode));
		graph_remove_edge(graph,vertex1,vertex2); //διαγραφουμε την ακμη που εχουμε προσθεσει μεταξυ των στοιχεων		
		List adj=graph_get_adjacent(graph,list_node_value(list,lnode));
		TEST_ASSERT(list_size(adj)==0);  //ελεγχουμε οτι εχει αφαιρεθει η ακμη που υπηρχε(το οτι υπηρχε το ξερουμε το test της insert)
		lnode=list_next(list,list_next(list,lnode));
		list_destroy(adj);
	}
	graph_destroy(graph);
	list_destroy(list);

	graph=graph_create(compare_ints,free);					// δοκιμη remove_vertex η οποια πρεπει να διαγραφει και τις ακμες
	graph_set_hash_function(graph,hash_int);
	for(int i=0;i<N;i++)  {
		graph_insert_vertex(graph,create_integer(i));
		TEST_ASSERT(graph_size(graph)==i+1);
	}
	list=graph_get_vertices(graph);
	lnode=list_first(list);
	count=1;
	while (lnode!=LIST_EOF)  {
		Pointer vertex1=list_node_value(list,lnode); //ομοιως ανα δυο στοιχεια βαζουμε μια ακμη
		Pointer vertex2=list_node_value(list,list_next(list,lnode));
		graph_insert_edge(graph,vertex1,vertex2,count);
		count++;
		lnode=list_next(list,list_next(list,lnode));
	}
	lnode=list_first(list);
	while (lnode!=LIST_EOF)  {
		Pointer vertex2=list_node_value(list,list_next(list,lnode));
		list_remove_next(list,lnode);
		graph_remove_vertex(graph,vertex2); //αφαρουμε το vertex απο το γραφο
		List adj=graph_get_adjacent(graph,list_node_value(list,lnode));
		TEST_ASSERT(list_size(adj)==0); //ελεγχουμε οτι η λιστα γειτνιασης του πρωην γειτονα του στοιχειου που διαγραψαμε εχει ενημερωθει
		lnode=list_next(list,lnode);
		list_destroy(adj);
	}

	graph_destroy(graph);
	list_destroy(list);
} 

void test_shortest_path(void)  {
	int N=10;											
	Graph graph=graph_create(compare_ints,free);
	graph_set_hash_function(graph,hash_int);
	for(int i=0;i<N;i++)  {
		graph_insert_vertex(graph,create_integer(i));
	}
	List path;
	List list=graph_get_vertices(graph);
	ListNode lnode=list_first(list);
	
	path=graph_shortest_path(graph,list_node_value(list,lnode),list_node_value(list,list_next(list,lnode)));
	TEST_ASSERT(list_size(path)==0); //ελεγχουμε οτι ανα δεν υπαρχει κανενα μονοπατι , επιστρεφεται κενη λιστα 
	list_destroy(path);

	graph_insert_edge(graph,list_node_value(list,lnode),list_node_value(list,list_next(list,lnode)),8);
	path=graph_shortest_path(graph,list_node_value(list,lnode),list_node_value(list,list_next(list,lnode)));
	TEST_ASSERT(list_size(path)==2); //ελεγχος για την απλουστερη και πιο συντομη διαδρομη
	ListNode path_node=list_first(path);
	TEST_ASSERT(graph_get_weight(graph,list_node_value(path,path_node),list_node_value(path,list_next(path,path_node)))==8);
	list_destroy(list);
	list_destroy(path);
	graph_destroy(graph);


	graph=graph_create(compare_ints,free);  //οι γραφοι που δημιουργουνται παρακατω υπαρχουν σε φωτογραφιες 
	graph_set_hash_function(graph,hash_int);
	for(int i=0;i<N;i++)  {
		graph_insert_vertex(graph,create_integer(i));
	}
	list=graph_get_vertices(graph); //Γραφος 1 (ελεγχουμε οτι δουλευει σωστα σε ελεγχο πολλων μονοπατιων)
	ListNode nd1=list_first(list),nd2=list_next(list,nd1),nd3=list_next(list,nd2),nd4=list_next(list,nd3),nd5=list_next(list,nd4),nd6=list_next(list,nd5),nd7=list_next(list,nd6);
	graph_insert_edge(graph,list_node_value(list,nd1),list_node_value(list,nd2),2);
	graph_insert_edge(graph,list_node_value(list,nd2),list_node_value(list,nd4),3);
	graph_insert_edge(graph,list_node_value(list,nd7),list_node_value(list,nd4),10);
	graph_insert_edge(graph,list_node_value(list,nd4),list_node_value(list,nd3),2);
	graph_insert_edge(graph,list_node_value(list,nd3),list_node_value(list,nd5),5);
	graph_insert_edge(graph,list_node_value(list,nd5),list_node_value(list,nd6),12);
	graph_insert_edge(graph,list_node_value(list,nd6),list_node_value(list,nd7),6);
	graph_insert_edge(graph,list_node_value(list,nd5),list_node_value(list,nd7),1);
	graph_insert_edge(graph,list_node_value(list,nd1),list_node_value(list,nd4),4);
	path=graph_shortest_path(graph,list_node_value(list,nd1),list_node_value(list,nd4));
	TEST_ASSERT(list_size(path)==2);
	list_destroy(path);
	path=graph_shortest_path(graph,list_node_value(list,nd4),list_node_value(list,nd7));
	TEST_ASSERT(list_size(path)==4);
	list_destroy(path);
	path=graph_shortest_path(graph,list_node_value(list,nd1),list_node_value(list,nd5));
	TEST_ASSERT(list_size(path)==4);
	list_destroy(path);
	path=graph_shortest_path(graph,list_node_value(list,nd4),list_node_value(list,nd6));
	TEST_ASSERT(list_size(path)==5);
	list_destroy(path);
	list_destroy(list);
	graph_destroy(graph);
	


	graph=graph_create(compare_ints,free);
	graph_set_hash_function(graph,hash_int);
	for(int i=0;i<N;i++)  {
		graph_insert_vertex(graph,create_integer(i));
	}
	list=graph_get_vertices(graph);//Γραφος 2 (ειδικη περιπτωση για να δουμε αν δουλευει η ανανεωση στην ουρα προτεραιοτητας οταν βρισκεται καλυτερο μονοπατι)
	nd1=list_first(list),nd2=list_next(list,nd1),nd3=list_next(list,nd2),nd4=list_next(list,nd3);
	graph_insert_edge(graph,list_node_value(list,nd1),list_node_value(list,nd2),2);
	graph_insert_edge(graph,list_node_value(list,nd1),list_node_value(list,nd4),1);
	graph_insert_edge(graph,list_node_value(list,nd4),list_node_value(list,nd3),7);
	graph_insert_edge(graph,list_node_value(list,nd3),list_node_value(list,nd2),5);
	path=graph_shortest_path(graph,list_node_value(list,nd1),list_node_value(list,nd3));
	TEST_ASSERT(list_size(path)==3); 
	path_node=list_first(path);//δειτε εξηγηση στη φωτογραφια (παρακατω ελεγχω οτι το αθροισμα των βαρων ολης της διαδρομης ειναι 7 και οχι 8)
	TEST_ASSERT(graph_get_weight(graph,list_node_value(path,path_node),list_node_value(path,list_next(path,path_node)))+graph_get_weight(graph,list_node_value(path,list_next(list,list_next(list,path_node))),list_node_value(path,list_next(path,path_node)))==7);

	graph_destroy(graph);
	list_destroy(path);
	list_destroy(list);
} 



// Λίστα με όλα τα tests προς εκτέλεση
TEST_LIST = {
	// { "create", test_create },

	{ "graph_create", test_create },
	{ "graph_insert", test_insert },
	{ "graph_remove", test_remove },
	{ "graph_shortest_path",test_shortest_path},

	{ NULL, NULL } // τερματίζουμε τη λίστα με NULL
}; 