#include "Service/User/RegisterUserHandler.h"
#include "Service/MessageBus.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "mocks/PasswordHasher.mock.h"
#include "mocks/UserUpdatableStorage.mock.h"

using namespace testing;

class RegisterUserHandlerFixture : public Test
{
public:
    RegisterUserHandlerFixture() :
        mockUserStorage(std::make_shared<NiceMock<MockUserUpdatableStorage>>()),
        mockPasswordHasher(std::make_shared<NiceMock<MockPasswordHasher>>()),
        messageBus(std::make_shared<Service::MessageBus>()),

        handler(Service::User::RegisterUserHandler::make(messageBus, mockUserStorage, mockPasswordHasher))
    {}

protected:
    std::shared_ptr<NiceMock<MockUserUpdatableStorage>> mockUserStorage;
    std::shared_ptr<NiceMock<MockPasswordHasher>> mockPasswordHasher;
    std::shared_ptr<Service::MessageBus> messageBus;
    std::shared_ptr<Service::User::RegisterUserHandler> handler;

    const std::string VALID_EMAIL = "newuser@example.com";
    const std::string RAW_PASSWORD = "securepassword";
    const std::vector<char> HASHED_PASSWORD = std::vector<char>(64, 'h');
    const int NEW_USER_ID = 99;

    Service::User::RegisterUserCommand createValidCommand()
    {
        return Service::User::RegisterUserCommand{.email = VALID_EMAIL, .rawPassword = RAW_PASSWORD};
    }
};

TEST_F(RegisterUserHandlerFixture, SuccessfulRegistration)
{
    auto mockUow = std::make_unique<NiceMock<MockUoW>>();
    EXPECT_CALL(*mockUow, commit()).Times(1);
    EXPECT_CALL(*mockUow, rollback()).Times(0);
    EXPECT_CALL(*mockUserStorage, beginWork()).WillOnce(Return(ByMove(std::move(mockUow))));

    EXPECT_CALL(*mockUserStorage, add(_))
        .WillOnce(DoAll(
            Invoke([&](Domain::User& user) { user.setId(NEW_USER_ID); }),
            Return()
        ));

    EXPECT_CALL(*mockPasswordHasher, hash(RAW_PASSWORD)).WillOnce(Return(HASHED_PASSWORD));

    Service::User::RegisterUserCommand command = createValidCommand();
    Service::User::RegisterUserResult result = handler->execute(command);

    EXPECT_EQ(result.userId, NEW_USER_ID);
}

TEST_F(RegisterUserHandlerFixture, DuplicateEmailFailsAndRollsBack)
{
    auto mockUow = std::make_unique<NiceMock<MockUoW>>();
    EXPECT_CALL(*mockUow, rollback()).Times(1);
    EXPECT_CALL(*mockUow, commit()).Times(0);
    EXPECT_CALL(*mockUserStorage, beginWork()).WillOnce(Return(ByMove(std::move(mockUow))));

    EXPECT_CALL(*mockPasswordHasher, hash(RAW_PASSWORD)).WillOnce(Return(HASHED_PASSWORD));

    EXPECT_CALL(*mockUserStorage, add(_))
        .WillOnce(Throw(Util::BusinessException("User with this email already exists")));

    Service::User::RegisterUserCommand command = createValidCommand();
    EXPECT_THROW(handler->execute(command), Util::BusinessException);
}