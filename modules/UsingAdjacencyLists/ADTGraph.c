#include "ADTGraph.h"
#include "ADTPriorityQueue.h"
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#define LIMIT 5000

struct graph {
    Map map;
    int size;
    CompareFunc compare;
    DestroyFunc destroy;
    HashFunc hash_function;
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
    W_Vertex weighted_vertex=malloc(sizeof(*weighted_vertex));
    weighted_vertex->vertex=vertex;
    weighted_vertex->weight=weight;
    return weighted_vertex;
}

D_Vertex create_distance(Pointer vertex,uint distance)  {
    D_Vertex distance_vertex=malloc(sizeof(*distance_vertex));
    distance_vertex->vertex=vertex;
    distance_vertex->distance=distance;
    return distance_vertex;
}

Graph graph_create(CompareFunc compare, DestroyFunc destroy_vertex)  {
    Graph graph=malloc(sizeof(*graph));
    graph->map=map_create(compare,destroy_vertex,(Pointer)list_destroy);
    graph->size=0;
    graph->compare=compare;
    graph->destroy=destroy_vertex;

    return graph;
}

int graph_size(Graph graph)  {
    return graph->size;
}

void graph_insert_vertex(Graph graph, Pointer vertex)  {
    List list=list_create(free);
    map_insert(graph->map,vertex,list);
    graph->size++;
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
    List list=map_node_value(graph->map,map_find_node(graph->map,vertex));
    ListNode lnode=list_first(list);
    if (list_size(list)!=0)  {
        Pointer adj_vertex=((W_Vertex)list_node_value(list,lnode))->vertex;
        graph_remove_edge(graph,vertex,adj_vertex);
    }
    while(list_size(list)!=0)  {
        Pointer adj_vertex=((W_Vertex)list_node_value(list,list_next(list,lnode)))->vertex;
        graph_remove_edge(graph,vertex,adj_vertex);
        lnode=list_next(list,lnode);
    }
    map_remove(graph->map,vertex);
    graph->size--;
    return ;
}

void graph_insert_edge(Graph graph, Pointer vertex1, Pointer vertex2, uint weight)  {
    MapNode m_node1=map_find_node(graph->map,vertex1);
    MapNode m_node2=map_find_node(graph->map,vertex2);
    List list;
    list=(List)map_node_value(graph->map,m_node1);
    list_insert_next(list,LIST_BOF,create_edge(vertex2,weight));
    //map_insert(graph->map,vertex1,list);
    list=(List)map_node_value(graph->map,m_node2);
    list_insert_next(list,LIST_BOF,create_edge(vertex1,weight));
    //map_insert(graph->map,vertex2,list);
    return ;
}

void graph_remove_edge(Graph graph, Pointer vertex1, Pointer vertex2)  {
    MapNode m_node1=map_find_node(graph->map,vertex1);
    MapNode m_node2=map_find_node(graph->map,vertex2);
    List list;
    ListNode lnode;
    list=(List)map_node_value(graph->map,m_node1);
    lnode=list_first(list);
    if (graph->compare(((W_Vertex)list_node_value(list,lnode))->vertex,vertex2)==0)  {
        list_remove_next(list,LIST_BOF);
    }
    else  {
        while(graph->compare(((W_Vertex)list_node_value(list,list_next(list,lnode)))->vertex,vertex2)!=0)  {
            lnode=list_next(list,lnode);
        }
        list_remove_next(list,lnode);
    }
    //map_insert(graph->map,vertex1,list);
    list=(List)map_node_value(graph->map,m_node2);
    lnode=list_first(list);
    if (graph->compare(((W_Vertex)list_node_value(list,lnode))->vertex,vertex1)==0)  {
        list_remove_next(list,LIST_BOF);
    }
    else  {
        while(graph->compare(((W_Vertex)list_node_value(list,list_next(list,lnode)))->vertex,vertex1)!=0)  {
            lnode=list_next(list,lnode);
        }
        list_remove_next(list,lnode);
    }
    //map_insert(graph->map,vertex2,list);
    return ;
}

