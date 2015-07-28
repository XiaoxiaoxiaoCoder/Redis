/* adlist.c - A generic doubly linked list implementation
 *
 * Copyright (c) 2006-2010, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */


#include <stdlib.h>
#include "adlist.h"
#include "zmalloc.h"

/* Create a new list. The created list can be freed with
 * AlFreeList(), but private value of every node need to be freed
 * by the user before to call AlFreeList().
 *
 * On error, NULL is returned. Otherwise the pointer to the new list. */
/* ----------------------------------------------------------------------*/
/**
 * @brief:listCreate    创建一个链表
 *
 * @return              成功返回链表指针，失败返回 NULL
 */
/* ----------------------------------------------------------------------*/
list *listCreate(void)
{
    struct list *list;

    if ((list = zmalloc(sizeof(*list))) == NULL)
        return NULL;
    list->head = list->tail = NULL;
    list->len = 0;
    list->dup = NULL;
    list->free = NULL;
    list->match = NULL;
    return list;
}

/* Free the whole list.
 *
 * This function can't fail. */
/* ----------------------------------------------------------------------*/
/**
 * @brief:listRelease   释放链表
 *
 * @param:list          释放的链表
 */
/* ----------------------------------------------------------------------*/
void listRelease(list *list)
{
    unsigned long len;
    listNode *current, *next;

    current = list->head;
    len = list->len;
    while(len--) {
        next = current->next;
        if (list->free) list->free(current->value);
        zfree(current);
        current = next;
    }
    zfree(list);
}

/* Add a new node to the list, to head, containing the specified 'value'
 * pointer as value.
 *
 * On error, NULL is returned and no operation is performed (i.e. the
 * list remains unaltered).
 * On success the 'list' pointer you pass to the function is returned. */
/* ----------------------------------------------------------------------*/
/**
 * @brief:listAddNodeHead   头插法添加一个节点至双链表中
 *
 * @param:list              待添加node的双链表
 * @param:value             添加的node中的 value
 *
 * @return                  成功返回添加后的list指针，失败返回NULL
 */
/* ----------------------------------------------------------------------*/
list *listAddNodeHead(list *list, void *value)
{
    listNode *node;

    if ((node = zmalloc(sizeof(*node))) == NULL)
        return NULL;
    node->value = value;
    if (list->len == 0) {
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    } else {
        node->prev = NULL;
        node->next = list->head;
        list->head->prev = node;
        list->head = node;
    }
    list->len++;
    return list;
}

/* Add a new node to the list, to tail, containing the specified 'value'
 * pointer as value.
 *
 * On error, NULL is returned and no operation is performed (i.e. the
 * list remains unaltered).
 * On success the 'list' pointer you pass to the function is returned. */
/* ----------------------------------------------------------------------*/
/**
 * @brief:listAddNodeHead   尾插法添加一个节点至双链表中
 *
 * @param:list              待添加node的双链表
 * @param:value             添加的node中的 value
 *
 * @return                  成功返回添加后的list指针，失败返回NULL
 */
/* ----------------------------------------------------------------------*/
list *listAddNodeTail(list *list, void *value)
{
    listNode *node;

    if ((node = zmalloc(sizeof(*node))) == NULL)
        return NULL;
    node->value = value;
    if (list->len == 0) {
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    } else {
        node->prev = list->tail;
        node->next = NULL;
        list->tail->next = node;
        list->tail = node;
    }
    list->len++;
    return list;
}

/* ----------------------------------------------------------------------*/
/**
 * @brief:listInsertNode    在链表的指定节点前或后插入新节点
 *
 * @param:list              链表指针
 * @param:old_node          指定插入前后的节点
 * @param:value             待插入节点的 value
 * @param:after             在指定插入节点前或后插入
 *
 * @return                  操作成功返回list指针，失败返回NULL
 */
