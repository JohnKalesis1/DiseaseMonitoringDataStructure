#include "DiseaseMonitor.h"
#include <stdlib.h>
#include <string.h>
#include "ADTMap.h"
#include "Set_Mutated.h"
#include "ADTPriorityQueue.h"
#include <stdio.h>
struct dataset  {
    Map id_record_relate;  //αντιστοιχηση id =>record
    Map disease_dates;     //αντιστοιχηση disease=>set με records της ασθενειας
    Map country_dates;     //αντιστοιχηση country=>set με records της χωρας
    Map disease_country_related; //αντιστοιχηση country=>map που αντιστοιχει disease=>set με records της ασθενειας σε αυτη τη χωρα
    Set All_records;       //set με ολα τα κρουσματα
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
void s_destroy(Pointer set)  {
    set_destroy((Set)set);
    return ;
}
void m_destroy(Pointer map)  {
    map_destroy((Map)map);
    return ;
}
void dm_init()  {
    if (DataBase!=NULL)  {
        dm_destroy();
    }
    DataBase=malloc(sizeof(*DataBase));
    DataBase->id_record_relate=map_create(compare_ints,NULL,NULL);
    map_set_hash_function(DataBase->id_record_relate,hash_int);
    DataBase->country_dates=map_create(str_compare,NULL,s_destroy);
    map_set_hash_function(DataBase->country_dates,hash_string);
    DataBase->disease_dates=map_create(str_compare,NULL,s_destroy);
    map_set_hash_function(DataBase->disease_dates,hash_string);
    DataBase->disease_country_related=map_create(str_compare,NULL,m_destroy);
    map_set_hash_function(DataBase->disease_country_related,hash_string);
    DataBase->All_records=set_create(date_compare,NULL);
}

void dm_destroy()  {
    map_destroy(DataBase->id_record_relate);
    map_destroy(DataBase->country_dates);
    map_destroy(DataBase->disease_country_related);
    map_destroy(DataBase->disease_dates);
    set_destroy(DataBase->All_records);
    free(DataBase);
    DataBase=NULL;
}

bool dm_insert_record(Record record)  {
    bool return_bool=0;
    if (map_find_node(DataBase->id_record_relate,&(record->id))!=MAP_EOF)  { //αν το id υπαρχει ηδη 
        return_bool=1;
        dm_remove_record(record->id);  //το αφαιρουμε και επειτα το ξαναπροσθετουμε 
    }
    bool no_subset_available=0; //αν υπαρχει σετ διαθεσιμο που αντιστοιχει στο συνδιασμο χωρας,ασθενειας
    bool no_set1_available=0;   //αν υπαρχει σετ διαθεσιμο που να αντιστοιχει στην χωρα
    bool no_set2_available=0;   //αν υπαρχει σετ διαθεσιμο που να αντιστοιχει στην ασθενεια
    MapNode map_node;
    Set tree;
    if ((map_node=map_find_node(DataBase->disease_country_related,record->country))==MAP_EOF)  {
        Map disease_related_country_dates=map_create(str_compare,NULL,s_destroy); //αν δεν υπαρχει υπο πινακας με ασθενειες για αυτη τη χωρα
        map_set_hash_function(disease_related_country_dates,hash_string); //τοτε τον δημιουργουμε
        no_subset_available=1;  //και συναμα δε θα υπαρχει σιγουρα διαθεσιμο σετ
        map_insert(DataBase->disease_country_related,record->country,disease_related_country_dates);
    }
    else  {
        MapNode inner_node; 
        if ((inner_node=map_find_node(map_node_value(DataBase->disease_country_related,map_node),record->disease))==MAP_EOF )  {
            no_subset_available=1; //δεν υπαρχει σετ στον υπο πινακα 
        }
        else  {
            tree=map_node_value(map_node_value(DataBase->disease_country_related,map_node),inner_node);
            set_insert(tree,record); //αλλιως προσθετουμε το record 
        }
    }
    if (map_find_node(DataBase->country_dates,record->country)==MAP_EOF)  {
        no_set1_available=1; //δεν υπαρχει σετ για την χωρα αυτη
    }
    if (map_find_node(DataBase->disease_dates,record->disease)==MAP_EOF)  {
        no_set2_available=1; //δεν υπαρχει σετ για την ασθενεια αυτη
    }
    map_node=map_find_node(DataBase->disease_country_related,record->country);
    if (no_subset_available==1)  {
        tree=set_create(date_compare,NULL);
        set_insert(tree,record); //δημιουργια ενος σετ για συνδιασμο ασθενειας χωρας
        map_insert(map_node_value(DataBase->disease_country_related,map_node),record->disease,tree);
    }
    if (no_set1_available==1)  {
        tree=set_create(date_compare,NULL);
        set_insert(tree,record); 
        map_insert(DataBase->country_dates,record->country,tree);
    }
    if (no_set2_available==1)  {
        tree=set_create(date_compare,NULL);
        set_insert(tree,record);
        map_insert(DataBase->disease_dates,record->disease,tree);
    }
    
    if (no_set1_available==0)  {
        set_insert(map_node_value(DataBase->country_dates,map_find_node(DataBase->country_dates,record->country)),record);
    }
    if (no_set2_available==0)  {
        set_insert(map_node_value(DataBase->disease_dates,map_find_node(DataBase->disease_dates,record->disease)),record);
    }   
    set_insert(DataBase->All_records,record); //το προσθετουμε ουτως η αλλως στο σετ με ολα τα κρουσματα
    map_insert(DataBase->id_record_relate,&(record->id),record);   //και καταγραφουμε την υπαρξη του id
    return return_bool;  
}

bool dm_remove_record(int id)  {
    MapNode mnode=map_find_node(DataBase->id_record_relate,&id);
    if (mnode!=MAP_EOF)  { //αν το id υπαρχει ηδη το αφαιρουμε
        Record record=map_node_value(DataBase->id_record_relate,mnode); //βρισκουμε ποιο ειναι το record που πρεπει να αφαιρεσουμε 
        
        mnode=map_find_node(DataBase->disease_country_related,record->country);
        Map sub_map=(Map)map_node_value(DataBase->disease_country_related,mnode); //βρισκουμε σε ποιο sub map ανηκει το σετ που υπαρχει το στοιχειο μας
        mnode=map_find_node(sub_map,record->disease);
        Set tree=map_node_value(sub_map,mnode);
        set_remove(tree,record);  //το αφαιρουμε

        mnode=map_find_node(DataBase->disease_dates,record->disease);
        tree=map_node_value(DataBase->disease_dates,mnode);
        set_remove(tree,record);//το αφαιρουμε

        mnode=map_find_node(DataBase->country_dates,record->country);
        tree=map_node_value(DataBase->country_dates,mnode);
        set_remove(tree,record);//το αφαιρουμε

        set_remove(DataBase->All_records,record);//το αφαιρουμε

        map_remove(DataBase->id_record_relate,&id);
        return true;
    }
    return false;
   
}

List dm_get_records(String disease, String country, Date date_from, Date date_to)  {
    MapNode mnode;
    Set set;  //αρχικα πρεπει να επιλεξουμε απο ποιο σετ θα παρουμε τα στοιχεια μας βαση των κριτηριων ασθενειας χωρας
    List record_list=list_create(NULL); //υπαρχει η περιπτωση καμια καταγραφη να ταιριαζει στα κριτηρια των χωρων και ασθενειων , οποτε αν δε βρουμε επιστρεφουμε κενη λιστα
    if (disease==NULL)  { 
        if (country==NULL)  {
            set=DataBase->All_records;
            if (set_size(set)==0)  {
                return record_list;
            }
        }
        else  {
            mnode=map_find_node(DataBase->country_dates,country);
            if (mnode==MAP_EOF)  {
                return record_list;
            }
            set=map_node_value(DataBase->country_dates,mnode);
        }
    }  
    else if (country==NULL)  {
        mnode=map_find_node(DataBase->disease_dates,disease);
        if (mnode==MAP_EOF)  {
            return record_list;
        }
        set=map_node_value(DataBase->disease_dates,mnode);
    }
    else  {
        mnode=map_find_node(DataBase->disease_country_related,country);
        if (mnode==MAP_EOF)  {
            return record_list;
        }
        MapNode inner_node=map_find_node(map_node_value(DataBase->disease_country_related,mnode),disease);
        if (inner_node==MAP_EOF)  {
            return record_list;
        }
        set=map_node_value(map_node_value(DataBase->disease_country_related,mnode),inner_node);
    }
    List list;
    bool stop=0;
    SetNode prev=NULL;
    if (set_size(set)!=0)  { //αν το σετ δεν εχει στοιχεια ( μπορει να συμβει ,εφοσον δεν διαγραφουμε την υπαρξη ενος σετ , αν σε αυτο διαγραφουν ολα τα στοιχεια του)
        if (date_from!=NULL)  {
            list=set_find_greater_equal(set,set_root(set),date_from,&prev);  // επιστρεφει τη λιστα που περιεχει σετ που ειτε αντιστοιχουν στο κατω φραγμα που δωσαμε
            if (list==NULL)  {                                              // ειτε εχει τα στοιχεια με την αμεσως μεγαλυτερη ημερομηνια 
                stop=1; //αν κανενα στοιχειο δεν εχει μεγαλυτερη ημερομηνια απο το ανω φραγμα τοτε σταματαμε
            }
        }
        else  {
            list=set_node_value(set,set_first(set));  //αν δεν υπαρχει κατω φραγμα τοτε βρισκουμε τη λιστα στοιχειων με τη μικροτερη ημερομηνια
        }
        ListNode temp_node=list_first(list);
        if (stop!=1)  {
            if (date_to!=NULL)  {
                if (strcmp(((Record)list_node_value(list,temp_node))->date,date_to)>0)  {
                    stop=1; //αν ξεπερασουμε το ανω φραγμα τοτε σταματαμε , καθως επειδη διατρεχουμε σε αυξουσα διαταξη τις ημερομηνιες τα επομενα στοιχεια θα ναι ακομη μεγαλυτερα
                }
            }
        }
        if (stop!=1)  {
            while (temp_node!=LIST_EOF)  {
                list_insert_next(record_list,LIST_BOF,list_node_value(list,temp_node)); //αντιγραφη των στοιχεων της λιστας του κομβου στη λιστα που θα επιστρεψουμε 
                temp_node=list_next(list,temp_node);
            }
            while (true)  {
                if (stop==1)  {
                    break;   
                }
                Pointer Value=list_node_value(list,list_first(list));  //παντα για την αναζητηση του επομενου στοιχειου βαζουμε κατω φραγμα την ημερομηνια του προηγουμενου
                prev=NULL;
                list=set_next(set,set_root(set),Value,&prev);
                if (list==NULL)  {
                    break; //αν δεν υπαρχει μεγαλυτερο σταματαμε
                }
                else  {
                    temp_node=list_first(list);   
                    while (temp_node!=LIST_EOF)  { //αλλιως αφου ελεγξουμε οτι δε ξεπερνανε το αμω φραγμα τα αντιγραφουμε στη δικια μας λιστα
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
    return record_list;
 }

int dm_count_records(String disease, String country, Date date_from, Date date_to)  {
    MapNode mnode;
    Set set;
    int count1;
    int count2;  //πρψτα επιλεγουμε απο ποιο σετ θα μετρησουμε τα στοιχεια
    if (disease==NULL)  { 
        if (country==NULL)  {
            set=DataBase->All_records;       
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
    if (set_size(set)!=0)  { //προυποθεση το σετ να εχει στοιχεια
        if (date_from!=NULL)  { //αν υπαρχει κατω φραγμα μετραμε τα στοιχεια που χουν μεγαλυτερη ιση ημερομηνια με αυτο
            get_count_greater_equal(set,set_root(set),date_from,&count1); 
        }
        if (date_to!=NULL)  {
            count2=set_size(set);
            get_count_greater_equal(set,set_root(set),date_to,&count2);//αν υπαρχει ανω φραγμα μετραμε τα στοιχεια που χουν μεγαλυτερη ιση ημερομηνια με αυτο
            if (set_equal_value(set,set_root(set),date_to)!=NULL)  { //επειτα αν υπαρχει ιση τιμη με το ανω φραγμα 
                count2=count2-list_size((List)set_equal_value(set,set_root(set),date_to));  //την αφαιρουμε απο το αθροισμα 
            }
        }
    }
    return count1-count2; // και επιστρεφουμε την διαφορα του πληθους των στοιχειων 
}
struct disease_count  {
    int count;
    String disease;
};
typedef struct disease_count * Disease_count;
int compare_counts(Pointer a,Pointer b)  {
    return ((Disease_count)a)->count-((Disease_count)b)->count;
}
Disease_count create_count(String disease,int count)  {
    Disease_count Entity=malloc(sizeof(*Entity));
    Entity->disease=disease; 
    Entity->count=count;
    return Entity;
}
List dm_top_diseases(int k, String country)  {
    List list=list_create(NULL);
    PriorityQueue pqueue=pqueue_create(compare_counts,free,NULL); //η πυρα προτεραιοτητας με την οποια θα κανουμε sort τα στοιχεια 
    if (country==NULL)  {
        MapNode mnode=map_first(DataBase->disease_dates);// διατρεχουμε τον map με τα σετ της καθε ασθενειας 
        while (mnode!=MAP_EOF)  { //και τοποθετουμε στην ουρα το αθροισμα των κρουσματων της μαζι με το ονομα της (μεσα σε μια δομη)
            pqueue_insert(pqueue,create_count(map_node_key(DataBase->disease_dates,mnode),dm_count_records(map_node_key(DataBase->disease_dates,mnode),NULL,NULL,NULL)));
            mnode=map_next(DataBase->disease_dates,mnode);

        }
        int entries=0;   //τοποθετουμε απο μεγαλυτερο σε μικροτερο τα στοιχεια στη λιστα , εως να φτασουμε τον αριθμο των του ορισματος
        if (pqueue_size(pqueue)==0)  {  //περιπτωση να μην υπαρχει κανενα κρουσμα καταγγεγραμενο
            pqueue_destroy(pqueue);
            return list;
        }  
        list_insert_next(list,LIST_BOF,((Disease_count)pqueue_max(pqueue))->disease);
        pqueue_remove_max(pqueue);
        entries++;
        if (entries==k)  {
            pqueue_destroy(pqueue);
            return list;
        }
        ListNode lnode=list_first(list);
        while (pqueue_size(pqueue)!=0)  { //αν φτασουμε σε μηδενικο μεγεθος ουρας σημαινει οτι ο αριθμος ασθενειων που υπαρχουν στο συστημα εινια μικροτερος απο το ορισμα της συναρτησης
            list_insert_next(list,lnode,((Disease_count)pqueue_max(pqueue))->disease);// οπου και παλι απλα επιστρεφουμε την λιστα που δημιοργησαμε
            pqueue_remove_max(pqueue);
            lnode=list_next(list,lnode);
            entries++;
            if (entries==k)  {
                break;
            }
        }
        pqueue_destroy(pqueue);
        return list;
    }
    else  { //να δωθει σε ποια χωρα θελουμε να ψαξουμε , ομοιως διατρεχουμε τον sub map της και δημιουργουμε την ουρα προτεραιοτητας
        MapNode outer_node=map_find_node(DataBase->disease_country_related,country);
        if (outer_node!=MAP_EOF)  { //αν υπαρχει αυτη η χωρα για την οποια θελουμε να ψαξουμε τα κρουσματα της 
            MapNode inner_node=map_first(map_node_value(DataBase->disease_country_related,outer_node));
            while (inner_node!=MAP_EOF)  {
                pqueue_insert(pqueue,create_count(map_node_key(map_node_value(DataBase->disease_country_related,outer_node),inner_node),dm_count_records(map_node_key(map_node_value(DataBase->disease_country_related,outer_node),inner_node),country,NULL,NULL)));
                inner_node=map_next(map_node_value(DataBase->disease_country_related,outer_node),inner_node);
            }
            int entries=0;
            if (pqueue_size(pqueue)==0)  { ////περιπτωση να μην υπαρχει κανενα κρουσμα για αυτη τη χωρα
                pqueue_destroy(pqueue);
                return list;
            }  
            String disease=((Disease_count)pqueue_max(pqueue))->disease;
            list_insert_next(list,LIST_BOF,disease);
            pqueue_remove_max(pqueue);
            entries++;
            if (entries==k)  {
                pqueue_destroy(pqueue);
                return list;
            }
            ListNode lnode=list_first(list);
            while (pqueue_size(pqueue)!=0)  {
                list_insert_next(list,lnode,((Disease_count)pqueue_max(pqueue))->disease);
                pqueue_remove_max(pqueue);
                lnode=list_next(list,lnode);
                entries++;
                if (entries==k)  {
                    break;
                }
            }
            pqueue_destroy(pqueue);
            return list;
        }
        else  {
            pqueue_destroy(pqueue);
            return list;
        }
    }
}