/*
 *
 *  RISE OF BOTS
 *
 *  OBJECTS_H
 *
 *
 */

#ifndef _AIBOTS_H
#define _AIBOTS_H

#ifdef __cplusplus
extern "C"
{
#endif
int Init(char *);
int Move(int,int);
void Stop(void);
int Fire(int,int);
int GetInfoEdan(int ,int *,int *,char *,char *,int *,int *,unsigned long *,unsigned long *);
int GetInfoEnemy(int ,int *,int *,char *,int *);
int GetInfoNost(int,int *,int *,int *,int *,int *,int *,unsigned long *);
int LocateResource(char ,int *,int *);
int ScanEdan(int,int *,int *,char *,char *,int*);
int ScanNost(int,int *,int *,char *);
int ScanResource(int,int *,int *,char *);
int IsWall(int,int);
int PutResource(int,int);
int PassResourceToEdan(int);
int PassResourceToNost(int);
int GetResourceFromNost(int,char);
int MsgSend(int,char *);
void BroadCast(char *);
int MsgRecv(char *);
int NewMsgCount(void);
int MakeNost(int,int,int);
int UpgradeEdan(int);
int PowerEdan(int,char);
int HavePower(char);
int UpgradeNost(int,char);
#ifdef __cplusplus
}
#endif

#define EDAN 0
#define NOST 2

#define VASA 101
#define TATHAR 102
#define MALDA 103

#define MAPX 25
#define MAPY 25

#define TURBO 91
#define RANGEDBOMB 92
#define GOLDENEYE 93

#define SPEED 94
#define BALLISTO 95
#define SHIELD 96

#ifndef NULL
#define NULL 0L
#endif

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

#endif /*_AIBOTS_H*/

