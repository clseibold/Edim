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

Node *append(Node *head, int data)
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

Node *insert_after(Node *head, Node *prev, int data) {
	Node *cursor = head;
	while (cursor != prev) {
		cursor = cursor->next;
	}

	if (cursor != NULL) {
		Node *newNode = create(data, cursor->next);
		cursor->next = newNode;
		return head;
	} else {
		return NULL;
	}
}

Node *insert_before(Node *head, Node *next, int data) {
	if (head == NULL)
		return NULL;

	if (next == NULL) {
		head = append(head, data);
		return head;
	}

	if (head == next) {
		head = prepend(head, data);
		return head;
	}

	Node *cursor = head;
	while (cursor != next - 1) {
		cursor = cursor->next;
	}

	if (cursor != NULL) {
		Node *newNode = create(data, cursor->next);
		cursor->next = newNode;
		return head;
	} else {
		return NULL;
	}
}

Node *search(Node *head, int data) {
	Node *cursor = head;
	while (cursor != NULL) {
		if (cursor->data = data)
			return cursor;
		cursor = cursor->next;
	}
	return NULL;
}

Node *reverse(Node *head) { // TODO: I don't understand this!
	Node *prev = NULL;
	Node *current = head;
	Node *next;

	while (current != NULL) {
		next = current->next;
		current->next = prev;
		prev = current;
		current = next;
	}

	return NULL;
}

Node *remove_front(Node *head) {
	if (head == NULL)
		return NULL;
	
	Node *front = head;
	head = head->next;
	front->next = NULL;

	/* Is this the last node in the list? */
	if (front == head) head = NULL; // TODO: this doesn't make any sense to me

	free(front);
	return head;
}

Node *remove_back(Node *head) {
	
}

int main(int argc, char *argv[])
{
	Node *head = NULL;
	Node *last = head;
}