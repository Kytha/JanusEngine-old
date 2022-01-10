#include "jnpch.h"

#include "Core/Layer.h"

namespace Janus
{
	Layer::Layer(const std::string &debugName) : m_DebugName(debugName) {}

	Layer::~Layer()
	{
	}
}
