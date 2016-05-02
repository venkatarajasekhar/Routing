#include "lists.h"
static int init=0;
void list_init()
{
  timeout_list_head=NULL;
  timeout_list_tail=NULL;
}
void list_push(timer_elem *node)
{
  if(timeout_list_head==NULL)// && timeout_list_head == timeout_list_tail)
  {
    init = 1;
    timeout_list_head = node;
    timeout_list_tail = node;
    node->next = NULL;
    node->prev = NULL;
  }
  else
  {
    timeout_list_tail->next = node;
    node->prev = timeout_list_tail;
    node->next = NULL;
    timeout_list_tail = node;
  }
}
void list_insert_ordered(timer_elem *node)
{
  timer_elem *a;
  for(a = timeout_list_head; a!=NULL; a=a->next)
  {
    if(timercmp(&(a->timeout), &(node->timeout), >))
    {
      if(a==timeout_list_head)
      {
        timeout_list_head=node;
        node->next = a;
        a->prev = node;
      }
      else
      {
        (a->prev)->next = node;
        node->prev = (a->prev);
        node->next = a;
        a->prev = node;
      }
      return;
    }
  }
  node->prev = timeout_list_tail;
  timeout_list_tail->next = node;
  node->next = NULL;
  timeout_list_tail = node;
}
timer_elem* list_pop()
{
  if(timeout_list_head==NULL)
  {
    return NULL;
  }
  else
  {
    timer_elem* node = timeout_list_head;
    if(timeout_list_head->next == NULL)
    {
      timeout_list_head = NULL;
      timeout_list_tail = NULL;
      init=0;
    }
    else
    {
      timeout_list_head = timeout_list_head->next;
      timeout_list_head->prev = NULL;
    }
    return node;
  }
}
timer_elem* list_peek()
{
  if(timeout_list_head==NULL)
    return NULL;
  return timeout_list_head;
}
void list_remove(timer_elem *temp)
{
  if(temp==NULL)
    return;
  if(temp==timeout_list_head)
  {
    list_pop();
  }
  else if(temp==timeout_list_tail)
  {
    timer_elem *prev = temp->prev;
    temp->prev = NULL;
    temp->next = NULL;
    prev->next = NULL;
    timeout_list_tail = prev;
  }
}
timer_elem* find_timeout_by_ip(uint32_t ip)
{
  if(timeout_list_head==NULL)
    return NULL;
  timer_elem *temp=timeout_list_head;
  while(temp!=NULL)
  {
    if(temp->ip == ip)
      return temp;
    temp=temp->next;
  }
  return NULL;
}
void update_start()
{
  if(init)
    return;
  list_init();
  timer_elem* updates = malloc(sizeof(timer_elem));
  memset(updates, 0, sizeof(timer_elem));
  updates->update = true;
  updates->failures = 0;
  gettimeofday(&(updates->timeout), NULL);
  updates->timeout.tv_sec+= timeout;
  //TODO add timeout
  list_push(updates);
}
void print_router_list()
{
  char IP[INET_ADDRSTRLEN]; 
  for(int i=0; i<router_count; i++)
  {
    ip_readable(router_list[i].ip, IP);
    LOG("Router: %d: port %d %d\n Cost:%d IP:%s\n Neighbour %d\n",
        router_list[i].id, router_list[i].port_routing, router_list[i].port_data, 
        router_list[i].cost, IP, router_list[i].neighbour);
  }
}
struct timeval get_next_timeout()
{
  struct timeval null;
  memset(&null, 0, sizeof(struct timeval));
  if(!init)
  {
    //return NULL
    LOG("Not init\n");
    return null;
  }
  timer_elem *check;
  check = list_peek();
  if(check == NULL)
  {
    //return NULL or something
    return null;
  }
  return check->timeout;
}
struct timeval update_timeout()
{
  if(list_peek()!=NULL)
  {
    timer_elem *update = list_pop();
    update->timeout.tv_sec += timeout;
//    list_push(update);
    list_insert_ordered(update);
  }
  return get_next_timeout();
}

file_stats* find_file_transfer_id(uint8_t tfer_id)
{
  file_stats *res;
  if(stats_list==NULL)
    return stats_list;
  for(res = stats_list; res!=NULL; res=res->next)
  {
    if(res->transfer_id == tfer_id)
      break;
  }
  return res;
}
void insert_file(file_stats *new)
{
  if(stats_list==NULL)
  {
    stats_list = new;
    new->next=NULL;
    return;
  }
  new->next = stats_list;
  stats_list = new;
}
