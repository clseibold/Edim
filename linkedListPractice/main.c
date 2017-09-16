#include <stdio.h>

typedef struct Node {
	int data;
	Node *next;
} Node;

typedef void (*callback)(Node *data);

Node *create(int data, Node *next)
{
	Node *newNode = (Node *) malloc(sizeof(Node));
	if (newNode == NULL) {
		fprintf(stderr, "Error creating new node");
		exit(1);
	}

	newNode->data = data;
	newNode->next = next;

	return newNode;
}

Node *prepend(Node *head, int data)
{
	Node *newNode = create(data, head);
	head = newNode;
	return head;
}

void traverse(Node *head, callback f)
{
	Node *cursor = head;
	while (cursor != NULL)
	{
		f(cursor);
		cursor = cursor->next;
	}
}

int count(Node *head)
{
	Node *cursor = head;
	int c = 0;
	while(cursor != NULL)
	{
		++c;
		cursor = cursor->next;
	}

	return count;
}

Node *append(Node *head, data)
{
	/* Go to last node */
	Node *cursor = head;
	while (cursor->next != NULL)
	{
		cursor = cursor->next;
	}

	/* Create new node */
	Node *newNode = create(data, NULL);
	cursor->next = newNode;

	return newNode;
}

int main(int argc, char *argv[])
{
	Node *head = NULL;

	Node *last = head;

	append(last, 13);
}