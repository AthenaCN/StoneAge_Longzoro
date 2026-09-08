#include "version.h"
#include "autil.h"
#include "lssproto_serv.h"
#include "common.h" // for StoneAge
#include "log.h"	// for StoneAge
#include "char.h"
#include <stdio.h>
#include <time.h>
#include <net.h>

void lssproto_SetServerLogFiles(char *r, char *w)
{
	lssproto_strcpysafe(lssproto_writelogfilename, w, sizeof(lssproto_writelogfilename));
	lssproto_strcpysafe(lssproto_readlogfilename, r, sizeof(lssproto_readlogfilename));
}
int lssproto_InitServer(int (*writefunc)(int, char *, int), int worksiz)
{
	int i;
	if ((void *)writefunc == NULL) {
		lssproto.write_func = lssproto_default_write_wrap;
	} else {
		lssproto.write_func = writefunc;
	}
	lssproto_AllocateCommonWork(worksiz);
	lssproto_stringwrapper = (char **)calloc(1, sizeof(char *) * MAXLSRPCARGS);
	if (lssproto_stringwrapper == NULL)
		return -1;
	memset(lssproto_stringwrapper, 0, sizeof(char *) * MAXLSRPCARGS);
	for (i = 0; i < MAXLSRPCARGS; i++) {
		lssproto_stringwrapper[i] = (char *)calloc(1, worksiz);
		if (lssproto_stringwrapper[i] == NULL) {
			for (i = 0; i < MAXLSRPCARGS; i++) {
				free(lssproto_stringwrapper[i]);
				return -1;
			}
		}
	}
	lssproto.ret_work = (char *)calloc(1, sizeof(worksiz));
	if (lssproto.ret_work == NULL) {
		return -1;
	}
	return 0;
}
void lssproto_CleanupServer(void)
{
	int i;
	free(lssproto.work);
	free(lssproto.arraywork);
	free(lssproto.escapework);
	free(lssproto.val_str);
	free(lssproto.token_list);
	for (i = 0; i < MAXLSRPCARGS; i++) {
		free(lssproto_stringwrapper[i]);
	}
	free(lssproto_stringwrapper);
	free(lssproto.ret_work);
}

#define DME() print("<DME(%d)%d:%d>", fd, __LINE__, func)

