#pragma once
#include <gmock/gmock.h>

#include "Port/UnitOfWork.h"
#include "Port/User/UserUpdatableStorage.h"

class MockUoW : public Port::IUnitOfWork
{
public:
    MOCK_METHOD(void, commit, (), (override));
    MOCK_METHOD(void, rollback, (), (override));
};

class MockUserUpdatableStorage : public Port::User::IUserUpdatableStorage
{
public:
    MOCK_METHOD(std::optional<Domain::User>, findById, (const int&), (override));
    MOCK_METHOD(bool, existsByEmail, (const std::string&), (override));
    MOCK_METHOD(void, add, (Domain::User&), (override));
    MOCK_METHOD(void, update, (const Domain::User&), (override));
    MOCK_METHOD(std::unique_ptr<Port::IUnitOfWork>, beginWork, (), (override));
    MOCK_METHOD(std::optional<Domain::User>, findByEmail, (const std::string& email), (override));
};
