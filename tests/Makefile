# Κάνοντας compile το <foo>_test.c με μια υλοποίηση <foo>.c του
# συγκεκριμένου τύπου, παράγουμε ένα tets για την υλοποίηση αυτή.

# Υλοποιήσεις μέσω HashTable: ADTMap
#
UsingHashTable_ADTMap_test_OBJS	= ADTMap_test.o $(MODULES)/UsingHashTable/ADTMap.o

# ADTGraph
#
 UsingAdjacencyLists_ADTGraph_test_OBJS = ADTGraph_test.o $(MODULES)/UsingAdjacencyLists/ADTGraph.o $(MODULES)/UsingHashTable/ADTMap.o $(MODULES)/ADTPriorityQueue.o $(MODULES)/ADTVector.o $(MODULES)/UsingLinkedList/ADTList.o

# DiseaseMonitor
#
DiseaseMonitor_test_OBJS = DiseaseMonitor_test.o $(MODULES)/DiseaseMonitor/DiseaseMonitor.o $(MODULES)/DiseaseMonitor/Set_Mutated.o $(MODULES)/UsingHashTable/ADTMap.o $(MODULES)/ADTVector.o $(MODULES)/ADTList.o $(MODULES)/ADTPriorityQueue.o

# Ο βασικός κορμός του Makefile
include ../common.mk