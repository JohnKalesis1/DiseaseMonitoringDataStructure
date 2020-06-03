#include "common_types.h"
#include "ADTGraph.h"
#include <stdlib.h>
#include <limits.h>
#define LIMIT 5000

struct graph {
    Map map;
    int size;
    CompareFunc compare;
    DestroyFunc destroy;
};
struct weighted_vertex  {
    Pointer vertex;
    uint weight;
};
typedef struct weighted_vertex * W_Vertex;

W_Vertex create_edge(Pointer vertex,uint weight)  {
    W_Vertex weighted_vertex=malloc(sizeof(weighted_vertex));
    weighted_vertex->vertex=vertex;
    weighted_vertex->weight=weight;
    return weighted_vertex;
}

Graph graph_create(CompareFunc compare, DestroyFunc destroy_vertex)  {
    Graph graph=malloc(sizeof(*graph));
    graph->map=map_create(compare,destroy_vertex,list_destroy);
    graph->size=0;
    graph->compare=compare;
    graph->destroy=destroy_vertex;

    return graph;
}

int graph_size(Graph graph)  {
    return graph->size;
}

void graph_insert_vertex(Graph graph, Pointer vertex)  {
    List list=list_create(NULL);
    map_insert(graph->map,vertex,list);
}

List graph_get_vertices(Graph graph)  {
    List list=list_create(NULL);
    MapNode node=map_first(graph->map);
    if (graph->size!=0)  {
        while (node!=MAP_EOF)  {
            list_insert_next(list,LIST_BOF,map_node_key(graph->map,node));
            node=map_next(graph->map,node);
        }
    }
    return list;
}

void graph_remove_vertex(Graph graph, Pointer vertex)  {
    map_remove(graph->map,vertex);
    return ;
}

void graph_insert_edge(Graph graph, Pointer vertex1, Pointer vertex2, uint weight)  {
    MapNode m_node1=map_find_node(graph->map,vertex1);
    MapNode m_node2=map_find_node(graph->map,vertex2);
    List list;
    list=(List)map_node_value(graph->map,m_node1);
    list_insert_next(list,LIST_BOF,create_edge(vertex2,weight));
    map_insert(graph->map,vertex1,list);
    list=(List)map_node_value(graph->map,m_node2);
    list_insert_next(list,LIST_BOF,create_edge(vertex1,weight));
    map_insert(graph->map,vertex2,list);
    return ;
}

void graph_remove_edge(Graph graph, Pointer vertex1, Pointer vertex2)  {
    MapNode m_node1=map_find_node(graph->map,vertex1);
    MapNode m_node2=map_find_node(graph->map,vertex2);
    List list;
    ListNode lnode;
    list=(List)map_node_value(graph->map,m_node1);
    lnode=list_first(list);
    while(graph->compare(list_node_value(list,list_next(list,lnode)),vertex2)!=0)  {
        lnode=list_next(list,lnode);
    }
    list_remove_next(list,lnode);
    map_insert(graph->map,vertex1,list);
    list=(List)map_node_value(graph->map,m_node2);
    lnode=list_first(list);
    while(graph->compare(list_node_value(list,list_next(list,lnode)),vertex1)!=0)  {
        lnode=list_next(list,lnode);
    }
    list_remove_next(list,lnode);
    map_insert(graph->map,vertex2,list);
    return ;
}

uint graph_get_weight(Graph graph, Pointer vertex1, Pointer vertex2)  {
    MapNode m_node=map_find_node(graph->map,vertex1);
    List list=(List)map_node_value(graph->map,m_node);
    ListNode lnode=list_first(list);
    if (lnode!=LIST_BOF)  {
        while (graph->compare(((W_Vertex)list_node_value(list,lnode))->vertex,vertex2)!=0)  {
            lnode=list_next(list,lnode);
        }
        if (lnode!=LIST_EOF)  {
            return ((W_Vertex)list_node_value(list,lnode))->weight;
        }
        else  {
            return UINT_MAX;
        }
    }
    return UINT_MAX;
}


List graph_get_adjacent(Graph graph, Pointer vertex)  {
    List list,r_list=list_create(NULL);
    MapNode m_node=map_find_node(graph->map,vertex);
    list=(List)map_node_value(graph->map,m_node);
    ListNode lnode=list_first(list);
    while (lnode!=LIST_EOF)  {
        list_insert_next(r_list,LIST_BOF,list_node_value(list,lnode));
        lnode=list_next(list,lnode);
    }
    return r_list;
}

List graph_shortest_path(Graph graph, Pointer source, Pointer target)  {
    
}

void graph_destroy(Graph graph)  {
    map_destroy(graph->map);
    free(graph);
}

void graph_set_hash_function(Graph graph, HashFunc hash_func)  {
    map_set_hash_function(graph->map,hash_func);
}