/* ----------------------------------------------------------------------*/
list *listInsertNode(list *list, listNode *old_node, void *value, int after) {
    listNode *node;

    if ((node = zmalloc(sizeof(*node))) == NULL)
        return NULL;
    node->value = value;
    if (after) {
        node->prev = old_node;
        node->next = old_node->next;
        if (list->tail == old_node) {
            list->tail = node;
        }
    } else {
        node->next = old_node;
        node->prev = old_node->prev;
        if (list->head == old_node) {
            list->head = node;
        }
    }
    if (node->prev != NULL) {
        node->prev->next = node;
    }
    if (node->next != NULL) {
        node->next->prev = node;
    }
    list->len++;
    return list;
}

/* Remove the specified node from the specified list.
 * It's up to the caller to free the private value of the node.
 *
 * This function can't fail. */
/* ----------------------------------------------------------------------*/
/**
 * @brief:listDelNode   从链表中删除节点
 *
 * @param:list          待操作的链表
 * @param:node          删除的节点
 */
/* ----------------------------------------------------------------------*/
void listDelNode(list *list, listNode *node)
{
    if (node->prev)
        node->prev->next = node->next;
    else
        list->head = node->next;
    if (node->next)
        node->next->prev = node->prev;
    else
        list->tail = node->prev;
    if (list->free) list->free(node->value);
    zfree(node);
    list->len--;
}

/* Returns a list iterator 'iter'. After the initialization every
 * call to listNext() will return the next element of the list.
 *
 * This function can't fail. */
/* ----------------------------------------------------------------------*/
/**
 * @brief:listGetIterator   获取链表迭代器
 *
 * @param:list              迭代器所指向的链表
 * @param:direction         迭代器迭代的方向
 *
 * @return                  成功返回迭代器指针，失败返回NULL
 */
/* ----------------------------------------------------------------------*/
listIter *listGetIterator(list *list, int direction)
{
    listIter *iter;

    if ((iter = zmalloc(sizeof(*iter))) == NULL) return NULL;
    if (direction == AL_START_HEAD)
        iter->next = list->head;
    else
        iter->next = list->tail;
    iter->direction = direction;
    return iter;
}

/* Release the iterator memory */
/* ----------------------------------------------------------------------*/
/**
 * @brief:listReleaseIterator   释放迭代器
 *
 * @param:iter                  迭代器指针
 */
/* ----------------------------------------------------------------------*/
void listReleaseIterator(listIter *iter) {
    zfree(iter);
}

/* Create an iterator in the list private iterator structure */
/* ----------------------------------------------------------------------*/
/**
 * @brief:listRewind    正向重置迭代器，将迭代器设置为正向迭代，
 *                      并指向指定链表头结点
 *
 * @param:list          重置指向的链表
 * @param:li            待重置的迭代器
 */
/* ----------------------------------------------------------------------*/
void listRewind(list *list, listIter *li) {
    li->next = list->head;
    li->direction = AL_START_HEAD;
}

/* ----------------------------------------------------------------------*/
/**
 * @brief:listRewind    正向重置迭代器，将迭代器设置为正向迭代，
 *                      并指向指定链表头结点
 *
 * @param:list          重置指向的链表
 * @param:li            待重置的迭代器
 */
/* ----------------------------------------------------------------------*/
void listRewindTail(list *list, listIter *li) {
    li->next = list->tail;
    li->direction = AL_START_TAIL;
}

/* Return the next element of an iterator.
 * It's valid to remove the currently returned element using
 * listDelNode(), but not to remove other elements.
 *
 * The function returns a pointer to the next element of the list,
 * or NULL if there are no more elements, so the classical usage patter
 * is:
 *
 * iter = listGetIterator(list,<direction>);
 * while ((node = listNext(iter)) != NULL) {
 *     doSomethingWith(listNodeValue(node));
 * }
 *
 * */
/* ----------------------------------------------------------------------*/
/**
 * @brief:listNext      通过迭代器获取下一个节点
 *
 * @param:iter          迭代器指针
 *
 * @return              下一个node存在，返回node指针，否则返回 NULL
 */
/* ----------------------------------------------------------------------*/
listNode *listNext(listIter *iter)
{
    listNode *current = iter->next;

    if (current != NULL) {
        if (iter->direction == AL_START_HEAD)
            iter->next = current->next;
        else
            iter->next = current->prev;
    }
    return current;
}

