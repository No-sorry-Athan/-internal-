#pragma once
#include "valve_sdk/csgostructs.hpp"
#include "valve_sdk/sdk.hpp"
#include <deque>

class FakeLatency {
public:
	void AddLatencyToNetchan(INetChannel* netchan, float latency);
	void UpdateIncomingSequences();
}; extern FakeLatency fakelatency;

struct CIncomingSequence
{
	CIncomingSequence::CIncomingSequence(int instate, int outstate, int seqnr, float time)
	{
		inreliablestate = instate;
		outreliablestate = outstate;
		sequencenr = seqnr;
		curtime = time;
	}
	int inreliablestate;
	int outreliablestate;
	int sequencenr;
	float curtime;
};




