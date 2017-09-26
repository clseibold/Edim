#include <stdio.h>
#include <stdlib.h>

typedef struct Node
{
	int data;
	struct Node *next;
} Node;

/* Create node */
Node *node_create(int data, Node *next)
{
	Node *node = malloc(sizeof(Node));
	node->data = data;
	node->next = next;

	return node;
}

/* Finds first node after starting node whose data is same as given data.
 * It stops at the ending node.
 */
Node *node_find(Node *start, int data, Node *end)
{
	Node *tmp = start;

	while (tmp->next != end && tmp->data != data)
	{
		tmp = tmp->next;
	}
	
	if (tmp->next == end && tmp->next->data != data)
	{
		return NULL;
	}

	return tmp;
}

Node *node_find_index(Node *start, int index, Node *end)
{
	Node *tmp = start;
	int i = 0;

	while (tmp->next != end && i != index)
	{
		tmp = tmp->next;
		++i;
	}

	if (tmp->next == end)
	{
		return NULL;
	}

	return tmp;
}

/* Finds the node before the first node after starting node whose data is same as given data.
 * It stops at the ending node.
 */
Node *node_find_before(Node *start, int data, Node *end)
{
	Node *tmp = start;

	while (tmp->next != end && tmp->next->data != data)
	{
		tmp = tmp->next;
	}

	if (tmp->next == end)
	{
		return NULL;
	}

	if (tmp->next->data != data) {
		return NULL;
	}

	return tmp;
}

Node *node_find_index_before(Node *start, int index, Node *end)
{
	Node *tmp = start;
	int i = 0;

	while (tmp->next != end && i != index)
	{
		tmp = tmp->next;
		++i;
	}

	if (tmp->next == end)
	{
		return NULL;
	}

	return tmp;
}

/* Insert node after */
Node *node_insert_after(Node *after, Node *node)
{
	node->next = after->next;
	after->next = node;

	return node;
}

/* Deletes node from linked list, doesn't free the node */
Node *node_delete(Node *before, Node *node)
{
	before->next = node->next;
	
	return node;
}


typedef struct LinkedList
{
	Node *head;
	Node *z;
} LinkedList;

/* Create linked list */
void ll_create(LinkedList *ll)
{
	Node *head = node_create(0, NULL);
	Node *z = node_create(0, NULL);

	head->next = z;
	z->next = z;

	ll->head = head;
	ll->z = z;
}

/* Returns last node before z */
Node *ll_last_node(LinkedList *ll)
{
	Node *tmp = ll->head;

	while (tmp->next != ll->z)
	{
		tmp = tmp->next;
	}

	return tmp;
}

Node *ll_first_node(LinkedList *ll)
{
	return ll->head->next;
}

/* Insert as beginning */
Node *ll_prepend(LinkedList *ll, Node *node)
{
	return node_insert_after(ll->head, node);
}

/* Insert as end.
 * Note: You should only use this once, then switch to node_insert_after
 * so you don't go through the whole list again trying to find the last node.
 * You can save the last node by setting the return value to a variable
 * (perhaps calling it current or last). A good example of this is shown
 * in the 'linked_list_ex3' function.
 */
Node *ll_append(LinkedList *ll, Node *node)
{
	Node *last = ll_last_node(ll);
	return node_insert_after(last, node);
}

/* Finds first node with given data */
Node *ll_find(LinkedList *ll, int data)
{
	return node_find(ll->head, data, ll->z);
}

Node *ll_find_index(LinkedList *ll, int index)
{
	return node_find_index(ll->head, index, ll->z);
}

/* Returns node before the first node with the given data */
Node *ll_find_before(LinkedList *ll, int data)
{
	return node_find_before(ll->head, data, ll->z);
}

Node *ll_find_index_before(LinkedList *ll, int index)
{
	return node_find_index_before(ll->head, index, ll->z);
}

Node *ll_find_delete(LinkedList *ll, int data)
{
	Node *before = ll_find_before(ll, data);
	if (before == ll->head || before->next == ll->z) return NULL;
	return node_delete(before, before->next);
}

int ll_is_empty(LinkedList *ll)
{
	return ll->head == ll->z;
}

int ll_length(LinkedList *ll)
{
	Node *tmp = ll->head;
	int length = 0;

	while (tmp != ll->z)
	{
		tmp = tmp->next;
		++length;
	}

	return length;
}

void linked_list_ex1(void)
{
	Node *head = node_create(0, NULL);
	Node *z = node_create(0, NULL);

	head->next = z;
	z->next = z;

	Node *n1 = node_create(3, NULL);
	Node *n2 = node_create(2, NULL);
	Node *n3 = node_create(4, NULL);
	Node *n4 = node_create(0, NULL);
	Node *n5 = node_create(1, NULL);

	node_insert_after(head, n1);
	node_insert_after(n1, n2);
	node_insert_after(n2, n3);
	node_insert_after(n3, n4);
	node_insert_after(n4, n5);

	Node *i1 = node_insert_after(node_find(head, 4, z), node_create(4, NULL));

	Node *tmp = head;

	while (tmp != z)
	{
		if (tmp == head)
		{
			tmp = tmp->next;
			continue;
		}

		printf("%d ", tmp->data);
		tmp = tmp->next;
	}

	printf("\n");
}

void linked_list_ex2(void)
{
	LinkedList ll = { NULL, NULL };
	ll_create(&ll);

	Node *current = ll_prepend(&ll, node_create(3, NULL));
	current = node_insert_after(current, node_create(2, NULL));
	current = node_insert_after(current, node_create(4, NULL));
	current = node_insert_after(current, node_create(0, NULL));
	current = node_insert_after(current, node_create(1, NULL));

	Node *i1 = node_insert_after(ll_find(&ll, 4), node_create(4, NULL));

	/* Demonstracte that ll_find only finds *first* node with given data */
	Node *i2 = node_insert_after(ll_find(&ll, 4), node_create(9, NULL));

	/* Demonstrates how to delete stuff */
	Node *before = ll_find_before(&ll, 0); /* Find the node before the node you want to delete */
	Node *d1 = node_delete(before, before->next); /* Delete the node (before->next) from the list */
	free(d1); /* Free the node */

	/* The above could also be done like this:
	 * Node *d1 = ll_find_delete(&ll, 0);
	 * free(d1);
	 */

	Node *tmp = ll.head;
	while (tmp != ll.z)
	{
		if (tmp == ll.head)
		{
			tmp = tmp->next;
			continue;
		}

		printf("%d ", tmp->data);
		tmp = tmp->next;
	}

	printf("\n");
}

void linked_list_ex3(void)
{
	char c;
	LinkedList ll = { NULL, NULL };
	ll_create(&ll);

	Node *current = ll.head;
	while ((c = getchar()) != EOF)
	{
		current = node_insert_after(current, node_create((int) c, NULL));
	}

	printf("\n");

	Node *tmp = ll.head;
	while (tmp != ll.z)
	{
		if (tmp == ll.head)
		{
			tmp = tmp->next;
			continue;
		}

		printf("%c", (char) tmp->data);
		tmp = tmp->next;
	}

	printf("\nLength: %d\n", ll_length(&ll));
}

/* TODO: ll_reverse() and ll_clone() functions for linked list. */

int main(int argc, char *argv[])
{
	linked_list_ex1();
	linked_list_ex2();

	linked_list_ex3();

	return(0);
}