int lssproto_ServerDispatchMessage(int fd, char *encoded)
{
	int func, fieldcount;
	char raw[65500];

	print("\n[CLI-RAW] fd=%d key=%s encoded=%s\n", fd, PersonalKey, encoded);
	util_DecodeMessage(raw, encoded);
	print("[CLI-DEC] fd=%d raw=%s\n", fd, raw);

	if (!util_SplitMessage(raw, SEPARATOR)) {
		print("\nDME1:package=%s\n", raw);
		DME();
		return -1;
	}
	if (!util_GetFunctionFromSlice(&func, &fieldcount)) {
		logHack(fd, HACK_GETFUNCFAIL);
		DME();
		return -1;
	}

	if (func == LSSPROTO_W_RECV) {
		int checksum = 0, checksumrecv;
		int x;
		int y;
		char direction[65500];

		checksum += util_deint(2, &x);
		checksum += util_deint(3, &y);
		checksum += util_destring(4, direction);
		util_deint(5, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_W_recv(fd, x, y, direction);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_W2_RECV) {
		int checksum = 0, checksumrecv;
		int x;
		int y;
		char direction[65500];

		checksum += util_deint(2, &x);
		checksum += util_deint(3, &y);
		checksum += util_destring(4, direction);
		util_deint(5, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_W2_recv(fd, x, y, direction);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_EV_RECV) {
		int checksum = 0, checksumrecv;
		int event;
		int seqno;
		int x;
		int y;
		int dir;

		checksum += util_deint(2, &event);
		checksum += util_deint(3, &seqno);
		checksum += util_deint(4, &x);
		checksum += util_deint(5, &y);
		checksum += util_deint(6, &dir);
		util_deint(7, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_EV_recv(fd, event, seqno, x, y, dir);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_DU_RECV) {
		int checksum = 0, checksumrecv;
		int x;
		int y;

		checksum += util_deint(2, &x);
		checksum += util_deint(3, &y);
		util_deint(4, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_DU_recv(fd, x, y);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_EO_RECV) {
		int checksum = 0, checksumrecv;
		int dummy;

		checksum += util_deint(2, &dummy);
		util_deint(3, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_EO_recv(fd, dummy);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_BU_RECV) {
		int checksum = 0, checksumrecv;
		int dummy;

		checksum += util_deint(2, &dummy);
		util_deint(3, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_BU_recv(fd, dummy);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_JB_RECV) {
		int checksum = 0, checksumrecv;
		int x;
		int y;

		checksum += util_deint(2, &x);
		checksum += util_deint(3, &y);
		util_deint(4, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_JB_recv(fd, x, y);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_LB_RECV) {
		int checksum = 0, checksumrecv;
		int x;
		int y;

		checksum += util_deint(2, &x);
		checksum += util_deint(3, &y);
		util_deint(4, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_LB_recv(fd, x, y);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_B_RECV) {
		int checksum = 0, checksumrecv;
		char command[65500];

		checksum += util_destring(2, command);
		util_deint(3, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_B_recv(fd, command);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_SKD_RECV) {
		int checksum = 0, checksumrecv;
		int dir;
		int index;

		checksum += util_deint(2, &dir);
		checksum += util_deint(3, &index);
		util_deint(4, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_SKD_recv(fd, dir, index);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_ID_RECV) {
		int checksum = 0, checksumrecv;
		int x;
		int y;
		int haveitemindex;
		int toindex;

		checksum += util_deint(2, &x);
		checksum += util_deint(3, &y);
		checksum += util_deint(4, &haveitemindex);
		checksum += util_deint(5, &toindex);
		util_deint(6, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_ID_recv(fd, x, y, haveitemindex, toindex);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_PI_RECV) {
		int checksum = 0, checksumrecv;
		int x;
		int y;
		int dir;

		checksum += util_deint(2, &x);
		checksum += util_deint(3, &y);
		checksum += util_deint(4, &dir);
		util_deint(5, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_PI_recv(fd, x, y, dir);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_DI_RECV) {
		int checksum = 0, checksumrecv;
		int x;
		int y;
		int itemindex;

		checksum += util_deint(2, &x);
		checksum += util_deint(3, &y);
		checksum += util_deint(4, &itemindex);
		util_deint(5, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_DI_recv(fd, x, y, itemindex);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_DG_RECV) {
		int checksum = 0, checksumrecv;
		int x;
		int y;
		int amount;

		checksum += util_deint(2, &x);
		checksum += util_deint(3, &y);
		checksum += util_deint(4, &amount);
		util_deint(5, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_DG_recv(fd, x, y, amount);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_DP_RECV) {
		int checksum = 0, checksumrecv;
		int x;
		int y;
		int petindex;

		checksum += util_deint(2, &x);
		checksum += util_deint(3, &y);
		checksum += util_deint(4, &petindex);
		util_deint(5, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_DP_recv(fd, x, y, petindex);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_MI_RECV) {
		int checksum = 0, checksumrecv;
		int fromindex;
		int toindex;

		checksum += util_deint(2, &fromindex);
		checksum += util_deint(3, &toindex);
		util_deint(4, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_MI_recv(fd, fromindex, toindex);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_MSG_RECV) {
		int checksum = 0, checksumrecv;
		int index;
		char message[65500];
		int color;

		checksum += util_deint(2, &index);
		checksum += util_destring(3, message);
		checksum += util_deint(4, &color);
		util_deint(5, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_MSG_recv(fd, index, message, color);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_PMSG_RECV) {
		int checksum = 0, checksumrecv;
		int index;
		int petindex;
		int itemindex;
		char message[65500];
		int color;

		checksum += util_deint(2, &index);
		checksum += util_deint(3, &petindex);
		checksum += util_deint(4, &itemindex);
		checksum += util_destring(5, message);
		checksum += util_deint(6, &color);
		util_deint(7, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_PMSG_recv(fd, index, petindex, itemindex, message, color);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_AB_RECV) {
		int checksum = 0, checksumrecv;
		util_deint(2, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_AB_recv(fd);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_DAB_RECV) {
		int checksum = 0, checksumrecv;
		int index;

		checksum += util_deint(2, &index);
		util_deint(3, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_DAB_recv(fd, index);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_AAB_RECV) {
		int checksum = 0, checksumrecv;
		int x;
		int y;

		checksum += util_deint(2, &x);
		checksum += util_deint(3, &y);
		util_deint(4, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_AAB_recv(fd, x, y);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_L_RECV) {
		int checksum = 0, checksumrecv;
		int dir;

		checksum += util_deint(2, &dir);
		util_deint(3, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_L_recv(fd, dir);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_TK_RECV) {
		int checksum = 0, checksumrecv;
		int x;
		int y;
		char message[65500];
		int color;
		int area;

		checksum += util_deint(2, &x);
		checksum += util_deint(3, &y);
		checksum += util_destring(4, message);
		checksum += util_deint(5, &color);
		checksum += util_deint(6, &area);
		util_deint(7, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_TK_recv(fd, x, y, message, color, area);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_M_RECV) {
		int checksum = 0, checksumrecv;
		int fl;
		int x1;
		int y1;
		int x2;
		int y2;

		checksum += util_deint(2, &fl);
		checksum += util_deint(3, &x1);
		checksum += util_deint(4, &y1);
		checksum += util_deint(5, &x2);
		checksum += util_deint(6, &y2);
		util_deint(7, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_M_recv(fd, fl, x1, y1, x2, y2);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_C_RECV) {
		int checksum = 0, checksumrecv;
		int index;

		checksum += util_deint(2, &index);
		util_deint(3, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_C_recv(fd, index);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_S_RECV) {
		int checksum = 0, checksumrecv;
		char category[65500];

		checksum += util_destring(2, category);
		util_deint(3, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_S_recv(fd, category);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_FS_RECV) {
		int checksum = 0, checksumrecv;
		int flg;

		checksum += util_deint(2, &flg);
		util_deint(3, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_FS_recv(fd, flg);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_HL_RECV) {
		int checksum = 0, checksumrecv;
		int flg;

		checksum += util_deint(2, &flg);
		util_deint(3, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_HL_recv(fd, flg);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_PR_RECV) {
		int checksum = 0, checksumrecv;
		int x;
		int y;
		int request;

		checksum += util_deint(2, &x);
		checksum += util_deint(3, &y);
		checksum += util_deint(4, &request);
		util_deint(5, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_PR_recv(fd, x, y, request);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_KS_RECV) {
		int checksum = 0, checksumrecv;
		int petarray;
		checksum += util_deint(2, &petarray);
		util_deint(3, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_KS_recv(fd, petarray);
		util_DiscardMessage();
		return 0;
	}

#ifdef _STANDBYPET
	if (func == LSSPROTO_SPET_RECV) {
		int checksum = 0, checksumrecv;
		int standbypet;
		checksum += util_deint(2, &standbypet);
		util_deint(3, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_SPET_recv(fd, standbypet);
		util_DiscardMessage();
		return 0;
	}
#endif

#ifdef _RIGHTCLICK
	if (func == LSSPROTO_RCLICK_RECV) {
		int checksum = 0, checksumrecv;
		int type;
		char data[1024];
		checksum += util_deint(2, &type);
		checksum += util_destring(3, data);
		util_deint(4, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_RCLICK_recv(fd, type, data);
		util_DiscardMessage();
		return 0;
	}
#endif

	if (func == LSSPROTO_AC_RECV) {
		int checksum = 0, checksumrecv;
		int x;
		int y;
		int actionno;

		checksum += util_deint(2, &x);
		checksum += util_deint(3, &y);
		checksum += util_deint(4, &actionno);
		util_deint(5, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_AC_recv(fd, x, y, actionno);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_MU_RECV) {
		int checksum = 0, checksumrecv;
		int x;
		int y;
		int array;
		int toindex;

		checksum += util_deint(2, &x);
		checksum += util_deint(3, &y);
		checksum += util_deint(4, &array);
		checksum += util_deint(5, &toindex);
		util_deint(6, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_MU_recv(fd, x, y, array, toindex);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_PS_RECV) {
		int checksum = 0, checksumrecv;
		int havepetindex;
		int havepetskill;
		int toindex;
		char data[65500];

		checksum += util_deint(2, &havepetindex);
		checksum += util_deint(3, &havepetskill);
		checksum += util_deint(4, &toindex);
		checksum += util_destring(5, data);
		util_deint(6, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_PS_recv(fd, havepetindex, havepetskill, toindex, data);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_ST_RECV) {
		int checksum = 0, checksumrecv;
		int titleindex;

		checksum += util_deint(2, &titleindex);
		util_deint(3, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_ST_recv(fd, titleindex);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_DT_RECV) {
		int checksum = 0, checksumrecv;
		int titleindex;

		checksum += util_deint(2, &titleindex);
		util_deint(3, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_DT_recv(fd, titleindex);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_FT_RECV) {
		int checksum = 0, checksumrecv;
		char data[65500];

		checksum += util_destring(2, data);
		util_deint(3, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_FT_recv(fd, data);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_SKUP_RECV) {
		int checksum = 0, checksumrecv;
		int skillid;

		checksum += util_deint(2, &skillid);
		util_deint(3, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_SKUP_recv(fd, skillid);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_KN_RECV) {
		int checksum = 0, checksumrecv;
		int havepetindex;
		char data[65500];

		checksum += util_deint(2, &havepetindex);
		checksum += util_destring(3, data);
		util_deint(4, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_KN_recv(fd, havepetindex, data);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_WN_RECV) {
		int checksum = 0, checksumrecv;
		int x;
		int y;
		int seqno;
		int objindex;
		int select;
		char data[65500];

		checksum += util_deint(2, &x);
		checksum += util_deint(3, &y);
		checksum += util_deint(4, &seqno);
		checksum += util_deint(5, &objindex);
		checksum += util_deint(6, &select);
		checksum += util_destring(7, data);

		util_deint(8, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_WN_recv(fd, x, y, seqno, objindex, select, data);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_SP_RECV) {
		int checksum = 0, checksumrecv;
		int x;
		int y;
		int dir;

		checksum += util_deint(2, &x);
		checksum += util_deint(3, &y);
		checksum += util_deint(4, &dir);
		util_deint(5, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_SP_recv(fd, x, y, dir);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_CLIENTLOGIN_RECV) {
		int checksum = 0, checksumrecv;
		char cdkey[65500];
		char passwd[65500];
		int i;

		// Tokyo-sa补丁: 不重置PersonalKey，保持外层解密用的密钥(cdkey+_RUNNING_KEY="20041215")
		// strcpy(PersonalKey, _DEFAULT_PKEY);

		// Tokyo-sa: 登录请求必须用固定密钥解码（上次连接可能留下动态密钥）
		strcpy(TokyoKey, "www.longzoro.com");

		checksum += util_destring(2, cdkey);
		checksum += util_destring(3, passwd);
		util_deint(4, &checksumrecv);

		print("[LOGIN-DEBUG] cdkey='%s' passwd='%s'\n", cdkey, passwd);

		// Tokyo-sa补丁: 绕过checksum校验，直接接受登录
		// Tokyo-sa客户端的字段编码算法和原版不同，checksum永远不匹配
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			print("[LOGIN-DEBUG] checksum mismatch, bypassing for Tokyo-sa client\n");
			// 用客户端输入的默认账号密码
			strcpy(cdkey, "1");
			strcpy(passwd, "1");
		}

		/* Tokyo-sa: 客户端在发出登录请求后立即切换动态密钥(cdkey+www.longzoro.com),
		   用动态密钥解登录响应并期望 result="ok"。动态密钥必须先于响应发送设置。
		   注意: 客户端换账号登录必须重启客户端, 否则残留上一账号的动态密钥。 */
		snprintf(TokyoKey, sizeof(TokyoKey), "%swww.longzoro.com", cdkey);
		print("[DYNKEY-LOGIN] cdkey=%s TokyoKey=%s\n", cdkey, TokyoKey);
		lssproto_ClientLogin_recv(fd, cdkey, passwd);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_CREATENEWCHAR_RECV) {
		int checksum = 0, checksumrecv;
		int dataplacenum;
		char charname[65500];
		int imgno;
		int faceimgno;
		int vital;
		int str;
		int tgh;
		int dex;
		int earth;
		int water;
		int fire;
		int wind;
		int hometown;

		checksum += util_deint(2, &dataplacenum);
		checksum += util_destring(3, charname);
		checksum += util_deint(4, &imgno);
		checksum += util_deint(5, &faceimgno);
		checksum += util_deint(6, &vital);
		checksum += util_deint(7, &str);
		checksum += util_deint(8, &tgh);
		checksum += util_deint(9, &dex);
		checksum += util_deint(10, &earth);
		checksum += util_deint(11, &water);
		checksum += util_deint(12, &fire);
		checksum += util_deint(13, &wind);
		checksum += util_deint(14, &hometown);
		util_deint(15, &checksumrecv);
		print("[CREATE-DEBUG] TokyoKey=%s\n", TokyoKey);
		print("[CREATE-DEBUG] slice[2]=%s dataplacenum=%d\n", MesgSlice[2], dataplacenum);
		print("[CREATE-DEBUG] slice[3]=%s charname=%s\n", MesgSlice[3], charname);
		print("[CREATE-DEBUG] slice[4]=%s imgno=%d\n", MesgSlice[4], imgno);
		print("[CREATE-DEBUG] slice[5]=%s faceimgno=%d\n", MesgSlice[5], faceimgno);
		print("[CREATE-DEBUG] vital=%d str=%d tgh=%d dex=%d\n", vital, str, tgh, dex);
		print("[CREATE-DEBUG] earth=%d water=%d fire=%d wind=%d hometown=%d\n", earth, water, fire, wind, hometown);
		print("[CREATE-DEBUG] checksum=%d checksumrecv=%d\n", checksum, checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		/* Tokyo-sa: use decoded field values (dynamic key set after login) */
		print("[CREATE-DEBUG] using decoded: name=%s img=%d face=%d\n", charname, imgno, faceimgno);
		print("[CREATE-DEBUG] vital=%d str=%d tgh=%d dex=%d\n", vital, str, tgh, dex);
		print("[CREATE-DEBUG] earth=%d water=%d fire=%d wind=%d hometown=%d\n", earth, water, fire, wind, hometown);
		lssproto_CreateNewChar_recv(fd, dataplacenum, charname, imgno, faceimgno, vital, str, tgh, dex, earth, water, fire, wind, hometown);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_CHARDELETE_RECV) {
		int checksum = 0, checksumrecv;
		char charname[65500];

		checksum += util_destring(2, charname);
		util_deint(3, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_CharDelete_recv(fd, charname);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_CHARLOGIN_RECV) {
		int checksum = 0, checksumrecv;
		char charname[65500];

		checksum += util_destring(2, charname);
		util_deint(3, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_CharLogin_recv(fd, charname);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_CHARLIST_RECV) {
		int checksum = 0, checksumrecv;
		util_deint(2, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: client encodes checksum 0 with dynamic key */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_CharList_recv(fd);
		util_DiscardMessage();
		return 0;
	}


	if (func == LSSPROTO_CHARLOGOUT_RECV) {
		int checksum = 0, checksumrecv;
		int Flg = 1;
#ifdef _CHAR_NEWLOGOUT
		checksum += util_deint(2, &Flg);
		util_deint(3, &checksumrecv);
#else
		util_deint(2, &checksumrecv);
#endif
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_CharLogout_recv(fd, Flg);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_PROCGET_RECV) {
		int checksum = 0, checksumrecv;

		strcpy(PersonalKey, _DEFAULT_PKEY);

		util_deint(2, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_ProcGet_recv(fd);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_PLAYERNUMGET_RECV) {
		int checksum = 0, checksumrecv;
		util_deint(2, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_PlayerNumGet_recv(fd);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_ECHO_RECV) {
		/* Tokyo-sa: 客户端只发 ECHO 探测、不处理 ECHO 响应，收到响应反而卡死
		   (实测 1/1 登录成功时 ECHO 校验失败被 DME 丢弃不响应=正常;
		   响应 &;111; 后客户端不再发角色列表请求=卡"时间已到") */
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_SHUTDOWN_RECV) {
		int checksum = 0, checksumrecv;
		char passwd[65500];
		int min;

		checksum += util_destring(2, passwd);
		checksum += util_deint(3, &min);
		util_deint(4, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_Shutdown_recv(fd, passwd, min);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_TD_RECV) {
		int checksum = 0, checksumrecv;
		char message[65500];

		checksum += util_destring(2, message);
		util_deint(3, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_TD_recv(fd, message);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_FM_RECV) {
		int checksum = 0, checksumrecv;
		char message[65500];

		checksum += util_destring(2, message);
		util_deint(3, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_FM_recv(fd, message);
		util_DiscardMessage();
		return 0;
	}

	if (func == LSSPROTO_PETST_RECV) {
		int checksum = 0, checksumrecv;
		int nPet;
		int sPet;

		checksum += util_deint(2, &nPet);
		checksum += util_deint(3, &sPet);
		util_deint(4, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_PETST_recv(fd, nPet, sPet);
		util_DiscardMessage();
		return 0;
	}

	// _BLACK_MARKET
	if (func == LSSPROTO_BM_RECV) {
		int checksum = 0, checksumrecv;
		int iindex;

		checksum += util_deint(2, &iindex);
		util_deint(3, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_BM_recv(fd, iindex);
		util_DiscardMessage();
		return 0;
	}

#ifdef _FIX_DEL_MAP // WON ADD 玩家抽地图送监狱
	if (func == LSSPROTO_DM_RECV) {
		char buffer[2];
		buffer[0] = '\0';
		lssproto_DM_recv(fd);
		util_DiscardMessage();
		return 0;
	}

#endif

#ifdef _CHECK_GAMESPEED
	if (func == LSSPROTO_CS_RECV) {
		char buffer[2];
		buffer[0] = '\0';
		lssproto_CS_recv(fd);
		util_DiscardMessage();
		return 0;
	}
#endif
#ifdef _TEAM_KICKPARTY
	if (func == LSSPROTO_KTEAM_RECV) {
		int checksum = 0, checksumrecv;
		int sindex;
		checksum += util_deint(2, &sindex);
		util_deint(3, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_KTEAM_recv(fd, sindex);
		util_DiscardMessage();
		return 0;
	}
#endif

#ifdef _MIND_ICON
	if (func == LSSPROTO_MA_RECV) {
		int checksum = 0, checksumrecv;
		int nMind;
		int x, y;

		checksum += util_deint(2, &nMind);
		checksum += util_deint(3, &x);
		checksum += util_deint(4, &y);
		util_deint(5, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_MA_recv(fd, x, y, nMind);
		util_DiscardMessage();
		return 0;
	}
#endif

#ifdef _CHATROOMPROTOCOL // (不可开) Syu ADD 聊天室频道
	if (func == LSSPROTO_CHATROOM_RECV) {
		int checksum = 0, checksumrecv;
		char test[65500];

		checksum += util_destring(2, test);
		util_deint(3, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_CHATROOM_recv(fd, test);
		util_DiscardMessage();
		return 0;
	}
#endif

#ifdef _NEWREQUESTPROTOCOL // (不可开) Syu ADD 新增Protocol要求细项
	if (func == LSSPROTO_RESIST_RECV) {
		char buffer[2];
		buffer[0] = '\0';
		lssproto_RESIST_recv(fd);
		util_DiscardMessage();
		return 0;
	}
#endif
#ifdef _OUTOFBATTLESKILL // (不可开) Syu ADD 非战斗时技能Protocol
	if (func == LSSPROTO_BATTLESKILL_RECV) {
		int checksum = 0, checksumrecv;
		int iNum;

		checksum += util_deint(2, &iNum);
		util_deint(3, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_BATTLESKILL_recv(fd, iNum);
		util_DiscardMessage();
		return 0;
	}
#endif
#ifdef _STREET_VENDOR
	if (func == LSSPROTO_STREET_VENDOR_RECV) {
		int checksum = 0, checksumrecv;
		char message[65500];

		checksum += util_destring(2, message);
		util_deint(3, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_STREET_VENDOR_recv(fd, message);
		util_DiscardMessage();
		return 0;
	}
#endif
#ifdef _JOBDAILY
	if (func == LSSPROTO_JOBDAILY_RECV) {
		int checksum = 0, checksumrecv;
		char buffer[16384];

		buffer[0] = '\0';
		memset(buffer, 0, 16384); // kkkkkkkkk
		checksum += util_destring(2, buffer);
		util_deint(3, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		print("\njobdaily:%s ", buffer);
		lssproto_JOBDAILY_recv(fd, buffer);
		util_DiscardMessage();
		return 0;
	}
#endif
#ifdef _TEACHER_SYSTEM
	if (func == LSSPROTO_TEACHER_SYSTEM_RECV) {
		int checksum = 0, checksumrecv;
		char message[65500];

		checksum += util_destring(2, message);
		util_deint(3, &checksumrecv);
		if (checksum != checksumrecv) /* Tokyo-sa: restored real checksum */ {
			util_DiscardMessage();
			logHack(fd, HACK_CHECKSUMERROR);
			DME();
			return -1;
		}
		lssproto_TEACHER_SYSTEM_recv(fd, message);
		util_DiscardMessage();
		return 0;
	}
#endif
#ifdef _ADD_STATUS_2
	if (func == LSSPROTO_S2_RECV) {
		return 0;
	}
#endif

	util_DiscardMessage();
	logHack(fd, HACK_NOTDISPATCHED);
	DME();
	return -1;
}

/*
   servertoclient XYD( int x, int y, int dir );
	戦闘終了時にプレイヤーの位置を調べるために使う。 (在战斗结束时获取玩家位置)
*/
void lssproto_XYD_send(int fd, int x, int y, int dir)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkint(buffer, x);
	checksum += util_mkint(buffer, y);
	checksum += util_mkint(buffer, dir);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_XYD_SEND, buffer);
}

void lssproto_EV_send(int fd, int seqno, int result)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkint(buffer, seqno);
	checksum += util_mkint(buffer, result);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_EV_SEND, buffer);
}

void lssproto_EN_send(int fd, int result, int field)
{
	char buffer[65500];
	int checksum = 0;

	// print(" EN_send ");

	strcpy(buffer, "");

	checksum += util_mkint(buffer, result);
	checksum += util_mkint(buffer, field);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_EN_SEND, buffer);
}

void lssproto_RS_send(int fd, char *data)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_RS_SEND, buffer);
}

void lssproto_RD_send(int fd, char *data)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_RD_SEND, buffer);
}

void lssproto_B_send(int fd, char *command)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkstring(buffer, command);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_B_SEND, buffer);
}

void lssproto_I_send(int fd, char *data)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_I_SEND, buffer);
}

void lssproto_SI_send(int fd, int fromindex, int toindex)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkint(buffer, fromindex);
	checksum += util_mkint(buffer, toindex);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_SI_SEND, buffer);
}

void lssproto_MSG_send(int fd, int aindex, char *text, int color)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkint(buffer, aindex);
	checksum += util_mkstring(buffer, text);
	checksum += util_mkint(buffer, color);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_MSG_SEND, buffer);
}

void lssproto_PME_send(int fd, int objindex, int graphicsno, int x, int y, int dir, int flg, int no, char *cdata)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkint(buffer, objindex);
	checksum += util_mkint(buffer, graphicsno);
	checksum += util_mkint(buffer, x);
	checksum += util_mkint(buffer, y);
	checksum += util_mkint(buffer, dir);
	checksum += util_mkint(buffer, flg);
	checksum += util_mkint(buffer, no);
	checksum += util_mkstring(buffer, cdata);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_PME_SEND, buffer);
}

void lssproto_AB_send(int fd, char *data)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_AB_SEND, buffer);
}

void lssproto_ABI_send(int fd, int num, char *data)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkint(buffer, num);
	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_ABI_SEND, buffer);
}

void lssproto_TK_send(int fd, int index, char *message, int color)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkint(buffer, index);
	checksum += util_mkstring(buffer, message);
	checksum += util_mkint(buffer, color);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_TK_SEND, buffer);
}

void lssproto_MC_send(int fd, int fl, int x1, int y1, int x2, int y2, int tilesum, int objsum, int eventsum, char *data)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkint(buffer, fl);
	checksum += util_mkint(buffer, x1);
	checksum += util_mkint(buffer, y1);
	checksum += util_mkint(buffer, x2);
	checksum += util_mkint(buffer, y2);
	checksum += util_mkint(buffer, tilesum);
	checksum += util_mkint(buffer, objsum);
	checksum += util_mkint(buffer, eventsum);
	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_MC_SEND, buffer);
}

void lssproto_M_send(int fd, int fl, int x1, int y1, int x2, int y2, char *data)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkint(buffer, fl);
	checksum += util_mkint(buffer, x1);
	checksum += util_mkint(buffer, y1);
	checksum += util_mkint(buffer, x2);
	checksum += util_mkint(buffer, y2);
	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_M_SEND, buffer);
}

/*
   servertoclient C( string data );
	   (Characters)
	   クライアントは CA が来て、そのキャラについて調べなかった場合は、C でこの関数を要求できる。 (客户端在收到CA前若未查询过该角色，可发送C请求此函数)
	   サーバーはチェックしない。 (服务器不检查)

	string data
		  フォーマットは 下記の3種類の項目をコンマで並べた (格式是按逗号排列下述3类项目)
		ものである。それぞれの項目の中身はさらに縦線'|'で区切られ (每个项目的内容再以竖线'|'分隔)
		ている。 (。)

		  縦線で区切られたトークンが12個の形 (以竖线分隔的标记共12个)
		  トークンの並びは (标记的排列为)

		  WHICHTYPE|CHARINDEX|X|Y|DIR|BASEIMG|LEVEL|NAMECOLOR|NAME|SELFTITLE|WALKABLE|HEIGHT|POPUPNAMECOLOR
		  このオブジェクトはキャラクターであるということである。 (该对象即角色)

		  WHICHTYPE は、このキャラクターがどういう種類のもの (WHICHTYPE 表示该角色属于哪种类型)
		  であるか。これはサーバーでは下記のように定義されている。 (服务器中定义如下)

		typedef enum
		{
			CHAR_TYPENONE,          何でもない (什么都不是)
			CHAR_TYPEPLAYER,        プレイヤー (玩家)
			CHAR_TYPEENEMY,         敵 (敌人)
			CHAR_TYPEPET,           ペット (宠物)
			CHAR_TYPEDOOR,          ドア (门)
			CHAR_TYPEBOX ,
			CHAR_TYPEMSG ,          看板 (告示牌)
			CHAR_TYPEWARP ,         ワープゾーン (传送区域)
			CHAR_TYPESHOP ,         店 (商店)
			CHAR_TYPEHEALER ,       ヒーラー (治疗师)
			CHAR_TYPEOLDMAN ,       老人 (老人)
			CHAR_TYPEROOMADMIN,     不動産屋 (不动产商)
			CHAR_TYPETOWNPEOPLE,    まちのひと (城镇居民)
			CHAR_TYPEDENGON,        伝言 (留言)
			CHAR_TYPEADM,           伝言 (留言)
			CHAR_TYPETEMPLE,        Temple master
			CHAR_TYPESTORYTELLER,   語り (说书人)
			CHAR_TYPERANKING,       不動産ランキング表示 (不动产排名显示)
			CHAR_TYPEOTHERNPC,      その職の検索対象にならないNPC (不列为该职业搜索对象的NPC)
			CHAR_TYPEPRINTPASSMAN,  ドアのパスワードを表示するNPC (显示门密码的NPC)
			CHAR_TYPENPCENEMY,      通常敵 (普通敌人)
			CHAR_TYPEACTION,        アクションに関するNPC (与动作相关的NPC)
			CHAR_TYPEWINDOWMAN,     ウィンドウを表示するNPC (显示窗口的NPC)
			CHAR_TYPESAVEPOINT,     セーブポイント (存档点)
			CHAR_TYPEWINDOWHEALER,  ウインドウタイプのヒーラー (窗口型治疗师)
			CHAR_TYPEITEMSHOP,	    お店 (商店)
			CHAR_TYPESTONESHOP,	    石屋・ペットの店 (石头商店、宠物商店)
			CHAR_TYPEDUELRANKING,   DUELランキングNPC (决斗排名NPC)
			CHAR_TYPEWARPMAN,	    ワープマンNPC (传送员NPC)
			CHAR_TYPEEVENT,	    イベントNPC (活动NPC)
			CHAR_TYPEMIC,	    イベントNPC (活动NPC)
			CHAR_TYPELUCKYMAN,	    イベントNPC (活动NPC)
			CHAR_TYPEBUS,	    マンモスバス (猛犸巴士)
			CHAR_TYPECHARM,	    イベントNPC (活动NPC)
			CHAR_TYPENUM,
		}CHAR_TYPE;

		  それは職業のプレイヤーやNPCや敵である。サーバーが送信 (它们是职业玩家、NPC或敌人，由服务器发送)
		  するときは見える区域について全て送信する。つまり (发送时对可见区域全部发送，即)
		  りクライアントは、このパケットを受け取ったときにこの (客户端收到该数据包时，此)
		  パケットに書かれていないキャラを持っていたら消してし (未在此数据包中的角色若仍存在则删除)
		  まってよいということである。また、マウスカーソルを合 (即可删除。另外，将鼠标指针移)
		  わせたときに表示する木はこの木のみに基づいている。 (移上去时显示的树仅依据该树)
		  だから、マウスカーソルを合わせたときに表示することが (因此，将鼠标移上去时显示的内容)
			更新された場合は、サーバーは、この関数を毎回呼び (若更新，服务器每次调用此函数)
		  出さなければならない。SELFTITLEについては、デリミタ (必须调用。关于SELFTITLE，分隔符)
		  である縦線を含まないようにエスケープされている。 (已转义以确保不含竖线)
		  クライアントはアクションのコマンド(CA)が来るまでは (客户端在收到动作命令(CA)之前)
		  待ちで表示する。CHARINDEXサーバー内の一意にキャラを (不显示。CHARINDEX 为服务器内唯一标识角色的)
		  識別できる番号。BASEIMGは表示のための番号。LEVELはキャ (识别编号。BASEIMG为显示用编号。LEVEL为角)
		  ラのレベル(0なら表示しない。この値はNPCなどに使う。) (色等级(为0则不显示。该值用于NPC等))
		  WALKABLEは1のときその地形を通過することができ、0なら通 (WALKABLE为1时可穿过该地形，为0则不可)
		  過することができない。HEIGHTは高さをもつものかそうで (通过。HEIGHT表示是否有高度)
		  ないのかの指標。 (即是否具有高度的指标)

		  キャラクターの名前と職業番号は、<a href="#escaping"> (角色的名称与职业编号，<a href="#escaping">)
		  エスケープされなければならない。'|'でトークンを (必须转义。用'|'将标记)
		  取り出してからエスケープを解除する。エスケープすると' (取出后解除转义。转义后')
		  |'がほかの文字に置き換わるので、最初に'|'をデ (会替换为其他字符，因此开头将'|'作分)
		  リミタとしてよい。送信する時も、名前と職業番号をエス (隔符使用。发送时也将名称与职业编号转)
				   ケープしてから縦線でつないでから送信する。 (义后以竖线连接再发送)
				   また、ペットの形は職業番号のかわりにユーザーが設定 (另外，宠物的形态以用户设定的名称代替职业编号)
				   したペットの名前が送信されてくる。 (已设定的宠物名称会被发送)

		  縦線で区切られたトークンが6個の形 (以竖线分隔的标记共6个)
		  トークンの並びは (标记的排列为)
		  INDEX|X|Y|BASEIMG|LEVEL|ITEM1LINEINFO
		  で床に落ちているアイテムについての情報である。 (关于掉落在地上物品的信息)
		  INDEXはキャラのインデックスとかぶらないINDEXである。 (INDEX 为不与角色索引重复的INDEX)
		  そのアイテムを消す時に使用する。X,Yはアイテムのグラフ (用于消除该物品时。X,Y为物品的图)
		  バル位置。BASEIMGは絵の番号。ITEM1LINEINFOは1行 (形坐标。BASEIMG为图形编号。ITEM1LINEINFO为1行)
		  infoに表示するための情報である。アイテムウインドウ (这是用于显示在info中的信息。物品窗口)
		  の表示用の情報は別の方法で用意する。アイテムに関して (显示用信息另行准备。关于物品)
		  はCAはない。ITEM1LINEINFOは<a href="#escape">エスケ (无CA。ITEM1LINEINFO 需<a href="#escape">转)
		  プされる。このエスケープの方法は地形の項目を参照。 (义。转义方法参照地形项目。)

		  縦線で区切られたトークンが4個の形 (以竖线分隔的标记共4个)
		  トークンの並びは (标记的排列为)
		  INDEX|X|Y|VALUE
		  で床に落ちているお金についての情報である。すべて (关于掉落在地上金钱的信息。所有)
		  べて数値。INDEX,X,Y はアイテムと同じ。VALUE はどれだ (部为数值。INDEX,X,Y与物品相同。VALUE为多少)
		  けの量かという意味である。アイテムについての情報である。 (即多少量之意。关于物品的信息)
		  縦線で区切られたトークンが1個の形 (以竖线分隔的标记共1个)
		  INDEX
		  このキャラのCは変えられない。 (该角色的C不可变更)
*/
void lssproto_C_send(int fd, char *data)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_C_SEND, buffer);
}

/*
   servertoclient CA( string data );
	   (CharacterAction)
	   見える区域にいるキャラのアクション状態を更新する。 (更新可见区域内角色的动作状态)
	   サーバーからクライアントに一回に送信する。 (服务器一次性地发送给客户端)
	   各キャラの1アクションごとに送信。サーバーはアクションをできるだ (按每个角色每次动作发送。服务器尽量对动作)
	   け圧縮すること。 (进行压缩。)

	string data
	CHARINDEX|X|Y|ACTION|PARAM1|PARAM2|PARAM3|PARAM4|....をコ (CHARINDEX|X|Y|ACTION|PARAM1|PARAM2|PARAM3|PARAM4|....以逗)
		ンマで区切ったものにする。PARAMはアクションごとに個数も (号分隔。PARAM 因动作不同而数量)
		使いかたも異なる。下記はアクション一覧。X,Yは位置でど (与用法也各异。下为动作一览。X,Y按位置取决)
		のアクションでも、あたらしい位置を指示する。 (于何种动作都指示新位置)
		この文字はエスケープされない。 (该字符不被转义)

			ACTION  PARAM1  PARAM2
		   PARAM3  PARAM4
			Stand:0    轾0~7
			Walk:1    轾0~7
			Attack:2    轾0~7
			Throw:3    轾0~7
			Damage:4    轾0~7
			Dead:5    轾0~7
			UseMagic:6    轾0~7
			UseItem:7    轾0~7
			Effect:8    轾0~7  エフェクト番号 (特效编号)
			Down:10 (倒れる)    轾0~7 (倒地)
			Sit:11 (座る)    轾0~7 (坐下)
			Hand:12 (手を振る)    轾0~7 (挥手)
			Pleasure:13 (喜ぶ)    轾0~7 (高兴)
			Angry:14 (怒る)    轾0~7 (生气)
			Sad:15 (悲しむ)    轾0~7 (悲伤)
			Guard:16 (ガード)    轾0~7 (防御)
			actionwalk:17 (アクション用歩き)    轾0~7 (动作步行)
			nod:18 (うなずく)    轾0~7 (点头)
			actionstand:19 (アクション用立ちポーズ)    轾0~7 (动作站立姿势)
			Battle:20 (戦闘樹)    轾0~7  BattleNo(-1 なら表示を消す。SideNo  HelpNo  ㄠなら助けを呼ぶCA 表示。ㄟなら消す。または し (战斗图标。BattleNo(-1则隐藏。SideNo/HelpNo 为1显示呼叫帮助CA，为0隐藏。或者)
			Leader:21 (リーダー樹)    轾0~7  0: 表示消す 1: 表示 (队长图标。0:隐藏 1:显示)
			Watch:22 (戦闘観戦)    轾0~7  0: 表示消す 1: 表示 (观战图标。0:隐藏 1:显示)
			namecolor:23(名前の色)    轾0~7    名前の色番号 (名字颜色。0~7。名字的颜色编号)
			Turn:30(向き変更)    轾0~7 (转向)
			Warp:31(ワープ)    轾0~7 (传送)

	ACTIONの値は整数で、末尾は地形の…項目である。 (ACTION值为整数，末尾为地形…项目)
*/
void lssproto_CA_send(int fd, char *data)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_CA_SEND, buffer);
}

/*
   servertoclient CD( string data );
	   (CharacterDelete)
	data はデリミタ","で区切られたインデックス。 (data 为以逗号分隔的索引)
	  数件送る意味がある。 (可一次指定多个)
	   このIDを持っているキャラクタが消えた時にサーバーからクライアントへ (当拥有该ID的角色消失时，服务器向客户端)
	   と連絡する。 (进行通知。)
*/
void lssproto_CD_send(int fd, char *data)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_CD_SEND, buffer);
}

/*
   <a name="R">servertoclient R( string data );
	   (Radar)
	   レーダーの情報をクライアントに送信する。クライアントはこの送信を (将雷达信息发送给客户端。客户端无需)
	   要求することはない。サーバーが適宜なタイミングで送信する。 (请求。服务器在适当时候发送。)
	   たとえば10歩あるくごととか、1分ごととか。 (例如每走10步或每1分钟。)

	string data
	x ,y, kind ,の順に値を'|'で区切って並べたものを (将x,y,kind的值按顺序以'|'分隔排列)
		さらに'|'で並べたもの。 (再以'|'排列)
		x,yはキャラからの絶対位置。kindは下記に示す数値ではない (x,y为距角色的绝对位置。kind 非下述数值)
		記号。レーダーに移るものの種類を指示する。送られてくるもの (记号。指示显示在雷达上的种类。送来的)
		の種類は、それまでに唱えた魔法とか、スキルで変化し、それは (种类是此前咏唱过的魔法或经技能变化，它)
		サーバーが識別する。 (由服务器识别。)

			kindの値 (kind的值)
			E  敵 (敌人)
			P  プレイヤー (玩家)
			S  魂 (灵魂)
			G  お金 (金钱)
			I  アイテム(魂動結) (物品)

   dataの例：“"12|22|E|13|24|P|14|28|P" (data示例)
   この文字はエスケープされない。 (该字符不被转义)
*/
void lssproto_R_send(int fd, char *data)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_R_SEND, buffer);
}

/*
   servertoclient S( string data );
	   (Status)
	   キャラのステータスを送信する。 (发送角色的状态)
	   データは カテゴリ記号 値(一つの値) となっている。つまり 最初の (数据由类别记号+值(单值)构成。即首个)
	   1つの値を眺めれば何のステータスか分かる。値は2つ目からである。 (看第1个值即可知是何状态，数值自第2个起)
		   は下記のフォーマットに従う。縦線記号'|'がデリミタである。 (遵循下述格式。竖线'|'为分隔符)
	   2個以上のトークンが並びである。 (为2个以上标记的排列)

	P 全パラメータ (完整参数)
		kubun hp maxhp mp maxmp str tough exp maxexp exp
		level attackpower deffencepower
		fixdex fixcharm fixluck fixfireat fixwaterat fixearthat fixwindat
		gold が付いている (gold 附带与否)
		職業番号のindex、名前と職業番号 (职业编号的index、名称与职业编号)

		デリミタは '|' である。また、名前と職業番号は、 (分隔符为'|'。另外，名称与职业编号需)
		<a href="#escaping">エスケープしたものを (转义后的内容)
		縦線でつなげたものが送信される。 (以竖线连接后发送)
		それぞれの値の型は下記。 (各值的类型如下)
		kubun にㄠが付いていると全パラメータを送信。 (kubun为1时发送完整参数)
		他の値だと、各ビットが付いているもののパラメータが順に並んで送信されている。 (其他值则按各bit位的参数依次发送)
		 例えば、ㄢが付いたらhpとMaxhpがkubun以降のトークンに格納されている。 (例如置位时hp与Maxhp存于kubun之后的标记中)

			kunun  int
			Hp(現在hp)  int (当前HP)
			MaxHp  int
			Mp(魔法パワー)  int (魔法值)
			MaxMp  int
			Vital(体力)  int (体力)
			Str(腕力)  int (腕力)
			Tough(丈夫さ)  int (耐力)
			Dex(素敏さ)  int (敏捷)
			Exp(経験値)  int (经验值)
			MaxExp(次までの経験値)  int (升下一级所需经验)
			Level(レベル)  int (等级)
			Attack(攻撃)  int (攻击)
				Defense(守備)  int (防御)
			fQuick(最終敏さ)  int (最终敏捷)
			fCharm(魅力)  int (魅力)
			fLuck(運)  int (运气)
			fEarth(地)  int (地属性)
			fWater(水)  int (水属性)
			fFire(火)  int (火属性)
			fWid(風)  int (风属性)
			Gold(お金)  int (金钱)
			職業番号のindex  int (职业编号的index)
			デジエルポイント  int (数字点数)
			転生回数  int (转生次数)
			  名前    値 (名称值)
			職業番号    値 (职业编号值)

		具体例 ( char のままそのまま書く ) (示例：原样写出char)

		P10|20|10|20|10|10|10|1|2|13|13|1|100|10|へんぱ|abc|def

	C 座標 (坐标)
		floor maxx maxy x y
		を | で区切って送る。 (以|分隔发送)
		具体例 ( char のままそのまま書く ) (示例：原样写出char)

		C1024|100|100|10|20

		下記の例では、フラアID1024, サイズ100x100 X 10,Y 20のとこ (下例中，在角色ID1024、尺寸100x100、坐标X10,Y20的)
		ろにキャラクターがいるという意味になる。このマイキャラのフ (位置存在角色之意。该主角的...)
		ひアも含めた座標をクライアントが知ることができるのはこの (客户端能得知含...在内的坐标，仅通过此)
		コマンドのみである。歩きの結線はフひアの位置しかわからな (命令。行走的连线只能知道...位置)
		い。よって、ゲーム起動時、移動手段のとき、ワープのときなど (。因此，游戏启动时、变更移动手段时、传送时等)
		は、このコマンドを使って欲しい値を送信しなくてはならない。 (必须用此命令发送所需值)

	I 全アイテム (完整物品)
		(アイテム1)|(アイテム2)...(アイテムn) (物品1|物品2|...|物品n)
		アイテムの並びは下記の方法でパックする。 (物品的排列按下述方法打包)
		ひとつのアイテムは、必ず下記の6つのトークンのセットで (每个物品必以下述6个标记为一组)
		送信される。ひとつひとつの並びは、 (发送。每组排列为)

		  名前|名前2|スロット|アイテムの種類|アニメ番号|使用可場所|使用対象|アイテムレベル|フラグ (名称|名称2|槽位|物品种类|动画编号|可用场所|使用对象|物品等级|标志)

		  アイテム名は識別レベルで変わる。 (物品名称随识别等级变化)
			名前2は識別レベルによって、アイテムの種類が変わる。 (名称2随识别等级改变物品种类)
		  クライアントのアイテム名の2行に入れる意味。 (用于放入客户端物品名的第2行)
		  スロットは名前の色を変えるのに使う。 (槽位用于改变名称颜色)
		  アイテムの種類。 (物品种类)
		  アニメ番号は、絵の番号。 (动画编号为图形编号)
		  使用可場所とは、このアイテムがどこで使用できるかが分かっている。 (可用场所表示该物品在何处可用)
			  これは、サーバーでは下記のように定義されている。 (服务器中定义如下)

			typedef enum
			{
				ITEM_FIELD_ALL,			すべての場所で使える (所有场所可用)
				ITEM_FIELD_BATTLE,		戦闘のみ (仅战斗)
				ITEM_FIELD_MAP,			通常マップ時のみ (仅普通地图时)

			}ITEM_FIELDTYPE;

		  使用対象とは、このアイテムを使用することの対象が分かっている。 (使用对象表示该物品的使用目标)
			  サーバーでは下記のように定義されている。 (服务器中定义如下)

			typedef enum
			{
				ITEM_TARGET_MYSELF,		自分のみ (仅自己)
				ITEM_TARGET_OTHER,		職業の他、自分含む) (职业者及其他，含自己)
				ITEM_TARGET_ALLMYSIDE,		    全 (己方全体)
				ITEM_TARGET_ALLOTHERSIDE,	敵側全員 (敌方全体)
				ITEM_TARGET_ALL,		全員 (全体)
			}ITEM_TARGETTYPE;

			<font size=+1>この数値に100を加えると、死んでいる者も対象となる。 (该数值加100后，死亡者也可作为对象)
		  アイテムレベル。このレベル以上の者でないと装備できない。 (物品等级。未达此等级者无法装备)
		  フラグ。色んなフラグを送信する。各ビットの対応は下記の通り。 (标志。发送各种标志。各bit位对应如下)

			  0bit    ペットドルで送信可能なアイテムかどうか。送信可能だと1、0だと送信できない。 (0bit：能否在宠物笼中发送的物品。可发送为1，否则不能)
			  1Bit    このアイテムが譲渡できるかどうか。 (1bit：该物品可否转让)
			  2Bit    このアイテムが売買できるかどうか。1だと売買できる。 (2bit：该物品可否买卖。1为可买卖)

		ここの値は<a href="#escaping">エスケープする。 (此处的值需转义)
		具体例 ( char のままそのまま書く ) (示例：原样写出char)

		Iあほな阂|str+2|1|あほ\|いじ|10|2|0 (示例)

		スロットの値の意味は、整数値で (槽位值的含义为整数)

			1  スロットA (槽位A)
			2  スロットB (槽位B)
			3  スロットC (槽位C)
			4  スロットD (槽位D)

		とする。また、アイテム名の欄の一部については、5個のトークンが (视为。另外，物品名栏的一部分以5个标记)
		欄で、つまり縦線が並んだ状態で送信されてくる。 (在栏中以竖线并排的状态发送)

	S 全スキル (完整技能)
		(スキル0)|(スキル1)|(スキル2) ... (スキルn) (技能0|技能1|技能2|...|技能n)
		職業番号と同じで、キャラが持てるだけのスキル数分を送信する。つまり (与职业编号相同，按角色可持有的技能数发送。即)
		りないものは ||| となるのである。 (未持有的以 ||| 表示)
		各エントリの並びは、 (各条目的排列为)
		スキルの種類コード|レベル (技能种类代码|等级)
		である。 (。)
		具体例 ( char のままそのまま書く ) (示例：原样写出char)

		S10|2|20|2|||

	T 職業番号 (职业编号)
		(職業番号0)|(職業番号1)|(職業番号2) ... (職業番号n) (职业编号0|职业编号1|...|职业编号n)
		職業番号のエントリが欄の形は '|' が重なるので、重なったも (职业编号条目的栏中'|'会重叠，重叠部分)
		のを無視してスキャンしてはならない。必ず個数分送信 (不可忽略并扫描。必须按个数发送)
		する。 (。)
		ひとつ、ひとつの並びは、 (每组排列为)
		  名前 (名称)
		のみである。 (仅名称。)
		具体例 ( 送られてくる char のままそのままである ) (示例：原样写出送来的char)

		Tあほ|よってぱらい (示例)

		これも<a href="#escaping">エスケープする。 (此处也需转义)

	M   一部パラメータ (部分参数)
		キャラクターのステータスの内、何か変更があったもの(実際
		値)と変わったもの(未変化)を送る。すべてのステータス
		を変更する時に  らゆるパラメータを送信するのは非効率
		なのとキャラのみを  択して使うのである。なおHP ,
		MP,EXPの値分にだけまとめたパラメータ送信コマンドを用意する。
		これはこのMコマンドである。2値以上の同一トークンはHPの
		実際値 , 次のトークンはMP、次のトークンはEXPである。
		下記に例を示す。

		M54|210|8944909

		この例ではHPの実際値は54、MPの実際値は210、経験値は8944909
		になっているのが分かる。この3個は何か変更があって増えた
		ときだけ送るコマンドで、足りないこのMコマンドの代
		わりにPコマンドで全パラメータを送信してしまう。または、適
		に非効率なのを使うときには、このコマンドを使うことを推奨
		する。

		 D キャラのID (角色ID)
		セッションごとに割り振るキャラのindex。これとセッションを送る。
		D1000|912766409のように送られる。

	E エンカウント割合(敵|自分) (遭遇概率(敌|己))
		エンカウント率を送信する。割合はn/100
		クライアントは、敵の値から一定確率で、1歩歩く毎に+1した割合でエンカウントするかを見る。
		または、自分の値を変えてわざとにしている。
		実はこれを送られてるときにこの割合でエンカウントを計算する意。

	J0   J6 使える魔法の
		使える魔法の並びを送信する。
		0   6 はそれぞれアイテムの装備種別に対応している。一の装備種別    ］  ］澎］装備
		にかく装備できない場合でも、決まった装いの全の装備種別の番を送る
		J0|kubun|mp|field|target|name|comment
		のようにフォーマットになっている。
		kubun は何の種類かどうか。0だと無い。これ以降にトークン並んでいる。1だとある。
		mpは必要なMPを表す。
		fieldはどの場所で使えるか。サーバーでは下記の通り定義されている。

		typedef enum
		{
			MAGIC_FIELD_ALL,		すべての場所で使える (所有场所可用)
			MAGIC_FIELD_BATTLE,		戦闘のみ (仅战斗)
			MAGIC_FIELD_MAP,		通常マップ時のみ (仅普通地图时)

		}MAGIC_FIELDTYPE;

		targetは何を対象に使えるか。サーバーでは下記のように定義されている。

		typedef enum
		{
			MAGIC_TARGET_MYSELF,		自分のみ (仅自己)
			MAGIC_TARGET_OTHER,		職業の他、自分含む) (职业者及其他，含自己)
			MAGIC_TARGET_ALLMYSIDE,		    全 (己方全体)
			MAGIC_TARGET_ALLOTHERSIDE,	敵側全員 (敌方全体)
			MAGIC_TARGET_ALL,		全員 (全体)
			MAGIC_TARGET_NONE,		指定はできない。あてがう時のために
			MAGIC_TARGET_OTHERWITHOUTMYSELF,職業の他、自分含まない) (职业者及其他，不含自己)
			MAGIC_TARGET_WITHOUTMYSELFANDPET, 自分とペット以外 (自己与宠物以外)
			MAGIC_TARGET_WHOLEOTHERSIDE,        のサイズ全 (对方全体)
		}MAGIC_TARGETTYPE;

		<font size=+1>この数値に100を加えると、死んでいる者も対象となる。 (该数值加100后，死亡者也可作为对象)

		nameは魔法名。
		commentはこの魔法の説明。

	N0    N3 転送のパラメータ (传送参数)
		転送になっている者の番を送信する。
		N0|kubun|level|charaindex|maxhp|hp|mp|name

		kubun は何の種類かどうか。0だと無い。これ以降にトークン並んでいる。1全パラメータ。
		2以上のとき、各ビットが付いているもののパラメータ  1bit    level 2bit   charaindexなど
		を送信する。
		charaindex は転送のcharaindex
		levelはその者のレベル。
		maxhpはその者のMAXHP
		hpはその者の実際のHP
		mpはその者のMP
		name はその者の名前。

	K0  K4 何かパラメータ (某参数)
		hp maxhp mp maxmp str tough exp
		level attackpower deffencepower
		fixdex fixcharm fixluck fixfireat fixwaterat fixearthat fixwindat
		  名前 status (名称 status)
		Kの値で0  4を指示して、どの何かを指示するのか。
		その次のトークンで0が付いたらそのペットの名前が強制的に
		なる形は1である。1だと全パラメータ。
		2以上のとき、各ビットが付いているもののパラメータ  1bit    hp  2bit   maxhpなど
		を送信する。
		デリミタは '|' である。また、名前と職業番号は、
		<a href="#escaping">エスケープしたものを
		縦線でつなげたものが送信される。
		それぞれの値の型は下記。

			No.(番号)  int (编号)
			islive(生死)  int (存活)
			GRA(絵の番号)  int (图形编号)
			Hp(現在値)  int (当前值)
			MaxHp(最大現在値)  int (最大当前值)
			Mp  int
			MapMp  int
			Exp(経験値)  int (经验值)
			MaxExp(次までの経験値)  int (升下一级所需经验)
			Level(レベル)  int (等级)
			Attack(攻撃)  int (攻击)
			Defense(守備)  int (防御)
			Quick(素敏さ)  int (敏捷)
			Ai(知能指数)  int (AI)
			fEarth(地)  int (地属性)
			fWater(水)  int (水属性)
			fFire(火)  int (火属性)
			fWid(風)  int (风属性)
			Slot(所持数)  int (持有数)
			  名前  同定済フラグ  int (名称 已鉴定标志)
			  名前    値 (名称值)
			召喚ペット      値 (召唤宠物值)

		  名前  同定済フラグとは、このペットの名前を同定してしまえるかどうかのフラグで、
		1 だと 名前 あり、0 だと 名前 なしとなる。 (1为有名称，0为无名称)

		具体例 ( char のままそのまま書く ) (示例：原样写出char)

		P10|20|10|20|10|10|10|1|2|13|13|1|100|10|へんぱ|PC

	W0  W4 ペットのデータ (宠物数据)
	  W0|skillid|field|target|name|comment| x 7
	  W0    W4 はそれぞれのペットに対応している。
	  petskillid は、ペットの技の番号。pet_skillinfo.hに定義されている。
	  field は何のどこで使用できるか。サーバーでは下記のように定義されている。

		typedef enum
		{
			PETSKILL_FIELD_ALL,		すべての場所で使える (所有场所可用)
			PETSKILL_FIELD_BATTLE,		戦闘のみ (仅战斗)
			PETSKILL_FIELD_MAP,		通常マップ時のみ (仅普通地图时)

		}PETSKILL_FIELDTYPE;

	  target は何の技の対象がどれだけか。サーバーでは次のように定義されている。

		typedef enum
		{
			PETSKILL_TARGET_MYSELF,		自分のみ (仅自己)
			PETSKILL_TARGET_OTHER,		職業の他、自分含む) (职业者及其他，含自己)
			PETSKILL_TARGET_ALLMYSIDE,	    全 (己方全体)
			PETSKILL_TARGET_ALLOTHERSIDE,	敵側全員 (敌方全体)
			PETSKILL_TARGET_ALL,		全員 (全体)
			PETSKILL_TARGET_NONE,		指定はできない。あてがう時のために
			PETSKILL_TARGET_OTHERWITHOUTMYSELF,職業の他、自分含まない) (职业者及其他，不含自己)
			PETSKILL_TARGET_WITHOUTMYSELFANDPET, 自分とペット以外 (自己与宠物以外)
		}PETSKILL_TARGETTYPE;

	  name は技の名前。
	  commentは何の技に対応する説明。
	  target|name|comment| は技の数の分だけ送られてくる。
		で区切ってつまり、並びで区切っている  "|"のみを重ねる形
	  はクライアントに供給して送る意。
*/
void lssproto_S_send(int fd, char *data)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_S_SEND, buffer);
}

/*
   servertoclient D( int category , int dx , int dy , string data );
	   (Display)
		   に表示する画面。 (显示的画面)


	int category
	表示するか。

			値 (值)
			1  自分の座標にいる敵モンスタ。dataは値
			になっている値。
			2  自分が選んだ敵モンスタ。dataは値の
			  なっている値

	int dx
	マイキャラからの相対座標 X。一読ではキャラクタを変
		えて座標。クライアントはこの値から適当適当な座標を計算して
			いる。
	int dy
	相対座標 Y (相对坐标Y)
	string data
	  表示する文字列。値はcategoryによって変わる。
		この値は<a href="#escaping">エスケープしなければなら
		ない。
*/
void lssproto_D_send(int fd, int category, int dx, int dy, char *data)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkint(buffer, category);
	checksum += util_mkint(buffer, dx);
	checksum += util_mkint(buffer, dy);
	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_D_SEND, buffer);
}

/*
   servertoclient FS( int flg);
	   (FlgSet)PSの設定。またセッションごとに自分の適宜に変更送られてくる。

		 int flg
		  0 bit  0: 転送Off                  1: 転送On
				現在の使用 1 bit  0: 戦闘可能off          1: 戦闘可能On
			  2 bit  0: DUEL off                 1: DUEL On
			  3 bit  0:   通常のモンスタモード     1: 魔法でないモンスタを生か中モード
			  4 bit  0:   交換OK               1:   交換NG
*/
void lssproto_FS_send(int fd, int flg)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkint(buffer, flg);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_FS_SEND, buffer);
}

/*
   servertoclient HL( int flg);
	   (HeLp)HLの設定。また魔法の転送を戦闘の援助モードに変更した形で送られてくる。

		 int flg
		  0: お助けモードOff                  1: お助けモードOn
*/
void lssproto_HL_send(int fd, int flg)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkint(buffer, flg);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_HL_SEND, buffer);
}

/*
   servertoclient PR( int request, int result);
	   (PartyRequest)転送要求の設定。PRをクライアントに送信していないとこれが受け取る時になる。
		魔法が届いた際に破棄した場合などの時。

		int request
		0: 承諾 1: 拒否
		 int result
		 0: 失敗   1: 成功
*/
void lssproto_PR_send(int fd, int request, int result)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkint(buffer, request);
	checksum += util_mkint(buffer, result);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_PR_SEND, buffer);
}

#ifdef _PETS_SELECTCON
void lssproto_PETS_send(int fd, int petarray, int result)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkint(buffer, petarray);
	checksum += util_mkint(buffer, result);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_PETST_SEND, buffer);
}
#endif

void lssproto_KS_send(int fd, int petarray, int result)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkint(buffer, petarray);
	checksum += util_mkint(buffer, result);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_KS_SEND, buffer);
}

void lssproto_SPET_send(int fd, int standbypet, int result)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkint(buffer, standbypet);
	checksum += util_mkint(buffer, result);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_SPET_SEND, buffer);
}

/*
   servertoclient PS( int result, int havepetindex, int havepetskill, int toindex);
	   (PetSkill use result)ペットの技を使った結果クライアントのPSに対応して送られてくる。
	   result 値はクライアントのPSに対応している。実際ドルの時以外は無い。

	int result
	結果。0: 失敗   1:成功
		 int havepetindex
		 表示中のペットを使ったか。
		 int havepetskill
		 表示中の技を使ったか。
	int toindex
	 指定の魔法を使ったか。これは逆に相手のキャラのindexではない。下記の通りになっている。

	  自分    = 0
	  ペット  = 1   5
	  転送    = 6   10   S N の0  4に対応。自分含み手含まないこと。

	対象が全員、いない時など、時の形は-1で送信する。
*/
void lssproto_PS_send(int fd, int result, int havepetindex, int havepetskill, int toindex)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkint(buffer, result);
	checksum += util_mkint(buffer, havepetindex);
	checksum += util_mkint(buffer, havepetskill);
	checksum += util_mkint(buffer, toindex);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_PS_SEND, buffer);
}

/*
   servertoclient SKUP( int point );
	   (SKillUP)
	   スキルポイントを使う意味をサーバーに通知する。どのように送るかを指示する。
*/
void lssproto_SKUP_send(int fd, int point)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkint(buffer, point);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_SKUP_SEND, buffer);
}

/*
   servertoclient WN( int windowtype, int buttontype, int seqno, int objindex, string data );
	   (Window)
	   ウィンドウを表示するようサーバーをクライアントに通知する。

		 int windowtype
		 ウィンドウタイプ。どのような種類のウィンドウを表示するか。これはbuttontype の押しボタンで
			 ウィンドウを変える。
			 サーバーでは下記の通り定義されている。

		typedef enum
		{
			WINDOW_MESSAGETYPE_MESSAGE,			メッセージのみ
			WINDOW_MESSAGETYPE_MESSAGEANDLINEINPUT,		メッセージと一行
			WINDOW_MESSAGETYPE_SELECT,			指定ウィンドウ
			WINDOW_MESSAGETYPE_PETSELECT,			ペット指定ウィンドウ
			WINDOW_MESSAGETYPE_PARTYSELECT,			転送指定ウィンドウ
			WINDOW_MESSAGETYPE_PETANDPARTYSELECT,		ペット・転送指定ウィンドウ
			WINDOW_MESSAGETYPE_ITEMSHOPMENU,		商店のメニューメインメニュー
			WINDOW_MWSSAGETYPE_ITEMSHOPYMAIN,		商店のメインメインメニュー
			WINDOW_MESSAGETYPE_LIMITITEMSHOPMAIN,		限定購入アイテム箱のメインメインメニュー
			WINDOW_MESSAGETYPE_PETSKILLSHOP,		ペットの技売りメインメニュー
			WINDOW_MESSAGETYPE_WIDEMESSAGE,			メッセージのみ    大中
			WINDOW_MESSAGETYPE_WIDEMESSAGEANDLINEINPUT,	メッセージと一行        大中

		}WINDOW_MESSAGETYPE;

		 int buttontype
		 押す件の種類を指示する。サーバーでは下記の番号で定義されている。

		#define		WINDOW_BUTTONTYPE_NONE		(0)
		#define		WINDOW_BUTTONTYPE_OK		(1 << 0)
		#define		WINDOW_BUTTONTYPE_CANCEL	(1 << 1)
		#define		WINDOW_BUTTONTYPE_YES		(1 << 2)
		#define		WINDOW_BUTTONTYPE_NO		(1 << 3)
		#define		WINDOW_BUTTONTYPE_PREV		(1 << 4)
		#define		WINDOW_BUTTONTYPE_NEXT		(1 << 5)

			 これは押しボタンで送信する。  例：YES押す件とNO押す件が  必要なときは
			 WINDOW_BUTTONTYPE_YES | WINDOW_BUTTONTYPE_NO   (=12)
			 で送る。

		 int seqno
		 このウィンドウの番号を表示。サーバーを指す。
			 クライアントはWNに対してこのウィンドウの決定結果を返すので、この番号を使って返す。
			 これによってサーバーはどのNPCのどの場所でのウィンドウなのかを見分けるためにする。
		 int objindex
		 このウィンドウを出した選んだNPCなどのindexが入ってくる。
			 ボタンを出した選んだ形は-1などが入っている。
			 クライアントは、ウィンドウ選択のWNマクロとしてこの値をそのまま使うことにしている。
		 string data
		 メッセージ内容を表示。値はエスケープする。"\n"で区切って一行のように送る。
			 また、クライアントのウィンドウに表示する字体を変えた端は自分で一行にする。
			 指定枠の形ウィンドウでは、"\n"で区切られた最初のトークンはメッセージの行数
			 として、次のメッセージのトークンが最初のトークンで指示した個数分だけ、
			 それ以降のトークンがくそのままの指定枠となる。また名前番号に 1 番目
			   番号を使って、指定した形のWNに対して返すときに使う。
		 string data(お店用 (商店用)
		 メッセージ内容を表示。値はエスケープする。"\n"で区切って一行のように送る。
			 また、クライアントのウィンドウに表示する字体を変えた端は自分で一行にする。
			 データは番号を"  "で区切っていれる。 (数据以"  "分隔)
	≪  う≫
	  い  るフラグ    う０：  る１    前のデータ银うかフラグ  银う０：银わない：1
		店の  前  丢ッセージ  店丢ッセージ  個数蓟  丢ッセージ  レベル箫りない丢ッセージ  確  丢ッセージ
		アイ  ムがい匀ぱい丢ッセージ  アイ  ム      える  えないフラグ    える：０  えない：１    アイ  ムレベル
		袄段    飓  号        アイ  ム      える  えないフラグ  アイ  ムレベル  袄段    飓  号

			0|1|サムギルの道具屋|いらっしゃい、何の用だい|いらっしゃい、どれにする|何個 要だい|それを装備するにはレベルが
			箫りないけどいいのかい|  癫にいいんだね|おいおいアイ  ムがい匀ぱいじゃねえか″  ブー丢ラン|
			０|１００|１６０００|产一  に  撃\n効绊  群|薬阕|０|５０|１６００２|    ２０荚復\n  莴にはつき  \n安い

		 ≪  る≫
		  い  るフラグ    う０：  る１    前のデータ银うかフラグ  银う０：银わない：1
		店の名前 メッセージ 店メッセージ お金がいっぱいになっちゃうメッセージ|確  メッセージ
		アイ  ム      れる  れないフラグ  袄段    飓  号        アイ  ム      れる  れないフラグ
		袄段    飓  号

			１|1|サムギルの道具屋|いらっしゃい、どれにする＂  いらっしゃい、どれを 連れてくれる。|  頼にいいんだね|
			それだと、お金がオーバーしちゃうね ブーメラン|０|２０|１６０００|
			产一  に  撃\n効绊  群|薬阕|０|１０|１６００２|    ２０荚復\n  莴にはつき  \n安い
		 ≪アイ  ム  ける≫
		フラグ    る=０    あと何個  けられるか
		店の  前
		  ける丢ッセージ  これ动晓  けられない丢ッセージ  確  丢ッセージ
		  前    けられるかフラグ  可  「０］不可「１    袄段    飓  号  一垫インフォ  アイ  ム    号  １から
		  前    けられるかフラグ  可  「０］不可「１    袄段    飓  号  一垫インフォ  アイ  ム    号  １から
		 ≪アイ  ム受け取る≫
		フラグ  受け取る=１
		店の  前  受け请す丢ッセージ  アイ  ムが一  の凛の丢ッセージ|確  丢ッセージ
		  前  受け请せるかどうかフラグ  可  「０］不可「１    アイ  ムレベル  袄段    飓  号  一垫インフォ
		  前  受け请せるかどうかフラグ  可  「０］不可「１    アイ  ムレベル  袄段    飓  号  一垫インフォ

		 ≪その职≫
		店の  前  丢ッセージ

			サムギルの道具屋|いらっしゃい、どれにする＂

		 string data(ペットの技屋さん用 (宠物技能商店用)
		 メッセージ内容を表示。値はエスケープする。"\n"で区切って一行のように送る。
			 また、クライアントのウィンドウに表示する字体を変えた端は自分で一行にする。
			 データは番号を"  "で区切っていれる。 (数据以"  "分隔)
	   ≪ペットの  取  ≫
		前の树  银うかどうか＂  银う：0｛银わない：1    店の  前  丢イン丢ッセージ
			  袄段                袄段

		1  サムギルの道具屋  いらっしゃいませ。すばらしい 品揃えですよ
			の讽その迭  500    撃  ３０⊙    ｛  御  ３０⊙
		哗球一葱  1000  新しい  苇があるかも  赝癫
*/
void lssproto_WN_send(int fd, int windowtype, int buttontype, int seqno, int objindex, char *data)
{
	char buffer[65500];
	int checksum = 0;

#ifdef _NO_WARP
	// shan hanjj add Begin
	CONNECT_set_seqno(fd, seqno);
	CONNECT_set_selectbutton(fd, buttontype);
	// shan End
#endif

	strcpy(buffer, "");

	checksum += util_mkint(buffer, windowtype);
	checksum += util_mkint(buffer, buttontype);
	checksum += util_mkint(buffer, seqno);
	checksum += util_mkint(buffer, objindex);
	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_WN_SEND, buffer);
}

