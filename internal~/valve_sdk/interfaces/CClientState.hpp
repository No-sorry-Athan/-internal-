#pragma once

#include <cstdint>
#include "netadr.hpp"
#include "bfwrite.hpp"

class Netmsgbinder;
class INetChannel;

// Created with ReClass.NET by KN4CK3R
#pragma pack(push, 1)

class INetChannelHandler
{
public:
	virtual	~INetChannelHandler(void) {};

	virtual void ConnectionStart(INetChannel* chan) = 0;	// called first time network channel is established

	virtual void ConnectionClosing(const char* reason) = 0; // network channel is being closed by remote site

	virtual void ConnectionCrashed(const char* reason) = 0; // network error occured

	virtual void PacketStart(int incoming_sequence, int outgoing_acknowledged) = 0;	// called each time a new packet arrived

	virtual void PacketEnd(void) = 0; // all messages has been parsed

	virtual void FileRequested(const char* fileName, unsigned int transferID) = 0; // other side request a file for download

	virtual void FileReceived(const char* fileName, unsigned int transferID) = 0; // we received a file

	virtual void FileDenied(const char* fileName, unsigned int transferID) = 0;	// a file request was denied by other side

	virtual void FileSent(const char* fileName, unsigned int transferID) = 0;	// we sent a file
};

class INetChannel
{
public:
	__int32 vtable; //0x0000 
	Netmsgbinder* msgbinder1; //0x0004 
	Netmsgbinder* msgbinder2; //0x0008 
	Netmsgbinder* msgbinder3; //0x000C 
	Netmsgbinder* msgbinder4; //0x0010 
	unsigned char m_bProcessingMessages; //0x0014 
	unsigned char m_bShouldDelete; //0x0015 
	char pad_0x0016[0x2]; //0x0016
	__int32 m_nOutSequenceNr; //0x0018 
	__int32 m_nInSequenceNr; //0x001C 
	__int32 m_nOutSequenceNrAck; //0x0020 
	__int32 m_nOutReliableState; //0x0024 
	__int32 m_nInReliableState; //0x0028 
	__int32 m_nChokedPackets; //0x002C 

	bf_write m_StreamReliable; //0x0030 
	/*CUtlMemory*/ char m_ReliableDataBuffer[12]; //0x0048 
	bf_write m_StreamUnreliable; //0x0054 
	/*CUtlMemory*/ char m_UnreliableDataBuffer[12]; //0x006C 
	bf_write m_StreamVoice; //0x0078 
	/*CUtlMemory*/char m_VoiceDataBuffer[12]; //0x0090 
	__int32 m_Socket; //0x009C 
	__int32 m_StreamSocket; //0x00A0 
	__int32 m_MaxReliablePayloadSize; //0x00A4 
	char pad_0x00A8[0x4]; //0x00A8
	netadr_t remote_address; //0x00AC 
	char dylanpadding[84]; //padding added by dylan
	float last_received; //0x00B8  //dylan found 0x10c
	//char pad_0x00BC[0x4]; //0x00BC
	double /*float*/ connect_time; //0x00C0 //dylan found 0x110
	//char pad_0x00C4[0x4]; //0x00C4
	__int32 m_Rate; //0x00C8  //dylan found 0x118
	/*float*/double m_fClearTime; //0x00CC  //dylan found 0x120
	char pad_0x00D0[0x8]; //0x00D0
	char m_WaitingList[48];
	//CUtlVector m_WaitingList[0]; //0x00D8 
	//CUtlVector m_WaitingList[1]; //0x00EC 
	//char pad_0x0100[0x4120]; //0x0100
	char pad_0x0100[0x40F0]; //dylan changed
	__int32 m_PacketDrop; //0x4220  //dylan found 0x4250
	char m_Name[32]; //0x4224 
	__int32 m_ChallengeNr; //0x4244 
	float m_Timeout; //0x4248 //dylan found 0x4278
	INetChannelHandler* m_MessageHandler; //0x424C 
	/*CUtlVector*/char m_NetMessages[14]; //0x4250 
	__int32 dylanUnknown;
	void* m_pDemoRecorder; //0x4264 
	__int32 m_nQueuedPackets; //0x4268  //dylan found 0x4298
	float m_flInterpolationAmount; //0x426C //dylan found 0x429c
	float m_flRemoteFrameTime; //0x4270 //dylan found 0x42a0
	float m_flRemoteFrameTimeStdDeviation; //0x4274  //dylan found 0x42a4
	float m_flRemoteFrameTimeUnknown; //dylan found 0x42a8
	__int32 m_nMaxRoutablePayloadSize; //0x4278  //dylan found 0x42ac
	__int32 m_nSplitPacketSequence; //0x427C  //dylan found 0x42b0
	char pad_0x4280[0x14]; //0x4280
};//Size=0x4294

class CClockDriftMgr
{
public:
	float m_ClockOffsets[16];   //0x0000
	uint32_t m_iCurClockOffset; //0x0044
	uint32_t m_nServerTick;     //0x0048
	uint32_t m_nClientTick;     //0x004C
}; //Size: 0x0050

class CClientState
{
public:
	void ForceFullUpdate()
	{
		*(uint32_t*)((uintptr_t)this + 0x174) = -1;
	}

	char pad_0000[156];             //0x0000
	uint32_t m_NetChannel;          //0x009C
	uint32_t m_nChallengeNr;        //0x00A0
	char pad_00A4[100];             //0x00A4
	uint32_t m_nSignonState;        //0x0108
	char pad_010C[8];               //0x010C
	float m_flNextCmdTime;          //0x0114
	uint32_t m_nServerCount;        //0x0118
	uint32_t m_nCurrentSequence;    //0x011C
	char pad_0120[8];               //0x0120
	CClockDriftMgr m_ClockDriftMgr; //0x0128
	uint32_t m_nDeltaTick;          //0x0178
	bool m_bPaused;                 //0x017C
	char pad_017D[3];               //0x017D
	uint32_t m_nViewEntity;         //0x0180
	uint32_t m_nPlayerSlot;         //0x0184
	char m_szLevelName[260];        //0x0188
	char m_szLevelNameShort[40];    //0x028C
	char m_szGroupName[40];         //0x02B4
	char pad_02DC[52];              //0x02DC
	uint32_t m_nMaxClients;         //0x0310
	char pad_0314[18820];           //0x0314
	float m_flLastServerTickTime;   //0x4C98
	bool insimulation;              //0x4C9C
	char pad_4C9D[3];               //0x4C9D
	uint32_t oldtickcount;          //0x4CA0
	float m_tickRemainder;          //0x4CA4
	float m_frameTime;              //0x4CA8
	uint32_t lastoutgoingcommand;   //0x4CAC
	uint32_t chokedcommands;        //0x4CB0
	uint32_t last_command_ack;      //0x4CB4
	uint32_t command_ack;           //0x4CB8
	uint32_t m_nSoundSequence;      //0x4CBC
	char pad_4CC0[80];              //0x4CC0
	Vector viewangles;              //0x4D10
}; //Size: 0x4D1C
#pragma pack(pop)
