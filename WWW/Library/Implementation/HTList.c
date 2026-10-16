/*
 * $LynxId: HTList.c,v 1.24 2026/10/15 22:48:58 tom Exp $
 *
 *	A small List class					      HTList.c
 *	==================
 *
 *	A list is represented as a sequence of linked nodes of type HTList.
 *	The first node is a header which contains no object.
 *	New nodes are inserted between the header and the rest of the list.
 */

#include <HTUtils.h>
#include <HTList.h>

#include <LYLeaks.h>

/*	Create list.
*/
HTList *HTList_new(void)
{
    HTList *newList;

    if ((newList = typeMalloc(HTList)) == NULL)
	  outofmem(__FILE__, "HTList_new");

    newList->object = NULL;
    newList->next = NULL;

    return newList;
}

/*	Delete list.
*/
void HTList_delete(HTList *me)
{
    HTList *current;

    while ((current = me)) {
	me = me->next;
	FREE(current);
    }

    return;
}

/*	Reverse order of elements in list.
 */
HTList *HTList_reverse(HTList *start)
{
    HTList *cur, *succ;

    if (!(start && start->next && (cur = start->next->next)))
	return start;
    start->next->next = NULL;
    while (cur) {
	succ = cur->next;
	cur->next = start->next;
	start->next = cur;
	cur = succ;
    }
    return start;
}

/*	Append a list to another.
 *
 *	If successful, the second list will become empty but not freed.
 */
HTList *HTList_appendList(HTList *start,
			  HTList *tail)
{
    HTList *temp = start;

    if (!start) {
	CTRACE((tfp,
		"HTList: Trying to append list %p to a nonexisting list\n",
		(void *) tail));
	return NULL;
    }
    if (!(tail && tail->next))
	return start;

    while (temp->next)
	temp = temp->next;

    temp->next = tail->next;
    tail->next = NULL;		/* tail is now an empty list */
    return start;
}

/*	Link object to START of list (so it is pointed to by the head).
 *
 *	Unlike HTList_addObject(), it does not malloc memory for HTList entry,
 *	it use already allocated memory which should not be free'd by any
 *	list operations (optimization).
 */
void HTList_linkObject(HTList *me, void *newObject,
		       HTList *newNode)
{
    if (me) {
	if (newNode->object == NULL && newNode->next == NULL) {
	    /*  It is safe: */
	    newNode->object = newObject;
	    newNode->next = me->next;
	    me->next = newNode;

	} else {
	    /*
	     * This node is already linked to some list (probably this one), so
	     * refuse changing node pointers to keep the list valid!!!
	     */
	    CTRACE((tfp, "*** HTList: Refuse linking already linked obj "));
	    CTRACE((tfp, "%p, node %p, list %p\n",
		    (void *) newObject, (void *) newNode, (void *) me));
	}

    } else {
	CTRACE((tfp,
		"HTList: Trying to link object %p to a nonexisting list\n",
		newObject));
    }

    return;
}

/*      Add object to START of list (so it is pointed to by the head).
*/
void HTList_addObject(HTList *me, void *newObject)
{
    HTList *newNode;

    if (me) {
	if ((newNode = typeMalloc(HTList)) == NULL)
	      outofmem(__FILE__, "HTList_addObject");

	newNode->object = newObject;
	newNode->next = me->next;
	me->next = newNode;

    } else {
	CTRACE((tfp, "HTList: Trying to add object %p to a nonexisting list\n",
		newObject));
    }

    return;
}

/*      Append object to END of list (furthest from the head).
*/
void HTList_appendObject(HTList *me, void *newObject)
{
    HTList *temp = me;

    if (temp && newObject) {
	while (temp->next)
	    temp = temp->next;
	HTList_addObject(temp, newObject);
    }

    return;
}

/*	Insert an object into the list at a specified position.
 *      If position is 0, this places the object at the head of the list
 *      and is equivalent to HTList_addObject().
 */
void HTList_insertObjectAt(HTList *me, void *newObject,
			   int pos)
{
    HTList *newNode;
    HTList *temp = me;
    HTList *prevNode;
    int Pos = pos;

    if (!temp) {
	CTRACE((tfp, "HTList: Trying to add object %p to a nonexisting list\n",
		newObject));
	return;
    }
    if (Pos < 0) {
	Pos = 0;
	CTRACE((tfp, "HTList: Treating negative object position %d as %d.\n",
		pos, Pos));
    }

    prevNode = temp;
    while ((temp = temp->next)) {
	if (Pos == 0) {
	    if ((newNode = typeMalloc(HTList)) == NULL)
		  outofmem(__FILE__, "HTList_addObjectAt");

	    newNode->object = newObject;
	    newNode->next = temp;
	    if (prevNode)
		prevNode->next = newNode;
	    return;
	}
	prevNode = temp;
	Pos--;
    }
    if (Pos >= 0)
	HTList_addObject(prevNode, newObject);

    return;
}