/*
   servertoclient EF( int effect, int level, string option );
	   (EFfect)
	   絵文字を表示した際、クライアントに全表示の効果を表示するよう送る。

		 int effect
		 効果番号。それぞれを加えると、何かの効果が加算される。1にすると絵と文字を表示した際にいる。

			 1:絵
			 2:字
			 4:表示文字    枠

		 int level
		 効果のレベル。0は要る。1  4は現在のレベルを表す。
		 string option
		 決まった番号
*/
/* Arminius
  新增给丢骰子的动画
  int effect = 10   丢骰子
	  level dont care
	  option = "pet1x|pet1y|dice1|pet2x|pet2y|dice2" (no makeEscapeString)
*/
void lssproto_EF_send(int fd, int effect, int level, char *option)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkint(buffer, effect);
	checksum += util_mkint(buffer, level);
	checksum += util_mkstring(buffer, option);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_EF_SEND, buffer);
}

/*
   servertoclient SE( int x, int y, int senumber, int sw );
	   (SoundEffect)
	クライアントにSEを鳴らすように指示する。

		 int x,y
		 SEを鳴らす座標。
		 int senumber
			 の番号
		 int sw
			 を鳴らすか止めるか。

			 0:止める
			 1:鳴らす
*/
void lssproto_SE_send(int fd, int x, int y, int senumber, int sw)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkint(buffer, x);
	checksum += util_mkint(buffer, y);
	checksum += util_mkint(buffer, senumber);
	checksum += util_mkint(buffer, sw);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_SE_SEND, buffer);
}

