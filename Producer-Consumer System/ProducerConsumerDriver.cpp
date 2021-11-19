//Nathan Sexton 4100-Program 2
//April, 2021
//mybuild.sh is for this program and compiles into ProducerConsumer.exe

#include <cmath>
#include <ctime>
#include <cstdlib>
#include <cstdio>

#include "BinarySearchTree.h"
using CSC1310::BinarySearchTree;
#include "BinaryTreeIterator.h"
using CSC1310::BinaryTreeIterator;
#include "ListArray.h"
using CSC1310::ListArray;
#include "ListArrayIterator.h"
using CSC1310::ListArrayIterator;

#include "Permutation.h"
using CSC1310::Permutation;
#include "Random.h"
using CSC1310::Random;

#include <pthread.h>
#include "CD.h"

#include <iostream>
using namespace std;


 typedef struct _producer_parallel_args {
	 ListArray<CD>* cds;
	 Random* rand;
	 int num_trees;
 } producer_parallel_args;

 typedef struct _consumer_parallel_args {
	 int num_items;
	 int expected_height;
 } consumer_parallel_args;

int BUFFER_SIZE = 100;
int NUM_PRODUCERS = 10;
int NUM_CONSUMERS = 50;
int NUM_TREES = 10000;
int NUM_ERRORS = 10;

CD*** buffer;
int buffer_count = 0;
int producer_index = 0;
int consumer_index = 0;

int num_trees_p = 0;
int num_trees_c = 0;

pthread_cond_t empty, full;
pthread_mutex_t mutex;

void put(CD** cds_array);
CD** get();
void deleteCDs(ListArray<CD>* list);
CD** producer_seq(ListArray<CD>* cds, Random* rand);
void consumer_seq(CD** cds_array, int num_items, int expected_height);
void* producer_parallel(void* args);
void* consumer_parallel(void* args);


int main()
{
	buffer = new CD**[BUFFER_SIZE];
	
	time_t start, end;
	Random* rand = Random::getRandom();
	   
   //the unsorted ListArray of cds
   ListArray<CD>* cds = CD::readCDs("cds.txt");
   int num_items = cds->size();
   cout << num_items << endl;
   cout << "based on the number of items, the min height should be: " << endl;
   int expected_height = ceil(log2(num_items + 1));
   cout << expected_height << endl;
   cout << endl;
   
   long* producer_args = new long[2];
   producer_args[0] = (long) cds;
   producer_args[1] = (long) rand;
   long** temp_p = &producer_args;
   
   int* consumer_args = new int[2];
   consumer_args[0] = num_items;
   consumer_args[1] = expected_height;
   int** temp_c = &consumer_args;


   //Creating arguments for producer_parallel
   producer_parallel_args prod_args;
   prod_args.cds = cds;
   prod_args.rand = rand;
   //Creating arguments for consumer_parallel
   consumer_parallel_args con_args;
   con_args.num_items = num_items;
   con_args.expected_height = expected_height;
  
   //Sequential 
	cout << "Begin Sequential\n----------------\n";
    start = time(NULL);
//sequential solution: process one tree at a time
	for (int i = 1; i <= NUM_TREES; i++)
	{
	    CD** cd_array = producer_seq(cds, rand);
		consumer_seq(cd_array, num_items, expected_height);
	}
   
    end = time(NULL);
   printf("sequential: %ds\n", (int)(end - start)); 
	
	//Parallel 
	cout << "\nBegin Parallel\n----------------\n";
    start = time(NULL);
	
	pthread_t producers[NUM_PRODUCERS];
	pthread_t consumers[NUM_CONSUMERS];

	//Allows for different variations of producers and consumers
	int min_size = std::min(NUM_CONSUMERS, NUM_PRODUCERS);

	for (int i = 0; i < NUM_PRODUCERS; i++) {
		pthread_create(&producers[i], NULL, &producer_parallel, &prod_args);
	}
	for (int i = 0; i < NUM_CONSUMERS; i++) {
		pthread_create(&consumers[i], NULL, &consumer_parallel, &con_args);
	}
	for (int i = 0; i < min_size; i++) {
		pthread_join(producers[i], NULL);
		pthread_join(consumers[i], NULL);
	}

    end = time(NULL);
    printf("parallel: %ds\n", (int)(end - start)); 


   delete[] producer_args;
   delete[] consumer_args;
   deleteCDs(cds);
   delete cds;
   delete[] buffer;

   return 0;
}

