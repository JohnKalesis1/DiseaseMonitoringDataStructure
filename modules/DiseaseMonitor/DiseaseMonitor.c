#include "DiseaseMonitor.h"
#include <stdlib.h>
#include <string.h>
#include "ADTMap.h"
#include "ADTSet.h"
struct dataset  {
    Map id_record_relate;
    Map disease_dates;
    Map country_dates;
    Map disease_country_related;
};
static struct dataset* DataBase=NULL;
int compare_ints(Pointer a,Pointer b)  {
    return *(int*)a-*(int*)b;
}
int date_compare(Pointer a,Pointer b)  {
    return strcmp(((Record)a)->date,((Record)b)->date);
}
void destroy_function(Pointer map)  {
    map=(Map)map;
    MapNode mnode=map_first(map);
    while (mnode!=MAP_EOF)  {
        set_destroy(map_node_value(map,mnode));
        mnode=map_next(map,mnode);
    }
    return ;
}
void dm_init()  {
    if (DataBase!=NULL)  {
        dm_destroy();
    }
    DataBase=malloc(sizeof(*DataBase));
    DataBase->id_record_relate=map_create(compare_ints,NULL,NULL);
    map_set_hash_function(DataBase->country_dates,hash_int);
    DataBase->country_dates=map_create(strcmp,NULL,NULL);
    map_set_hash_function(DataBase->country_dates,hash_string);
    DataBase->disease_dates=map_create(strcmp,NULL,NULL);
    map_set_hash_function(DataBase->disease_dates,hash_string);
    DataBase->disease_country_related=map_create(strcmp,NULL,NULL);
    map_set_hash_function(DataBase->disease_country_related,hash_string);
}

void dm_destroy()  {
    map_destroy(DataBase->country_dates);
    map_destroy(DataBase->disease_country_related);
    map_destroy(DataBase->disease_dates);
    free(DataBase);
}

bool dm_insert_record(Record record)  {
    if (map_find_node(DataBase->id_record_relate,record->id)!=MAP_EOF)  {
        dm_remove_record(record->id);
    }
    bool no_set_available=0;
    MapNode map_node;
    Set tree;
    if ((map_node=map_find_node(DataBase->disease_country_related,record->country))==MAP_EOF)  {
        Map disease_related_country_dates=map_create(strcmp,NULL,destroy_function);
        map_set_hash_function(disease_related_country_dates,hash_string);
        no_set_available=1;
        map_insert(DataBase->disease_country_related,record->country,record->disease);
    }
    else  {
        MapNode inner_node; 
        if ((inner_node=map_find_node(map_node_value(DataBase->disease_country_related,map_node),record->disease))==MAP_EOF )  {
            no_set_available=1;
        }
        else  {
            tree=map_node_value(map_node_value(DataBase->disease_country_related,map_node),inner_node);
            set_insert(tree,record);
        }
    }
    if (no_set_available==1)  {
        tree=set_create(date_compare,NULL);
        set_insert(tree,record);
        map_insert(map_node_value(DataBase->disease_country_related,map_node),record->disease,tree);
        tree=set_create(date_compare,NULL);
        set_insert(tree,record);
        map_insert(DataBase->disease_dates,record->disease,tree);
        tree=set_create(date_compare,NULL);
        set_insert(tree,record);
        map_insert(DataBase->country_dates,record->country,tree);
    }
    else  {
        set_insert(map_node_value(DataBase->country_dates,map_find_node(DataBase->country_dates,record->disease)),record);
        set_insert(map_node_value(DataBase->disease_dates,map_find_node(DataBase->disease_dates,record->country)),record);
    }   
    map_insert(DataBase->id_record_relate,record->id,record);    
}

bool dm_remove_record(int id)  {
    MapNode mnode=map_find_node(DataBase->id_record_relate,id);
    if (mnode!=MAP_EOF)  {
        Record record=map_node_value(DataBase->id_record_relate,mnode);
        
        mnode=map_find_node(DataBase->disease_country_related,record->country);
        Map sub_map=map_node_value(DataBase,mnode);
        mnode=map_find_node(sub_map,record->disease);
        Set tree=map_node_value(sub_map,mnode);
        set_remove(tree,record);

        mnode=map_find_node(DataBase->country_dates,record->country);
        tree=map_node_value(DataBase->country_dates,mnode);
        set_remove(tree,record);

        mnode=map_find_node(DataBase->disease_dates,record->disease);
        tree=map_node_value(DataBase->disease_dates,mnode);
        set_remove(tree,record);
    }
    map_remove(DataBase->id_record_relate,id);
}

List dm_get_records(String disease, String country, Date date_from, Date date_to)  {
    MapNode mnode;
    Set set;
    if (disease==NULL)  {
        mnode=map_find_node(DataBase->country_dates,country);
        set=map_node_value(DataBase->country_dates,mnode);
    }  
    else if (country==NULL)  {
        mnode=map_find_node(DataBase->disease_dates,disease);
        set=map_node_value(DataBase->disease_dates,mnode);
    }
    else  {
        mnode=map_find_node(DataBase->disease_country_related,country);
        MapNode inner_node=map_find_node(map_node_value(DataBase->disease_country_related,mnode),disease);
        set=map_node_value(map_node_value(DataBase->disease_country_related,mnode),inner_node);
    }
    List record_list=list_create(NULL);
    List list=list_create(NULL);
    if (date_from!=NULL)  {
        list=set_node_value(set,set_find_node(set,date_from));
    }
    else  {
        list=set_node_value(set,set_first(set));
    }
    ListNode temp_node=list_first(list);
    list_insert_next(record_list,LIST_BOF,list_node_value(list,temp_node));
    temp_node=list_next(list,temp_node);
    ListNode main_node=list_first(record_list);
    while (temp_node!=LIST_EOF)  {
        list_insert_next(record_list,main_node,list_node_value(list,temp_node));
        temp_node=list_next(list,temp_node);
        main_node=list_next(list,main_node);
    }
    list_destroy(list);
    Pointer Value=list_node_value()
    list=set_next(set,set_root(set),)
}

int dm_count_records(String disease, String country, Date date_from, Date date_to)  {
    MapNode mnode;
    Set set;
    int count1;
    int count2;
    if (disease==NULL)  {
        mnode=map_find_node(DataBase->country_dates,country);
        set=map_node_value(DataBase->country_dates,mnode);
    }  
    else if (country==NULL)  {
        mnode=map_find_node(DataBase->disease_dates,disease);
        set=map_node_value(DataBase->disease_dates,mnode);
    }
    else  {
        mnode=map_find_node(DataBase->disease_country_related,country);
        MapNode inner_node=map_find_node(map_node_value(DataBase->disease_country_related,mnode),disease);
        set=map_node_value(map_node_value(DataBase->disease_country_related,mnode),inner_node);
    }
    count1=set_size(set);
    if (date_from!=NULL)  {
        get_count_greater_equal(set,set_root(set),date_from,&count1);
    }
    if (date_to!=NULL)  {
        count2=set_size(set);
        get_count_greater_equal(set,set_root(set),date_to,&count2);
    }
    else  {
        count2=0;
    }
    return count1-count2;
}

List dm_top_diseases(int k, String country)  {

}