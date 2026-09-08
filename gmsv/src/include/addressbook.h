#ifndef __ADDRESSBOOK_H__
#define __ADDRESSBOOK_H__

#include "common.h"
#include "util.h"
#include "net.h"

#define ADDRESSBOOK_MAX 40

typedef struct
{
	int use;
	BOOL online;				/* 相手キャラがサーバにいたらTRUE,  (对方角色在服务器上则TRUE,)
									そうでなかったらFALSE  (否则FALSE) */
	int level;					/* 相手キャラのレベル  (对方角色等级) */
	int duelpoint;				/* duelpoint*/
	int graphicsno;				/* 相手キャラのグラフィックNo  (对方角色图像编号) */
	char cdkey[CDKEYLEN];		/* CD ナンバ  (CD编号) */
	char charname[CHARNAMELEN]; /* 相手キャラの名前  (对方角色名) */
	int transmigration;			/* 転生回数  (转生次数) */

} ADDRESSBOOK_entry;

char *ADDRESSBOOK_makeAddressbookString(ADDRESSBOOK_entry *a);
BOOL ADDRESSBOOK_makeAddressbookEntry(char *in, ADDRESSBOOK_entry *a);
BOOL ADDRESSBOOK_deleteEntry(int meindex, int index);
BOOL ADDRESSBOOK_addEntry(int meindex);
BOOL ADDRESSBOOK_sendAddressbookTable(int cindex);
BOOL ADDRESSBOOK_sendAddressbookTableOne(int cindex, int num);
BOOL ADDRESSBOOK_sendMessage(int cindex, int aindex, char *text,
							 int color);
BOOL ADDRESSBOOK_sendMessage_FromOther(char *fromcdkey, char *fromcharaname,
									   char *tocdkey, char *tocharaname,
									   char *text, int color);

void ADDRESSBOOK_notifyLoginLogout(int cindex, int flg);
void ADDRESSBOOK_addAddressBook(int meindex, int toindex);
void ADDRESSBOOK_DispatchMessage(char *cd, char *nm, char *value, int mode);
int ADDRESSBOOK_getIndexInAddressbook(int cindex, char *cdkey, char *charname);
BOOL ADDRESSBOOK_AutoaddAddressBook(int meindex, int toindex);

#endif