void put(CD** cds_array)
{
	buffer[producer_index] = cds_array;
	producer_index = (producer_index + 1) % BUFFER_SIZE;
	buffer_count++;  //buffer fills up
}

CD** get()
{
	CD** cds_array = buffer[consumer_index];
	consumer_index = (consumer_index + 1) % BUFFER_SIZE;
	buffer_count--;  //buffer empties out
	return cds_array;
}

void deleteCDs(ListArray<CD>* list)
{
   ListArrayIterator<CD>* iter = list->iterator();

   while(iter->hasNext())
   {
      CD* cd = iter->next();
      delete cd;
   }
   delete iter;
}

CD** producer_seq(ListArray<CD>* cds, Random* rand)
{
//1.  get an array with the cds in a random order
	    int num_items = cds->size();
		Permutation* p = new Permutation(num_items, num_items, rand);
		p->basicInit();

		CD** permute_cds = new CD*[num_items];
		int count = 0;
		while(p->hasNext())
		{
			int i = p->next();
			permute_cds[count] = cds->get(i);
			count++;
		}
		delete p;

//2.  insert the cds in array order (different each time) into BST
		BinarySearchTree<CD>* bst = new BinarySearchTree<CD>(&CD::compare_items, &CD::compare_keys);
		for (int i = 0; i < num_items; i++)
		{
			CD* cd = permute_cds[i];
			bst->insert(cd);
		}
		delete[] permute_cds;
  
//3.  create a complete binary search tree
		BinarySearchTree<CD>* complete_bst = bst->minimizeComplete();
		CD** cds_array = new CD*[num_items];
		BinaryTreeIterator<CD>* complete_iter = complete_bst->iterator();

//4.  place the cds into an array using a level order traversal
		//intentionally inserting an error in some cases using a post order traversal
		int traversal_error = rand->getRandomInt(1, NUM_TREES);
		if (traversal_error >= NUM_ERRORS)
		{
			complete_iter->setLevelorder();
		}
		else
		{
			//wrong traversal, consumer should detect a height error
			//still will be sorted, however
			complete_iter->setPostorder();
		}
		count = 0;
		while(complete_iter->hasNext())
		{
			cds_array[count] = complete_iter->next();
			count++;
		}
		delete bst;
		delete complete_iter;
		delete complete_bst;

//5.  return the array that is supposed to represent a complete binary tree		
		return cds_array;
}

void consumer_seq(CD** cds_array, int num_items, int expected_height)
{
//1.  put the items in the provided array into a BST
//note that if the array represents a complete binary tree,
//this process will create a complete binary tree that is also a BST
		BinarySearchTree<CD>* bst = new BinarySearchTree<CD>(&CD::compare_items, &CD::compare_keys);
		for (int i = 0; i < num_items; i++)
		{
			CD* cd = cds_array[i];
			bst->insert(cd);
		}
   
//2.  verify that the items are in sorted order using an inorder traversal
		BinaryTreeIterator<CD>* bst_iter = bst->iterator();
		CD* first = bst_iter->next();
		bool sorted = true;
		while(bst_iter->hasNext())
		{
			CD* second = bst_iter->next();
			if (CD::compare_items(first, second) >= 0)
			{
				sorted = false;
				break;
			}
			first = second;
		}
		delete bst_iter;
   
//3.  check that the BST is minimum height and balanced
//the randomly determined errors will fail the height test
		int h = bst->getHeight();
		bool bal = bst->isBalanced();
   
		if (!sorted || h != expected_height || !bal)
		{
			cout << "invalid complete binary tree" << endl;
		}
		else
		{
			//cout << "valid complete binary tree" << endl;
		}
		
		delete bst;
		delete[] cds_array;
}

