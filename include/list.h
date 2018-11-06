#ifndef LIST_H
#define LIST_H

#ifndef NULL
#define NULL 0
#endif

typedef struct list_head {
    struct list_head *prev, *next;
} list_head;


#define INIT_LIST_HEAD(ptr) do {\
  struct list_head *_ptr = (struct list_head *) ptr; \
  (_ptr)->next = (_ptr); (_ptr->prev) = (_ptr);    \
}while(0)

#define offsetof(TYPE, MEMBER) ((size_t) & ((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ({    \
  const typeof(((type *)0)->member) *__mptr = (ptr);  \
  (type *)((char *)__mptr - offsetof(type, member));  \
})

#define list_entry(ptr, type, member) \
  container_of(ptr, type, member)

#define list_for_each(pos, head)  \
  for(pos = (head)->next; pos != (head); pos = pos->next)

static inline void __list_del(struct list_head *prev, struct list_head *next) {
    prev->next = next;
    next->prev = prev;
}

static inline void list_del(struct list_head *entry) {
    __list_del(entry->prev, entry->next);
}

static inline void __list_add(struct list_head *_new, struct list_head *prev, struct list_head *next) {
    _new->next = next;
    next->prev = _new;
    prev->next = _new;
    _new->prev = prev;
}

static inline void list_add(struct list_head *_new, struct list_head *head) {
    __list_add(_new, head, head->next);
}


#endif // ! LIST_H