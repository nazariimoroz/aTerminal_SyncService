#include "Util/BusinessException.h"

using namespace Util;

POCO_IMPLEMENT_EXCEPTION(BusinessException, Poco::RuntimeException, "BusinessException")
