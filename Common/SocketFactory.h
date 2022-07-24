#pragma once

#include "SocketBase.h"
#include <memory>

namespace SocketFactory
{
	std::shared_ptr<SocketBase> Create();
	void Destroy();
}