#include "Util/LogicException.h"

using namespace Util;

POCO_IMPLEMENT_EXCEPTION(LogicException, Poco::RuntimeException, "LogicException")