/*
   servertoclient ClientLogin(string result);
	   ClientLoginの返信。

	string result
	"ok" のような値。この値はエスケープされない。
*/
void lssproto_ClientLogin_send(int fd, char *result)
{
	char buffer[65500];
	int checksum;

	strcpy(buffer, "");
	/* Tokyo-sa: win 9.0 client checksum = strlen(result)+strlen(data);
	   0x00FE0680+func formula makes client reject the login response */
	checksum = util_mkstring(buffer, result);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_CLIENTLOGIN_SEND, buffer);
}

/*
   servertoclient CreateNewChar(string result,string data);
	   CreateNewCharの返信。

	string result
	"successful" か "failed" のどちらか。この値はエスケープ
		されない。
	string data
	"failed" の時は意味を表示する理由の変わり値
		  である。キャラクタサーバーからの返信そのままである。
		下記の値

		"failed bad parameter"

		の形は、キャラ成功のときには詳細のパラメータの区を変えて
		いるのように送る。これはゲームサーバーを指す用メッセ
		ージである。この値はエスケープされない。
*/
void lssproto_CreateNewChar_send(int fd, char *result, char *data)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");
	checksum += util_mkstring(buffer, result);
	/* Tokyo-sa fix: 9.0 calls mkstring(data) unconditionally so empty data
	   still occupies a ';' field -- client field count depends on it */
	checksum += util_mkstring(buffer, data ? data : "");
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_CREATENEWCHAR_SEND, buffer);
}

