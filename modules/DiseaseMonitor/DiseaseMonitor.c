#include "DiseaseMonitor.h"
#include <stdlib.h>
#include <string.h>
#include "ADTMap.h"
#include "Set_Mutated.h"
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
int str_compare(Pointer a,Pointer b)  {
    return strcmp((char*)a,(char*)b);
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
    map_set_hash_function(DataBase->id_record_relate,hash_int);
    DataBase->country_dates=map_create(str_compare,NULL,NULL);
    map_set_hash_function(DataBase->country_dates,hash_string);
    DataBase->disease_dates=map_create(str_compare,NULL,NULL);
    map_set_hash_function(DataBase->disease_dates,hash_string);
    DataBase->disease_country_related=map_create(str_compare,NULL,NULL);
    map_set_hash_function(DataBase->disease_country_related,hash_string);
}

void dm_destroy()  {
    map_destroy(DataBase->id_record_relate);
    map_destroy(DataBase->country_dates);
    map_destroy(DataBase->disease_country_related);
    map_destroy(DataBase->disease_dates);
    free(DataBase);
    DataBase=NULL;
}

bool dm_insert_record(Record record)  {
    bool return_bool=0;
    if (map_find_node(DataBase->id_record_relate,&(record->id))!=MAP_EOF)  {
        return_bool=1;
        dm_remove_record(record->id);
    }
    bool no_subset_available=0;
    bool no_set1_available=0;
    bool no_set2_available=0;
    MapNode map_node;
    Set tree;
    if ((map_node=map_find_node(DataBase->disease_country_related,record->country))==MAP_EOF)  {
        Map disease_related_country_dates=map_create(str_compare,NULL,destroy_function);
        map_set_hash_function(disease_related_country_dates,hash_string);
        no_subset_available=1;
        map_insert(DataBase->disease_country_related,record->country,disease_related_country_dates);
    }
    else  {
        MapNode inner_node; 
        if ((inner_node=map_find_node(map_node_value(DataBase->disease_country_related,map_node),record->disease))==MAP_EOF )  {
            no_subset_available=1;
        }
        else  {
            tree=map_node_value(map_node_value(DataBase->disease_country_related,map_node),inner_node);
            set_insert(tree,record);
        }
    }
    if (map_find_node(DataBase->country_dates,record->country)==MAP_EOF)  {
        no_set1_available=1;
    }
    if (map_find_node(DataBase->disease_dates,record->disease)==MAP_EOF)  {
        no_set2_available=1;
    }
    map_node=map_find_node(DataBase->disease_country_related,record->country);
    if (no_subset_available==1)  {
        tree=set_create(date_compare,NULL);
        set_insert(tree,record);
        map_insert(map_node_value(DataBase->disease_country_related,map_node),record->disease,tree);
        if (no_set2_available==1)  {
            tree=set_create(date_compare,NULL);
            set_insert(tree,record);
            map_insert(DataBase->disease_dates,record->disease,tree);
        }
        if (no_set1_available==1)  {
            tree=set_create(date_compare,NULL);
            set_insert(tree,record);
            map_insert(DataBase->country_dates,record->country,tree);
        }
    }
    if (no_set1_available==0)  {
        set_insert(map_node_value(DataBase->country_dates,map_find_node(DataBase->country_dates,record->country)),record);
    }
    if (no_set2_available==0)  {
        set_insert(map_node_value(DataBase->disease_dates,map_find_node(DataBase->disease_dates,record->disease)),record);
    }   
    map_insert(DataBase->id_record_relate,&(record->id),record);  
    return return_bool;  
}

bool dm_remove_record(int id)  {
    MapNode mnode=map_find_node(DataBase->id_record_relate,&id);
    if (mnode!=MAP_EOF)  {
        Record record=map_node_value(DataBase->id_record_relate,mnode);
        
        mnode=map_find_node(DataBase->disease_country_related,record->country);
        Map sub_map=(Map)map_node_value(DataBase->disease_country_related,mnode);
        mnode=map_find_node(sub_map,record->disease);
        Set tree=map_node_value(sub_map,mnode);
        set_remove(tree,record);
        /*if (set_size(tree)==0)  {
            map_remove(sub_map,record->disease);
        }*/

        mnode=map_find_node(DataBase->disease_dates,record->disease);
        tree=map_node_value(DataBase->disease_dates,mnode);
        set_remove(tree,record);
        /*if (set_size(tree)==0)  {
            map_remove(DataBase->disease_dates,record->disease);
        }*/

        mnode=map_find_node(DataBase->country_dates,record->country);
        tree=map_node_value(DataBase->country_dates,mnode);
        set_remove(tree,record);
        /*if (set_size(tree)==0)  {
            map_remove(DataBase->country_dates,record->country);
        }*/

        

        map_remove(DataBase->id_record_relate,&id);
        return true;
    }
    return false;
   
}

