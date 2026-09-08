#include <stdlib.h>
#include "version.h"
#include "link.h"
#include "buf.h"

/*
 *  リンクの末尾につける (在链表末尾添加)
 * 引数 (参数)
 *      top         トップノード (头节点)
 *      add         付け加えるノードの値を保存するノードへのポインター (指向保存待添加节点值的节点的指针)
 * 返り値 (返回值)
 *      TRUE(1)     真 (真)
 *      FALSE(0)    偽 (偽)
 */
BOOL Nodeappendtail(Node **top, Node *add)
{
	Node *c;	/*ループ用 (循环用)*/
	Node *next; /*新しく作るノード用 (用于新建节点)*/

	/*トップがNULLかどうかのチェック (检查头节点是否为NULL)*/
	if (*top == NULL) {
		*top = allocateMemory(sizeof(Node));
		if (*top == NULL)
			return FALSE;
		(*top)->next = NULL;	  /*終わりはない (无末尾)*/
		(*top)->size = add->size; /*サイズの保存 (保存大小)*/
		(*top)->val = add->val;	  /*ポインターのコピー (指针的复制)*/
		return TRUE;
	}

	for (c = *top; c->next; c = c->next)
		; /* cの末尾まで辿る (遍历到c的末尾) */
	next = allocateMemory(sizeof(Node));
	if (next == NULL)
		return FALSE;
	c->next = next;			/* nextに値を記憶する (将值存入next) */
	next->next = NULL;		/*終わりはない (无末尾)*/
	next->val = add->val;	/*ポインターのコピー (指针的复制)*/
	next->size = add->size; /*サイズの保存 (保存大小)*/
	return TRUE;
}

/*
 *  リンクの前につける (在链表头部添加)
 * 引数 (参数)
 *      top         トップノード (头节点)
 *      add         付け加えるノードの値を保存するノードへのポインター (指向保存待添加节点值的节点的指针)
 * 返り値 (返回值)
 *      TRUE(1)     真 (真)
 *      FALSE(0)    偽 (偽)
 */
BOOL Nodeappendhead(Node **nowtop, Node *add)
{
	Node *newtop; /*新しい先頭用 (用于新的头节点)*/

	/*トップがNULLかどうかのチェック (检查头节点是否为NULL)*/
	if (*nowtop == NULL) {
		*nowtop = allocateMemory(sizeof(Node));
		if (*nowtop == NULL)
			return FALSE;
		(*nowtop)->next = NULL;		 /*終わりはない (无末尾)*/
		(*nowtop)->size = add->size; /*サイズの保存 (保存大小)*/
		(*nowtop)->val = add->val;	 /*ポインターのコピー (指针的复制)*/
		return TRUE;
	}

	/*
	 * 新しいノードを一つ作る (新建一个节点)
	 * next が差すものを nowtop にする。 (将next指向的内容设为nowtop)
	 * nowtop には 確保した newtop を設定する。 (将新分配的newtop设为nowtop)
	 */
	newtop = allocateMemory(sizeof(Node));
	newtop->next = *nowtop;
	newtop->val = add->val;
	newtop->size = add->size;
	*nowtop = newtop;
	return TRUE;
}

/*
 * 先頭のノードを引数にコピーして取り除く (将头节点复制到参数后删除)
 * 引数 (参数)
 *          top         ノードの先頭 (节点链表头)
 *          ret         取り除いたノードのコピー (被删除节点的副本)
 * 返り値 (返回值)
 *      TRUE(1)         真 (真)
 *      FALSE(0)        偽      ノードの先頭がNULL (伪，节点链表头为NULL)
 */
BOOL Noderemovehead(Node **top, Node *ret)
{
	Node *newtop; /*新しく先頭になるノード (将成为新头部的节点)*/

	if (*top == NULL)
		return FALSE;

	ret->val = (*top)->val;
	ret->size = (*top)->size;
	newtop = (*top)->next;
	freeMemory(*top);
	*top = newtop;

	return TRUE;
}

/*
 * 先頭のノードを引数にコピーして取り除く (将头节点复制到参数后删除)
 * 引数 (参数)
 *          top         ノードの先頭 (节点链表头)
 *          ret         取り除いたノードのコピー (被删除节点的副本)
 * 返り値 (返回值)
 *      TRUE(1)         真 (真)
 *      FALSE(0)        偽      ノードの先頭がNULL (伪，节点链表头为NULL)
 */
BOOL Noderemovetail(Node **top, Node *ret)
{
	Node *c;  /*ループ用 (循环用)*/
	Node *c1; /*ループ用 いつでも c->nextを指す (循环用，始终指向c->next)*/

	if (*top == NULL)
		return FALSE;

	c = *top;	  /*初期値記憶 (保存初始值)*/
	c1 = c->next; /*初期値記憶 (保存初始值)*/
	while (1) {
		if (c1->next == NULL)
			/*
			 * この状態で          c1
			 *                      |
			 *  c ---> +------+
			 *         | next |---->+------+
			 *         |------|     | next |---->NULL
			 *         |      |     +------+
			 *         +------+     |      |
			 *                      +------+
			 *となっている (成为这种状态)
			 */
			break;
		c = c->next;
		c1 = c->next;
	}
	c->next = NULL; /*c1にあたるものを削除するのでnextはNULLとする (由于要删除c1指向的节点，故将next设为NULL)*/
	/*コピー (复制)*/
	ret->val = c1->val;
	ret->size = c1->size;

	freeMemory(c1);

	return TRUE;
}