void* producer_parallel(void* args) {
	
	producer_parallel_args* p_args = (producer_parallel_args*) args;
	ListArray<CD>* cds = p_args->cds;
	Random* rand = p_args->rand;

	while (num_trees_p < NUM_TREES) {
//1.  get an array with the cds in a random order
		int num_items = cds->size();
		Permutation* p = new Permutation(num_items, num_items, rand);
		p->basicInit();

		CD** permute_cds = new CD*[num_items];
		int count = 0;
		while(p->hasNext())
		{
			int i = p->next();
			permute_cds[count] = cds->get(i);
			count++;
		}
		delete p;

//2.  insert the cds in array order (different each time) into BST
		BinarySearchTree<CD>* bst = new BinarySearchTree<CD>(&CD::compare_items, &CD::compare_keys);
		for (int i = 0; i < num_items; i++)
		{
			CD* cd = permute_cds[i];
			bst->insert(cd);
		}
		delete[] permute_cds;

//3.  create a complete binary search tree
		BinarySearchTree<CD>* complete_bst = bst->minimizeComplete();
		CD** cds_array = new CD*[num_items];
		BinaryTreeIterator<CD>* complete_iter = complete_bst->iterator();

//4.  place the cds into an array using a level order traversal
		//intentionally inserting an error
		int traversal_error = rand->getRandomInt(1, NUM_TREES);
		if (traversal_error >= NUM_ERRORS)
		{
			complete_iter->setLevelorder();
		}
		else
		{
			//wrong traversal, consumer should detect a height error
			//still will be sorted, however
			complete_iter->setPostorder();
		}
		count = 0;
		while(complete_iter->hasNext())
		{
			cds_array[count] = complete_iter->next();
			count++;
		}
		delete bst;
		delete complete_iter;
		delete complete_bst;
		
		//Creating the lock for inserting cds_array into the buffer
		pthread_mutex_lock(&mutex);
		while (buffer_count == BUFFER_SIZE) {
			pthread_cond_wait(&empty, &mutex);
		}
		put(cds_array);
		num_trees_p++;
		pthread_cond_signal(&full);
		pthread_mutex_unlock(&mutex);
		
	}
}

void* consumer_parallel(void* args) {
	//Save arguments passed
	consumer_parallel_args* c_args = (consumer_parallel_args*) args;
	int num_items = c_args->num_items;
	int expected_height = c_args->expected_height;
	
	while (num_trees_c < NUM_TREES)
	{
		pthread_mutex_lock(&mutex);
		while (buffer_count == 0) {
			pthread_cond_wait(&full, &mutex);	
		}
		CD** cds_array = get();
		num_trees_c++;
		pthread_cond_signal(&empty);
		pthread_mutex_unlock(&mutex);

//1.  put the items in the provided array into a BST
//note that if the array represents a complete binary tree,
//this process will create a complete binary tree that is also a BST
		BinarySearchTree<CD>* bst = new BinarySearchTree<CD>(&CD::compare_items, &CD::compare_keys);
			for (int i = 0; i < num_items; i++)
			{
				CD* cd = cds_array[i];
				bst->insert(cd);
			}
	
//2.  verify that the items are in sorted order using an inorder traversal
			BinaryTreeIterator<CD>* bst_iter = bst->iterator();
			bst_iter->setInorder();  //this was omitted
			CD* first = bst_iter->next();
			bool sorted = true;
			while(bst_iter->hasNext())
			{
				CD* second = bst_iter->next();
				if (CD::compare_items(first, second) >= 0)
				{
					sorted = false;
					break;
				}
				first = second;
			}
			delete bst_iter;
	
//3.  check that the BST is minimum height and balanced
//the randomly determined errors will fail the height test
			int h = bst->getHeight();
			bool bal = bst->isBalanced();
	
			if (!sorted || h != expected_height || !bal)
			{
				cout << "invalid complete binary tree" << endl;
			}
			else
			{
				//cout << "valid complete binary tree" << endl;
			}

			delete bst;
			delete[] cds_array;
	}
		

}