uint graph_get_weight(Graph graph, Pointer vertex1, Pointer vertex2)  {
    MapNode m_node=map_find_node(graph->map,vertex1);
    List list=(List)map_node_value(graph->map,m_node);
    ListNode lnode=list_first(list);
    if (lnode!=LIST_BOF)  {
        while (lnode!=LIST_EOF && graph->compare(((W_Vertex)list_node_value(list,lnode))->vertex,vertex2)!=0)  {
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
        list_insert_next(r_list,LIST_BOF,((W_Vertex)list_node_value(list,lnode))->vertex);
        lnode=list_next(list,lnode);
    }
    return r_list;
}

bool* create_bool(bool value)  {
    bool* p;
    p=malloc(sizeof(*p));
    *p=value;
    return p;
}
int* create_int(int value)  {
    int* p;
    p=malloc(sizeof(*p));
    *p=value;
    return p;
}

int compare_distances(Pointer a,Pointer b)  {
    D_Vertex vertex1= (D_Vertex) a;
    D_Vertex vertex2= (D_Vertex) b;
    return (-1)*(vertex1->distance-vertex2->distance);
}
int compare_bool(Pointer a,Pointer b)  {
    return (*(bool*)a-*(bool*)b);
}
int unimportant(Pointer a,Pointer b)  {
    return 0;
}
List graph_shortest_path(Graph graph, Pointer source, Pointer target)  {
    Map spider_web=map_create(compare_bool,NULL,free);
    Map pqueue_vertex_relate=map_create(graph->compare,NULL,NULL);
    PriorityQueue distance_array=pqueue_create(compare_distances,NULL,NULL);
    Map previous_shortest=map_create(unimportant,NULL,NULL);
    map_set_hash_function(spider_web,graph->hash_function);
    map_set_hash_function(pqueue_vertex_relate,graph->hash_function);
    map_set_hash_function(previous_shortest,graph->hash_function);
    MapNode mnode=map_first(graph->map);
    List r_list=list_create(NULL);
    for (int i=0;i<graph->size;i++)  {
        map_insert(spider_web,map_node_key(graph->map,mnode),create_bool(0));
        mnode=map_next(graph->map,mnode);
    }
    map_insert(previous_shortest,source,NULL);
    //map_insert(spider_web,source,create_bool(1));
    PriorityQueueNode qnode=pqueue_insert(distance_array,create_distance(source,0));
    map_insert(pqueue_vertex_relate,source,qnode);
    D_Vertex d_vertex;
    while(pqueue_size(distance_array)!=0)  {
        d_vertex=pqueue_max(distance_array);
        pqueue_remove_max(distance_array);
        if (*(bool*)map_node_value(spider_web,map_find_node(spider_web,d_vertex->vertex))==true)  {
            free(d_vertex);
            continue ;
        }
        if (graph->compare(d_vertex->vertex,target)==0)  {
            if (map_node_value(previous_shortest,map_find_node(previous_shortest,target))!=NULL)  {
                list_insert_next(r_list,LIST_BOF,target);
                Pointer vertex=target;
                while ((vertex=map_node_value(previous_shortest,map_find_node(previous_shortest,vertex)))!=NULL)  {
                    list_insert_next(r_list,LIST_BOF,vertex);
                }
            }
            free(d_vertex);
            pqueue_set_destroy_value(distance_array,free);
            map_destroy(spider_web);
            map_destroy(previous_shortest);
            map_destroy(pqueue_vertex_relate);
            pqueue_destroy(distance_array);
            return r_list;
        }
        map_insert(spider_web,d_vertex->vertex,create_int(1));
        List list=map_node_value(graph->map,map_find_node(graph->map,d_vertex->vertex));
        ListNode lnode=list_first(list);
        while (lnode!=LIST_EOF)  {
            if (*(bool*)map_node_value(spider_web,map_find_node(spider_web,((W_Vertex)list_node_value(list,lnode))->vertex))==1)  {
                lnode=list_next(list,lnode);
                continue ;
            }
            uint alt_route=d_vertex->distance+((W_Vertex)list_node_value(list,lnode))->weight;
            if (map_find_node(pqueue_vertex_relate,((W_Vertex) list_node_value(list,lnode))->vertex)!=MAP_EOF)  {
                qnode=map_node_value(pqueue_vertex_relate,map_find_node(pqueue_vertex_relate,((W_Vertex) list_node_value(list,lnode))->vertex));
                uint d=((D_Vertex)pqueue_node_value(distance_array,qnode))->distance;
                if (alt_route<d)  {
                    ((D_Vertex)pqueue_node_value(distance_array,qnode))->distance=alt_route;
                    pqueue_update_order(distance_array,qnode);
                    map_insert(previous_shortest,((W_Vertex)list_node_value(list,lnode))->vertex,d_vertex->vertex);
                }
            }
            else  {
                qnode=pqueue_insert(distance_array,create_distance(((W_Vertex)list_node_value(list,lnode))->vertex,d_vertex->distance+((W_Vertex)list_node_value(list,lnode))->weight));
                map_insert(pqueue_vertex_relate,((W_Vertex)list_node_value(list,lnode))->vertex,qnode);
                map_insert(previous_shortest,((W_Vertex)list_node_value(list,lnode))->vertex,d_vertex->vertex);
            }
            lnode=list_next(list,lnode);   
        }
        free(d_vertex);
    }
    pqueue_set_destroy_value(distance_array,free);
    map_destroy(spider_web);
    map_destroy(previous_shortest);
    map_destroy(pqueue_vertex_relate);
    pqueue_destroy(distance_array);
    return r_list;
}

void graph_destroy(Graph graph)  {
    map_destroy(graph->map);
    free(graph);
}

void graph_set_hash_function(Graph graph, HashFunc hash_func)  {
    map_set_hash_function(graph->map,hash_func);
    graph->hash_function=hash_func;
}