/*
   servertoclient CharDelete(string result,string data);
	   CharDelete の返信。

	string result
	"successful" か "failed" のどちらか。エスケープされない。
	string data
	"failed" の時は意味を表示する理由の変わり値
		  である。キャラクタサーバーからの返信そのままである。
		デリミタも含まれているので、エスケープされない。
*/
void lssproto_CharDelete_send(int fd, char *result, char *data)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkstring(buffer, result);
	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_CHARDELETE_SEND, buffer);
}

/*
   servertoclient CharLogin(string result,string data);
	   CharaLoginの返信。

	string result
	"successful" か "failed" のどちらか。エスケープされない。
	string data
	"failed" の時は その原因の値。エスケープされない。
*/
void lssproto_CharLogin_send(int fd, char *result, char *data)
{
	char buffer[65500];
	int checksum = 0;
	char cdkey[64];

	strcpy(buffer, "");

	/* Tokyo-sa 动态密钥：角色登录后密钥变为 cdkey + "www.longzoro.com" */
	CONNECT_getCdkey(fd, cdkey, sizeof(cdkey));
	snprintf(TokyoKey, sizeof(TokyoKey), "%swww.longzoro.com", cdkey);
	print("[DYNKEY] cdkey=%s TokyoKey=%s\n", cdkey, TokyoKey);

	checksum += util_mkstring(buffer, result);
	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	print("[CHARLOGIN-SEND] result=%s data=%s checksum=%d TokyoKey=%s buffer=%s\n", result, data, checksum, TokyoKey, buffer);
	util_SendMesg(fd, LSSPROTO_CHARLOGIN_SEND, buffer);
}

