#include "pch.h"
#include "FSMState.h"

#include "FSM.h"
#include "FSMTransition.h"

void FSMState::AddTransition(FSMTransition* transition)
{
	m_transitions.emplace_back(transition);
}

bool FSMState::CheckTransition()
{
	for(const auto& transition : m_transitions)
	{
		std::string stateName = (*transition)();
		if (!stateName.empty())
		{
			// state ��Ͽ��� find
			m_pParentFSM->SetNextState(stateName);
			return true;
		}
	}

	return false;
}
