#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

const int maxlen = 1000;

typedef struct node Node;

struct node 
{
  char *str;
  Node *next;
};


Node *push_front(List *list, Node *begin, const char *str)
{

  Node *p = (Node *)malloc(sizeof(Node));
  char *s = (char *)malloc(strlen(str) + 1);
  strcpy(s, str);

  *p = (Node){.str = s , .next = list->begin};
list->begin = p;
  return List; 
}


Node *pop_front(Node *begin)
{
  assert(begin != NULL);
  Node *p = begin->next;

  free(begin->str);
  free(begin);

  return p;
}

Node *push_back(Node *begin, const char *str)
{
  if (begin == NULL) { 
    return push_front(begin, str);
  }

  Node *p = begin;
  while (p->next != NULL) {
    p = p->next;
  }

  Node *q = (Node *)malloc(sizeof(Node));
  char *s = (char *)malloc(strlen(str) + 1);
  strcpy(s, str);

  *q = (Node){.str = s, .next = NULL};
  p->next = q;

  return begin;
}

// 実習1: pop_back の実装
Node *pop_back(Node *begin)
{
  
  Node *q = begin; 
  Node *p = begin;
  while(q->next != NULL){
  
      p = q;
      q = q->next;
      break;
    }
  //p is the last pointer

  free(q);
  
  free(q->str);//最後の構造体へのポインタを開放
  
  


  // 頑張って実装する
  return begin;
}


Node *remove_all(Node *begin)
{
  while ((begin = pop_front(begin))) 
    ; // Repeat pop_front() until the list becomes empty
  return begin;
}

int main()
{
  Node *begin = NULL; 


  char buf[maxlen];
  while (fgets(buf, maxlen, stdin)) {
    begin = pop_back(begin);
    //begin = push_back(begin, buf); // Try this instead of push_front()
  }

  //begin = pop_front(begin);  // What will happen if you do this?
  //begin = pop_back(begin);   // What will happen if you do this?
 
  //begin = remove_all(begin); // What will happen if you do this?

  for (const Node *p = begin; p != NULL; p = p->next) {
    printf("%s", p->str);
  }
  
  return EXIT_SUCCESS;
}