List dm_get_records(String disease, String country, Date date_from, Date date_to)  {
    MapNode mnode;
    Set set;
    List record_list=list_create(NULL);
    if (disease==NULL)  {
        if (country==NULL)  {
            mnode=map_first(DataBase->country_dates);
            while (mnode!=MAP_EOF)  {
                List list=list_create(NULL);
                set=map_node_value(DataBase->country_dates,mnode);
                bool stop=0;
                SetNode prev=NULL;
                if (set_size(set)!=0)  {
                    if (date_from!=NULL)  {
                        list=set_find_greater_equal(set,set_root(set),date_from,&prev);
                    }
                    else  {
                        list=set_node_value(set,set_first(set));
                    }
                    ListNode temp_node=list_first(list);
                    if (date_to!=NULL)  {
                        if (strcmp(((Record)list_node_value(list,temp_node))->date,date_to)>0)  {
                            stop=1;
                        }
                    }
                    if (stop==0)  {
                        while (temp_node!=LIST_EOF)  {
                            list_insert_next(record_list,LIST_BOF,list_node_value(list,temp_node));
                            temp_node=list_next(list,temp_node);
                        }
                        while (true)  {
                            if (stop==1)  {
                                break;
                            }
                            if (list_size(list)==0)  {
                                break;
                            }
                            Pointer Value=list_node_value(list,list_first(list));
                            prev=NULL;
                            list=set_next(set,set_root(set),Value,&prev);
                            if (list_size(list)==0)  {
                                break;
                            }
                            else  {
                                temp_node=list_first(list);   
                                while (temp_node!=LIST_EOF)  {
                                    if (date_to!=NULL)  {
                                        if (strcmp(((Record)list_node_value(list,temp_node))->date,date_to)>0)  {
                                            stop=1;
                                            break;
                                        }
                                    }
                                    list_insert_next(record_list,LIST_BOF,list_node_value(list,temp_node));
                                    temp_node=list_next(list,temp_node);
                                }
                            }
                        }
                    }   
                }
                mnode=map_next(DataBase->country_dates,mnode);
            }
            return record_list;
        }
        else  {
            mnode=map_find_node(DataBase->country_dates,country);
            set=map_node_value(DataBase->country_dates,mnode);
        }
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
    List list=list_create(NULL);
    bool stop=0;
    SetNode prev=NULL;
    if (date_from!=NULL)  {
        list=set_find_greater_equal(set,set_root(set),date_from,&prev);
    }
    else  {
        list=set_node_value(set,set_first(set));
    }
    ListNode temp_node=list_first(list);
    if (date_to!=NULL)  {
        if (strcmp(((Record)list_node_value(list,temp_node))->date,date_to)>0)  {
            stop=1;
        }
    }
    if (stop==0)  {
        while (temp_node!=LIST_EOF)  {
            list_insert_next(record_list,LIST_BOF,list_node_value(list,temp_node));
            temp_node=list_next(list,temp_node);
        }
        while (true)  {
            if (stop==1)  {
                break;
            }
            Pointer Value=list_node_value(list,list_first(list));
            prev=NULL;
            list=set_next(set,set_root(set),Value,&prev);
            if (list_size(list)==0)  {
                break;
            }
            else  {
                temp_node=list_first(list);   
                while (temp_node!=LIST_EOF)  {
                    if (date_to!=NULL)  {
                        if (strcmp(((Record)list_node_value(list,temp_node))->date,date_to)>0)  {
                            stop=1;
                            break;
                        }
                    }
                    list_insert_next(record_list,LIST_BOF,list_node_value(list,temp_node));
                    temp_node=list_next(list,temp_node);
                }
            }
        }
    }
    return record_list;
 }

int dm_count_records(String disease, String country, Date date_from, Date date_to)  {
    MapNode mnode;
    Set set;
    int count1;
    int count2;
    if (disease==NULL)  {
        if (country==NULL)  {
            int count_all=0;
            mnode=map_first(DataBase->country_dates);
            while (mnode!=MAP_EOF)  {
                set=map_node_value(DataBase->country_dates,mnode);
                count1=set_size(set);
                count2=0;
                if (set_size(set)!=0)  {
                    if (date_from!=NULL)  {
                        get_count_greater_equal(set,set_root(set),date_from,&count1);
                    }
                    if (date_to!=NULL)  {
                        count2=set_size(set);
                        get_count_greater_equal(set,set_root(set),date_to,&count2);
                        if (set_equal_value(set,set_root(set),date_to)!=NULL)  {
                            count2=count2-list_size((List)set_equal_value(set,set_root(set),date_to));
                        }
                    }
                    else  {
                        count2=0;
                    }
                }
                count_all=count_all+(count1-count2);
                mnode=map_next(DataBase->country_dates,mnode);
            }
            return count_all;       
        }
        else  {
            mnode=map_find_node(DataBase->country_dates,country);
            set=map_node_value(DataBase->country_dates,mnode);
        }
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
    count2=0;
    if (set_size(set)!=0)  {
        if (date_from!=NULL)  {
            get_count_greater_equal(set,set_root(set),date_from,&count1);
        }
        if (date_to!=NULL)  {
            count2=set_size(set);
            get_count_greater_equal(set,set_root(set),date_to,&count2);
            if (set_equal_value(set,set_root(set),date_to)!=NULL)  {
                count2=count2-list_size((List)set_equal_value(set,set_root(set),date_to));
            }
        }
        else  {
            count2=0;
        }
    }
    return count1-count2;
}

List dm_top_diseases(int k, String country)  {
    return NULL;
}