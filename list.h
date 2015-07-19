/*
 * list.h
 *
 *  Created on: 2015Äê7ÔÂ17ÈÕ
 *      Author: Administrator
 */

#ifndef LIST_H_
#define LIST_H_

#include <stddef.h>

struct list_head {
    struct list_head *next;
    struct list_head *prev;
};

#define LIST_HEAD_INIT(name) {&(name), &(name)}

#define LIST_HEAD(name) \
    struct list_head name = LIST_HEAD_INIT(name)

extern void INIT_LIST_HEAD(struct list_head *list);

extern void list_add_head(struct list_head *n, struct list_head *head);

extern void list_add_tail(struct list_head *n, struct list_head *head);

extern void list_del(struct list_head *p, struct list_head *n);

#include <stddef.h>

#define container_of(ptr, type, member) ({ \
        (type *)((char *)(ptr) - offsetof(type,member));})

/**
 *  * list_entry - get the struct for this entry
 *   * @ptr:    the &struct list_head pointer.
 *    * @type:  the type of the struct this is embedded in.
 *     * @member:   the name of the list_head within the struct.
 *      */
#define list_entry(ptr, type, member) \
        container_of(ptr, type, member)

/**
 *  * list_first_entry - get the first element from a list
 *   * @ptr:    the list head to take the element from.
 *    * @type:  the type of the struct this is embedded in.
 *     * @member:   the name of the list_head within the struct.
 *      *
 *       * Note, that list is expected to be not empty.
 *        */
#define list_first_entry(ptr, type, member) \
        list_entry((ptr)->next, type, member)

/**
 *  * list_last_entry - get the last element from a list
 *   * @ptr:    the list head to take the element from.
 *    * @type:  the type of the struct this is embedded in.
 *     * @member:   the name of the list_head within the struct.
 *      *
 *       * Note, that list is expected to be not empty.
 *        */
#define list_last_entry(ptr, type, member) \
        list_entry((ptr)->prev, type, member)

/**
 *  * list_first_entry_or_null - get the first element from a list
 *   * @ptr:    the list head to take the element from.
 *    * @type:  the type of the struct this is embedded in.
 *     * @member:   the name of the list_head within the struct.
 *      *
 *       * Note that if the list is empty, it returns NULL.
 *        */
#define list_first_entry_or_null(ptr, type, member) \
        (!list_empty(ptr) ? list_first_entry(ptr, type, member) : NULL)

/**
 *  * list_next_entry - get the next element in list
 *   * @pos:    the type * to cursor
 *    * @member:    the name of the list_head within the struct.
 *     */
#define list_next_entry(pos, member) \
        list_entry((pos)->member.next, typeof(*(pos)), member)

/**
 *  * list_prev_entry - get the prev element in list
 *   * @pos:    the type * to cursor
 *    * @member:    the name of the list_head within the struct.
 *     */
#define list_prev_entry(pos, member) \
        list_entry((pos)->member.prev, typeof(*(pos)), member)

/**
 *  * list_for_each -   iterate over a list
 *   * @pos:    the &struct list_head to use as a loop cursor.
 *    * @head:  the head for your list.
 *     */
#define list_for_each(pos, head) \
        for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 *  * list_for_each_prev    -   iterate over a list backwards
 *   * @pos:    the &struct list_head to use as a loop cursor.
 *    * @head:  the head for your list.
 *     */
#define list_for_each_prev(pos, head) \
        for (pos = (head)->prev; pos != (head); pos = pos->prev)

/**
 *  * list_for_each_safe - iterate over a list safe against removal of list entry
 *   * @pos:    the &struct list_head to use as a loop cursor.
 *    * @n:     another &struct list_head to use as temporary storage
 *     * @head: the head for your list.
 *      */
#define list_for_each_safe(pos, n, head) \
        for (pos = (head)->next, n = pos->next; pos != (head); \
                        pos = n, n = pos->next)

/**
 *  * list_for_each_prev_safe - iterate over a list backwards safe against removal of list entry
 *   * @pos:    the &struct list_head to use as a loop cursor.
 *    * @n:     another &struct list_head to use as temporary storage
 *     * @head: the head for your list.
 *      */
#define list_for_each_prev_safe(pos, n, head) \
        for (pos = (head)->prev, n = pos->prev; \
                         pos != (head); \
                         pos = n, n = pos->prev)

/**
 *  * list_for_each_entry   -   iterate over list of given type
 *   * @pos:    the type * to use as a loop cursor.
 *    * @head:  the head for your list.
 *     * @member:   the name of the list_head within the struct.
 *      */
#define list_for_each_entry(pos, head, member)              \
        for (pos = list_first_entry(head, typeof(*pos), member);    \
                         &pos->member != (head);                    \
                         pos = list_next_entry(pos, member))

/**
 *  * list_for_each_entry_reverse - iterate backwards over list of given type.
 *   * @pos:    the type * to use as a loop cursor.
 *    * @head:  the head for your list.
 *     * @member:   the name of the list_head within the struct.
 *      */
#define list_for_each_entry_reverse(pos, head, member)          \
        for (pos = list_last_entry(head, typeof(*pos), member);     \
                         &pos->member != (head);                    \
                         pos = list_prev_entry(pos, member))