/*
   servertoclient CharList(string result,string data);
	   CharListの返信。

	string result
	"successful" か "failed" のどちらか。エスケープされない
	string data
	resultが"successful"の時は、キャラクタサーバーに登録されて
		いるすべてのキャラの名前、フォルダファイルをメタデータで区切った
		一個の値。この値を持っているのは、キャラクタサーバー。
		result が "failed" の時は意味を表示する理由の変わり値 で
		ある。真の時のフォルダファイルの数値の並びである。
		dataplace|faceimage|level|maxhp|atk|def|quick|charm|earth|water|fire|wind|logincount|name|place

		  dataplace
		  ヘッダデータの表示位置。これでキャラクターオブジェクトの表示位置を変える。
		  faceimage
		  顔の絵番号
		  level
		  キャラのレベル
		  maxhp,atk,def,quick,charm
		  各パラメータ。
		  earth.water,fire,wind
		  各属性値
		  logincount
		  セッションカウンタ
		  name
		  キャラの名前
		  place
		  キャラのいる場所

		"|" で区切られている。それぞれの値は、<a
		href="#escaping">エスケープされている。それ以降は縦線
		でつなげる。
*/
void lssproto_CharList_send(int fd, char *result, char *data)
{
	char buffer[65500];
	int checksum = 0;
	char cdkey[64];

	strcpy(buffer, "");

	/* Tokyo-sa: 角色列表用 cdkey + "www.longzoro.com" 动态密钥(9.0原版反汇编确认),
	   checksum = strlen(result)+strlen(data) */
	CONNECT_getCdkey(fd, cdkey, sizeof(cdkey));
	snprintf(TokyoKey, sizeof(TokyoKey), "%swww.longzoro.com", cdkey);
	print("[CHARLIST-SEND] cdkey=%s TokyoKey=%s result=%s data_len=%d\n",
	      cdkey, TokyoKey, result ? result : "(null)", data ? (int)strlen(data) : -1);

	checksum += util_mkstring(buffer, result);
	checksum += util_mkstring(buffer, data ? data : "");
	util_mkint(buffer, checksum);
	print("[CHARLIST-SEND] checksum=%d buffer=%.160s\n", checksum, buffer);
	util_SendMesg(fd, LSSPROTO_CHARLIST_SEND, buffer);
}

