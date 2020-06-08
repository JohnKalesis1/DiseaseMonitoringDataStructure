#include "ADTGraph.h"
#include "ADTPriorityQueue.h"
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
struct distance_vertex  {
    Pointer vertex;
    uint distance;
};
typedef struct distance_vertex * D_Vertex;
typedef struct weighted_vertex * W_Vertex;

W_Vertex create_edge(Pointer vertex,uint weight)  {
    W_Vertex weighted_vertex=malloc(sizeof(weighted_vertex));
    weighted_vertex->vertex=vertex;
    weighted_vertex->weight=weight;
    return weighted_vertex;
}

D_Vertex create_distance(Pointer vertex,uint distance)  {
    D_Vertex distance_vertex=malloc(sizeof(distance_vertex));
    distance_vertex->vertex=vertex;
    distance_vertex->distance=distance;
    return distance_vertex;
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

bool* create_bool(bool value)  {
    bool* p;
    p=malloc(sizeof(bool));
    *p=value;
    return p;
}
int* create_int(int value)  {
    int* p;
    p=malloc(sizeof(int));
    *p=value;
    return p;
}

int compare_distances(Pointer a,Pointer b)  {
    D_Vertex vertex1= (D_Vertex) a;
    D_Vertex vertex2= (D_Vertex) b;
    return (-1)*(vertex1->distance-vertex2->distance);
}
List graph_shortest_path(Graph graph, Pointer source, Pointer target)  {
    Map spider_web=map_create(graph->compare,NULL,free);
    PriorityQueue distance_array=pqueue_create(compare_distances,free,NULL);
    Map previous_shortest=map_create(graph->compare,NULL,NULL);
    MapNode mnode=map_first(graph->map);
    for (int i=0;i<graph->size;i++)  {
        map_insert(spider_web,map_node_value(graph->map,mnode),create_bool(0));
        map_insert(distance_array,map_node_value(graph->map,mnode),create_int(INT_MAX));
        mnode=map_next(graph->map,mnode);
    }
    map_insert(previous_shortest,source,NULL);
    List list=map_node_value(graph->map,map_find_node(graph->map,source));
    ListNode lnode=list_first(list);
    while (lnode!=LIST_EOF)  {
        pqueue_insert(distance_array,create_distance(((W_Vertex)list_node_value(list,lnode))->vertex,((W_Vertex)list_node_value(list,lnode))->weight));
        lnode=list_next(list,lnode);
    }
    map_insert(spider_web,source,create_bool(1));
    pqueue_insert(distance_array,create_distance(source,0));
    while(pqueue_size(distance_array)!=0)  {
        D_Vertex d_vertex=pqueue_max(distance_array);
        pqueue_remove_max(distance_array);
        if (map_node_value(spider_web,d_vertex->vertex)==1)  {
            continue ;
        }
        if (graph->compare(d_vertex,target)==0)  {
            if (map_node_value(previous_shortest,map_find(previous_shortest,target))!=NULL)  {
                List r_list=list_create(NULL);
                Pointer vertex=target;
                while (vertex!=NULL)  {
                    list_insert_next(r_list,LIST_BOF,map_node_value(previous_shortest,map_find_node(previous_shortest,vertex)));
                    vertex=map_node_value(previous_shortest,map_find_node(previous_shortest,vertex));
                }
                return r_list; 
            }
            return NULL;
        }
        map_insert(spider_web,d_vertex->vertex,create_bool(1));
        List list=map_node_value(graph->map,map_find_node(graph->map,d_vertex->vertex));
        ListNode lnode=list_first(list);
        while (lnode!=LIST_EOF)  {
            if (map_node_value(spider_web,((W_Vertex)list_node_value(list,lnode))->vertex)==1)  {
                continue ;
            }
            uint alt_route=d_vertex->distance+((W_Vertex)list_node_value(list,lnode))->weight;
            if (alt_route<pqueue_node_value(pqueue,((W_Vertex) list_node_value(list,lnode))->vertex))  {
                
            }
        }
    }

}

void graph_destroy(Graph graph)  {
    map_destroy(graph->map);
    free(graph);
}

void graph_set_hash_function(Graph graph, HashFunc hash_func)  {
    map_set_hash_function(graph->map,hash_func);
}