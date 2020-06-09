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
    p=malloc(sizeof(int));
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
		TEST_ASSERT(graph_size(graph)==i+1);
	}
	List list=graph_get_vertices(graph);
	TEST_ASSERT(list_size(list)==N);
	ListNode lnode=list_first(list);
	while (lnode!=LIST_EOF)  {
		TEST_ASSERT(*(int*)list_node_value(list,lnode)<N && *(int*)list_node_value(list,lnode)>=0);
		lnode=list_next(list,lnode);
	}
	lnode=list_first(list);
	int count=1;
	while (lnode!=LIST_EOF)  {
		Pointer vertex1=list_node_value(list,lnode);
		Pointer vertex2=list_node_value(list,list_next(list,lnode));
		graph_insert_edge(graph,vertex1,vertex2,count);
		count++;
		lnode=list_next(list,list_next(list,lnode));
	}
	lnode=list_first(list);
	count=1;
	while (lnode!=LIST_EOF)  {
		Pointer vertex1=list_node_value(list,lnode);
		Pointer vertex2=list_node_value(list,list_next(list,lnode));
		TEST_ASSERT(graph_get_weight(graph,vertex2,vertex1)==graph_get_weight(graph,vertex1,vertex2) && graph_get_weight(graph,vertex2,vertex1)==count);
		List adj=graph_get_adjacent(graph,vertex1);
		TEST_ASSERT(*(int*)list_node_value(list,list_first(adj))==*(int*)vertex2);
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
		graph_insert_vertex(graph,create_integer(i));
		TEST_ASSERT(graph_size(graph)==i+1);
	}
	List list=graph_get_vertices(graph);
	ListNode lnode=list_first(list);
	for(int i=0;i<N;i++)  {
		graph_remove_vertex(graph,list_node_value(list,lnode));
		TEST_ASSERT(graph_size(graph)==N-i-1);
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
		graph_insert_edge(graph,vertex1,vertex2,count);
		count++;
		lnode=list_next(list,list_next(list,lnode));
	}
	lnode=list_first(list);
	while (lnode!=LIST_EOF)  {
		Pointer vertex1=list_node_value(list,lnode);
		Pointer vertex2=list_node_value(list,list_next(list,lnode));
		graph_remove_edge(graph,vertex1,vertex2);		
		List adj=graph_get_adjacent(graph,list_node_value(list,lnode));
		TEST_ASSERT(list_size(adj)==0);
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
		Pointer vertex1=list_node_value(list,lnode);
		Pointer vertex2=list_node_value(list,list_next(list,lnode));
		graph_insert_edge(graph,vertex1,vertex2,count);
		count++;
		lnode=list_next(list,list_next(list,lnode));
	}
	lnode=list_first(list);
	while (lnode!=LIST_EOF)  {
		Pointer vertex2=list_node_value(list,list_next(list,lnode));
		list_remove_next(list,lnode);
		graph_remove_vertex(graph,vertex2);		
		List adj=graph_get_adjacent(graph,list_node_value(list,lnode));
		TEST_ASSERT(list_size(adj)==0);
		lnode=list_next(list,lnode);
		list_destroy(adj);
	}

	graph_destroy(graph);
	list_destroy(list);
} 

void test_shortest_path(void)  {

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