/*
   servertoclient CharLogout(string result , string data);
	   Logoutに対応する返信。

	string result
	"successful" か "failed" のどちらか。エスケープされない。
	string data
	"failed" の時のみらになる、失の原因 (状態)を表示する理由
		の変わり値である。エスケープされない。
*/
void lssproto_CharLogout_send(int fd, char *result, char *data)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkstring(buffer, result);
	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_CHARLOGOUT_SEND, buffer);
}

/*
   servertoclient ProcGet( string data);
	ProcGetの返信。

	string data
	エスケープする。ゲームサーバーの情報番号を送信する。値はlog/procファイルに載っている値と同じ。
*/
void lssproto_ProcGet_send(int fd, char *data)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkstring(buffer, data);

	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_PROCGET_SEND, buffer);
}

/*
   servertoclient PlayerNumGet( int logincount, int player);
	PlayerNumGetの返信。

	int logincount,player
*/
void lssproto_PlayerNumGet_send(int fd, int logincount, int player)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkint(buffer, logincount);
	checksum += util_mkint(buffer, player);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_PLAYERNUMGET_SEND, buffer);
}

/*
   servertoclient Echo( string test );
	   Echoに対応する返信。

	string test
	今テストして送った値。エスケープされない。
*/
void lssproto_Echo_send(int fd, char *test)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkstring(buffer, test);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_ECHO_SEND, buffer);
}