/*	Unlink specified object from list.
 *	It does not free memory.
 */
BOOL HTList_unlinkObject(HTList *me, const void *oldObject)
{
    HTList *temp = me;

    if (temp && oldObject) {
	while (temp->next) {
	    HTList *prevNode = temp;

	    temp = temp->next;
	    if (temp->object == oldObject) {
		prevNode->next = temp->next;
		temp->next = NULL;
		temp->object = NULL;
		return YES;	/* Success */
	    }
	}
    }
    return NO;			/* object not found or NULL list */
}

/*	Remove specified object from list.
*/
BOOL HTList_removeObject(HTList *me, const void *oldObject)
{
    HTList *temp = me;

    if (temp && oldObject) {
	while (temp->next) {
	    HTList *prevNode = temp;

	    temp = temp->next;
	    if (temp->object == oldObject) {
		prevNode->next = temp->next;
		FREE(temp);
		return YES;	/* Success */
	    }
	}
    }
    return NO;			/* object not found or NULL list */
}

/*	Remove object at a given position in the list, where 0 is the
 *	object pointed to by the head (returns a pointer to the element
 *	(->object) for the object, and NULL if the list is empty, or
 *	if it doesn't exist - Yuk!).
 */
void *HTList_removeObjectAt(HTList *me, int position)
{
    HTList *temp = me;
    int pos = position;
    void *result = NULL;

    if (temp != NULL && pos >= 0) {
	HTList *prevNode = temp;

	while ((temp = temp->next) != NULL) {
	    if (pos == 0) {
		prevNode->next = temp->next;
		result = temp->object;
		FREE(temp);
		break;
	    }
	    prevNode = temp;
	    pos--;
	}
    }

    return result;
}

/*	Unlink object from START of list (the Last one inserted
 *	via HTList_linkObject(), and pointed to by the head).
 *	It does not free memory.
 */
void *HTList_unlinkLastObject(HTList *me)
{
    if (me && me->next) {
	HTList *lastNode = me->next;
	void *lastObject = lastNode->object;

	me->next = lastNode->next;
	lastNode->next = NULL;
	lastNode->object = NULL;
	return lastObject;

    } else {			/* Empty list */
	return NULL;
    }
}

/*	Remove object from START of list (the Last one inserted
 *	via HTList_addObject(), and pointed to by the head).
 */
void *HTList_removeLastObject(HTList *me)
{
    if (me && me->next) {
	HTList *lastNode = me->next;
	void *lastObject = lastNode->object;

	me->next = lastNode->next;
	FREE(lastNode);
	return lastObject;

    } else {			/* Empty list */
	return NULL;
    }
}

/*	Remove object from END of list (the First one inserted
 *	via HTList_addObject(), and furthest from the head).
 */
void *HTList_removeFirstObject(HTList *me)
{
    void *firstObject = NULL;

    if (me != NULL && me->next != NULL) {
	HTList *prevNode = me;

	while (me->next) {
	    prevNode = me;
	    me = me->next;
	}
	firstObject = me->object;
	prevNode->next = NULL;
	free(me);
    }
    return firstObject;
}

/*	Determine total number of objects in the list,
 *	not counting the head.
 */
int HTList_count(const HTList *me)
{
    const HTList *temp = me;
    int count = 0;

    if (temp)
	while ((temp = temp->next))
	    count++;

    return count;
}

/*	Determine position of an object in the list (a value of 0
 *	means it is pointed to by the head; returns -1 if not found).
 */
int HTList_indexOf(const HTList *me, const void *object)
{
    const HTList *temp = me;

    if (temp) {
	int position = 0;

	while ((temp = temp->next)) {
	    if (temp->object == object)
		return position;
	    position++;
	}
    }

    return -1;			/* Object not in the list */
}

/*	Return pointer to the object at a specified position in the list,
 *	where 0 is the object pointed to by the head (returns NULL if
 *	the list is empty, or if it doesn't exist - Yuk!).
 */
void *HTList_objectAt(const HTList *me, int position)
{
    const HTList *temp = me;
    int pos = position;

    if (!temp || pos < 0)
	return NULL;

    while ((temp = temp->next)) {
	if (pos == 0)
	    return temp->object;
	pos--;
    }

    return NULL;		/* Reached the end of the list */
}