/* Duplicate the whole list. On out of memory NULL is returned.
 * On success a copy of the original list is returned.
 *
 * The 'Dup' method set with listSetDupMethod() function is used
 * to copy the node value. Otherwise the same pointer value of
 * the original node is used as value of the copied node.
 *
 * The original list both on success or error is never modified. */
/* ----------------------------------------------------------------------*/
/**
 * @brief:listDup   复制一个链表,成功则获取一个完全相同的链表，失败什么都不操作
 *
 * @param:orig      待复制的链表
 *
 * @return          成功返回新list指针，失败返回NULL
 */
/* ----------------------------------------------------------------------*/
list *listDup(list *orig)
{
    list *copy;
    listIter *iter;
    listNode *node;

    if ((copy = listCreate()) == NULL)
        return NULL;
    copy->dup = orig->dup;
    copy->free = orig->free;
    copy->match = orig->match;
    iter = listGetIterator(orig, AL_START_HEAD);
    while((node = listNext(iter)) != NULL) {
        void *value;

        if (copy->dup) {
            value = copy->dup(node->value);
            if (value == NULL) {
                listRelease(copy);
                listReleaseIterator(iter);
                return NULL;
            }
        } else
            value = node->value;
        if (listAddNodeTail(copy, value) == NULL) {
            listRelease(copy);
            listReleaseIterator(iter);
            return NULL;
        }
    }
    listReleaseIterator(iter);
    return copy;
}

/* Search the list for a node matching a given key.
 * The match is performed using the 'match' method
 * set with listSetMatchMethod(). If no 'match' method
 * is set, the 'value' pointer of every node is directly
 * compared with the 'key' pointer.
 *
 * On success the first matching node pointer is returned
 * (search starts from head). If no matching node exists
 * NULL is returned. */
/* ----------------------------------------------------------------------*/
/**
 * @brief:listSearchKey     在链表中寻找指定的 key 的节点 node
 *
 * @param:list              待操作的链表
 * @param:key               寻找的 key
 *
 * @return                  成功返回匹配的 node，失败返回 NULL
 */
/* ----------------------------------------------------------------------*/
listNode *listSearchKey(list *list, void *key)
{
    listIter *iter;
    listNode *node;

    iter = listGetIterator(list, AL_START_HEAD);
    while((node = listNext(iter)) != NULL) {
        if (list->match) {
            if (list->match(node->value, key)) {
                listReleaseIterator(iter);
                return node;
            }
        } else {
            if (key == node->value) {
                listReleaseIterator(iter);
                return node;
            }
        }
    }
    listReleaseIterator(iter);
    return NULL;
}

/* Return the element at the specified zero-based index
 * where 0 is the head, 1 is the element next to head
 * and so on. Negative integers are used in order to count
 * from the tail, -1 is the last element, -2 the penultimate
 * and so on. If the index is out of range NULL is returned. */
/* ----------------------------------------------------------------------*/
/**
 * @brief:listIndex     返回指定 index 的node节点指针
 *
 * @param:list          待操作的 list
 * @param:index         指定的 index
 *
 * @return              成功返回 node 指针， 失败返回 NULL
 */
/* ----------------------------------------------------------------------*/
listNode *listIndex(list *list, long index) {
    listNode *n;

    if (index < 0) {
        index = (-index)-1;
        n = list->tail;
        while(index-- && n) n = n->prev;
    } else {
        n = list->head;
        while(index-- && n) n = n->next;
    }
    return n;
}

/* Rotate the list removing the tail node and inserting it to the head. */
/* ----------------------------------------------------------------------*/
/**
 * @brief:listRotate    将双链表尾节点移除至链表头节点前插入
 *
 * @param:list          待操作的双链表 list
 */
/* ----------------------------------------------------------------------*/
void listRotate(list *list) {
    listNode *tail = list->tail;

    if (listLength(list) <= 1) return;

    /* Detach current tail */
    list->tail = tail->prev;
    list->tail->next = NULL;
    /* Move it as head */
    list->head->prev = tail;
    tail->prev = NULL;
    tail->next = list->head;
    list->head = tail;
}
