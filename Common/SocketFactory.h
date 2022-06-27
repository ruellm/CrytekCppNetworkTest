#pragma once

#include "ISocketBase.h"
#include <memory>

namespace SocketFactory
{
	std::shared_ptr<ISocketBase> Create();
	void Destroy();
}