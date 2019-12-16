#include "FakeLatency.hpp"
#include "options.hpp"

static std::deque<CIncomingSequence> sequences;
static int lastincomingsequencenumber;

void FakeLatency::AddLatencyToNetchan(INetChannel* netchan, float Latency)
{
	for (auto& seq : sequences)
	{
		if (g_GlobalVars->realtime - seq.curtime >= Latency)
		{
			netchan->m_nInReliableState = seq.inreliablestate;
			netchan->m_nOutReliableState = seq.outreliablestate;
			netchan->m_nInSequenceNr = seq.sequencenr;
			break;
		}
	}
}

void FakeLatency::UpdateIncomingSequences() {
	if (g_ClientState)
	{
		INetChannel* netchan = (INetChannel*)g_ClientState->m_NetChannel;

		if (netchan)
		{
			if (g_LocalPlayer->m_nSequence() == 0)
			{
				g_LocalPlayer->m_nSequence() = netchan->m_nInSequenceNr;
				lastincomingsequencenumber = netchan->m_nInSequenceNr;
			}

			if (netchan->m_nInSequenceNr >= lastincomingsequencenumber)
			{
				lastincomingsequencenumber = netchan->m_nInSequenceNr;
				sequences.push_front(CIncomingSequence(netchan->m_nInReliableState, netchan->m_nOutReliableState, netchan->m_nInSequenceNr, g_GlobalVars->realtime));
			}

			if (sequences.size() > 1024) 
				sequences.pop_back();
			
		}
	}
}
FakeLatency fakelatency;