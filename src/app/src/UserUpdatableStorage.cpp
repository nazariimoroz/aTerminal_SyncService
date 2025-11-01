#include "Port/User/UserUpdatableStorage.h"

using namespace Port::User;

POCO_IMPLEMENT_EXCEPTION(EmailAlreadyRegisteredException, Util::BusinessException, "EmailAlreadyRegisteredException")

IUserUpdatableStorage::~IUserUpdatableStorage() = default;

