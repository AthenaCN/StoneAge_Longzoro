#ifndef __LINK_H__
#define __LINK_H__

#include "common.h"

/*
 * リスト構造を定義する  (定义链表结构)
 * このリストはvalは読みには何もしない、つまりポインタのコピーのみ  (此链表对读取val不做任何事，只复制指针)
 * 行う
 */
typedef struct tagNode {
	struct tagNode *next; /*次のノードへのポインタ  (指向下一节点的指针)*/
	char *val;			  /*保持する  値  (保存的值)  */
	int size;			  /*valのサイズ  (val的大小)*/
} Node;

BOOL Nodeappendhead(Node **top, Node *add);
BOOL Nodeappendtail(Node **top, Node *add);
BOOL Noderemovehead(Node **top, Node *ret);
BOOL Noderemovetail(Node **top, Node *ret);
#endif