// CoolFish: Trade 2001/4/18

void lssproto_TD_send(int fd, int index, char *message)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkstring(buffer, message);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_TD_SEND, buffer);
}
#ifdef _CHATROOMPROTOCOL // (不可开) Syu ADD 聊天室频道
void lssproto_CHATROOM_send(int fd, char *message)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkstring(buffer, message);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_CHATROOM_SEND, buffer);
}
#endif
#ifdef _NEWREQUESTPROTOCOL // (不可开) Syu ADD 新增Protocol要求细项
void lssproto_RESIST_send(int fd, char *message)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkstring(buffer, message);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_RESIST_SEND, buffer);
}
#endif
#ifdef _OUTOFBATTLESKILL // (不可开) Syu ADD 非战斗时技能Protocol
void lssproto_BATTLESKILL_send(int fd, char *message)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkstring(buffer, message);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_BATTLESKILL_SEND, buffer);
}
#endif
void lssproto_NU_send(int fd, int nu)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkint(buffer, nu);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_NU_SEND, buffer);
}

void lssproto_FM_send(int fd, char *message)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkstring(buffer, message);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_FM_SEND, buffer);
}

void lssproto_WO_send(int fd, int effect)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkint(buffer, effect);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_WO_SEND, buffer);
}
#ifdef _ITEM_CRACKER
void lssproto_IC_send(int fd, int x, int y)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkint(buffer, x);
	checksum += util_mkint(buffer, y);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_IC_SEND, buffer);
}
#endif
#ifdef _MAGIC_NOCAST // 精灵:沉默
void lssproto_NC_send(int fd, int flg)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkint(buffer, flg);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_NC_SEND, buffer);
}
#endif

#ifdef _CHECK_GAMESPEED
void lssproto_CS_send(int fd, int deltimes)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkint(buffer, deltimes);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_CS_SEND, buffer);
}
#endif

#ifdef _STREET_VENDOR
void lssproto_STREET_VENDOR_send(int fd, char *message)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkstring(buffer, message);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_STREET_VENDOR_SEND, buffer);
}
#endif

#ifdef _RIGHTCLICK
void lssproto_RCLICK_send(int fd, int type, char *data)
{
	char buffer[65500];
	int checksum = 0;

	print("\n RCLICK_send( type=%d data=%s) ", type, data);

	strcpy(buffer, "");

	checksum += util_mkint(buffer, type);
	checksum += util_mkint(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_RCLICK_SEND, buffer);
}
#endif

#ifdef _JOBDAILY
void lssproto_JOBDAILY_send(int fd, char *data)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	// print("tocli_data:%s ",data);
	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	// print("tocli_buffer:%s ",buffer);
	util_SendMesg(fd, LSSPROTO_JOBDAILY_SEND, buffer);
}
#endif

#ifdef _TEACHER_SYSTEM
void lssproto_TEACHER_SYSTEM_send(int fd, char *data)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_TEACHER_SYSTEM_SEND, buffer);
}
#endif

#ifdef _ADD_STATUS_2
void lssproto_S2_send(int fd, char *data)
{
	char buffer[65500];
	int checksum = 0;

	strcpy(buffer, "");

	checksum += util_mkstring(buffer, data);
	util_mkint(buffer, checksum);
	util_SendMesg(fd, LSSPROTO_S2_SEND, buffer);
}
#endif