/**
 *  * list_prepare_entry - prepare a pos entry for use in list_for_each_entry_continue()
 *   * @pos:    the type * to use as a start point
 *    * @head:  the head of the list
 *     * @member:   the name of the list_head within the struct.
 *      *
 *       * Prepares a pos entry for use as a start point in list_for_each_entry_continue().
 *        */
#define list_prepare_entry(pos, head, member) \
        ((pos) ? : list_entry(head, typeof(*pos), member))

/**
 *  * list_for_each_entry_continue - continue iteration over list of given type
 *   * @pos:    the type * to use as a loop cursor.
 *    * @head:  the head for your list.
 *     * @member:   the name of the list_head within the struct.
 *      *
 *       * Continue to iterate over list of given type, continuing after
 *        * the current position.
 *         */
#define list_for_each_entry_continue(pos, head, member)         \
        for (pos = list_next_entry(pos, member);            \
                         &pos->member != (head);                    \
                         pos = list_next_entry(pos, member))

/**
 *  * list_for_each_entry_continue_reverse - iterate backwards from the given point
 *   * @pos:    the type * to use as a loop cursor.
 *    * @head:  the head for your list.
 *     * @member:   the name of the list_head within the struct.
 *      *
 *       * Start to iterate over list of given type backwards, continuing after
 *        * the current position.
 *         */
#define list_for_each_entry_continue_reverse(pos, head, member)     \
        for (pos = list_prev_entry(pos, member);            \
                         &pos->member != (head);                    \
                         pos = list_prev_entry(pos, member))

/**
 *  * list_for_each_entry_from - iterate over list of given type from the current point
 *   * @pos:    the type * to use as a loop cursor.
 *    * @head:  the head for your list.
 *     * @member:   the name of the list_head within the struct.
 *      *
 *       * Iterate over list of given type, continuing from current position.
 *        */
#define list_for_each_entry_from(pos, head, member)             \
        for (; &pos->member != (head);                  \
                         pos = list_next_entry(pos, member))

/**
 *  * list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 *   * @pos:    the type * to use as a loop cursor.
 *    * @n:     another type * to use as temporary storage
 *     * @head: the head for your list.
 *      * @member:  the name of the list_head within the struct.
 *       */
#define list_for_each_entry_safe(pos, n, head, member)          \
        for (pos = list_first_entry(head, typeof(*pos), member),    \
                        n = list_next_entry(pos, member);           \
                         &pos->member != (head);                    \
                         pos = n, n = list_next_entry(n, member))

/**
 *  * list_for_each_entry_safe_continue - continue list iteration safe against removal
 *   * @pos:    the type * to use as a loop cursor.
 *    * @n:     another type * to use as temporary storage
 *     * @head: the head for your list.
 *      * @member:  the name of the list_head within the struct.
 *       *
 *        * Iterate over list of given type, continuing after current point,
 *         * safe against removal of list entry.
 *          */
#define list_for_each_entry_safe_continue(pos, n, head, member)         \
        for (pos = list_next_entry(pos, member),                \
                        n = list_next_entry(pos, member);               \
                         &pos->member != (head);                        \
                         pos = n, n = list_next_entry(n, member))

/**
 *  * list_for_each_entry_safe_from - iterate over list from current point safe against removal
 *   * @pos:    the type * to use as a loop cursor.
 *    * @n:     another type * to use as temporary storage
 *     * @head: the head for your list.
 *      * @member:  the name of the list_head within the struct.
 *       *
 *        * Iterate over list of given type from current point, safe against
 *         * removal of list entry.
 *          */
#define list_for_each_entry_safe_from(pos, n, head, member)             \
        for (n = list_next_entry(pos, member);                  \
                         &pos->member != (head);                        \
                         pos = n, n = list_next_entry(n, member))

/**
 *  * list_for_each_entry_safe_reverse - iterate backwards over list safe against removal
 *   * @pos:    the type * to use as a loop cursor.
 *    * @n:     another type * to use as temporary storage
 *     * @head: the head for your list.
 *      * @member:  the name of the list_head within the struct.
 *       *
 *        * Iterate backwards over list of given type, safe against removal
 *         * of list entry.
 *          */
#define list_for_each_entry_safe_reverse(pos, n, head, member)      \
        for (pos = list_last_entry(head, typeof(*pos), member),     \
                        n = list_prev_entry(pos, member);           \
                         &pos->member != (head);                    \
                         pos = n, n = list_prev_entry(n, member))

/**
 *  * list_safe_reset_next - reset a stale list_for_each_entry_safe loop
 *   * @pos:    the loop cursor used in the list_for_each_entry_safe loop
 *    * @n:     temporary storage used in list_for_each_entry_safe
 *     * @member:   the name of the list_head within the struct.
 *      *
 *       * list_safe_reset_next is not safe to use in general if the list may be
 *        * modified concurrently (eg. the lock is dropped in the loop body). An
 *         * exception to this is if the cursor element (pos) is pinned in the list,
 *          * and list_safe_reset_next is called after re-taking the lock and before
 *           * completing the current iteration of the loop body.
 *            */
#define list_safe_reset_next(pos, n, member)                \
        n = list_next_entry(pos, member)

#endif /* LIST_H_ */
