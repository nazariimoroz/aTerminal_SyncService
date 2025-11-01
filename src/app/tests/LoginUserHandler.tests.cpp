#include "Service/User/RegisterUserHandler.h"
#include "Infra/InMemoryUserStorage.h"
#include "Service/MessageBus.h"
#include "Service/User/LoginUserHandler.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "mocks/PasswordHasher.mock.h"
#include "mocks/UserUpdatableStorage.mock.h"

using namespace testing;

class LoginUserHandlerFixture : public Test
{
public:
    LoginUserHandlerFixture() :
        mockUserStorage(std::make_shared<NiceMock<MockUserUpdatableStorage>>()),
        mockPasswordHasher(std::make_shared<NiceMock<MockPasswordHasher>>()),
        messageBus(std::make_shared<Service::MessageBus>()),

        handler(Service::User::LoginUserHandler::make(messageBus, mockUserStorage, mockPasswordHasher))
    {}

protected:
    std::shared_ptr<NiceMock<MockUserUpdatableStorage>> mockUserStorage;
    std::shared_ptr<NiceMock<MockPasswordHasher>> mockPasswordHasher;
    std::shared_ptr<Service::MessageBus> messageBus;
    std::shared_ptr<Service::User::LoginUserHandler> handler;

    const std::string VALID_EMAIL = "test@example.com";
    const std::string INVALID_EMAIL = "wrong@example.com";
    const std::string RAW_PASSWORD = "password123";
    const std::vector<char> HASHED_PASSWORD = std::vector(128, '-');
    const int USER_ID = 42;

    Domain::User createMockUser()
    {
        Domain::User user;
        user.setId(USER_ID);
        user.setEmail(VALID_EMAIL);
        user.setPassword(HASHED_PASSWORD);
        return user;
    }

    Service::User::LoginUserCommand createValidCommand()
    {
        return Service::User::LoginUserCommand{.email = VALID_EMAIL, .rawPassword = RAW_PASSWORD};
    }
};

TEST_F(LoginUserHandlerFixture, SuccessfulLogin)
{
    Domain::User mockUser = createMockUser();
    EXPECT_CALL(*mockUserStorage, findByEmail(VALID_EMAIL)).WillOnce(Return(std::make_optional(mockUser)));

    EXPECT_CALL(*mockPasswordHasher, verify(RAW_PASSWORD, HASHED_PASSWORD)).WillOnce(Return(true));

    Service::User::LoginUserCommand command = createValidCommand();
    Service::User::LoginUserResult result = handler->execute(command);

    EXPECT_EQ(result.userId, USER_ID);
}

TEST_F(LoginUserHandlerFixture, InvalidEmail)
{
    EXPECT_CALL(*mockUserStorage, findByEmail(INVALID_EMAIL)).WillOnce(Return(std::nullopt));

    EXPECT_CALL(*mockPasswordHasher, verify(_, _)).Times(0);

    Service::User::LoginUserCommand command = createValidCommand();
    command.email = INVALID_EMAIL;

    EXPECT_THROW(handler->execute(command), Service::User::InvalidEmailOrPasswordException);
}

TEST_F(LoginUserHandlerFixture, InvalidPassword)
{
    Domain::User mockUser = createMockUser();
    EXPECT_CALL(*mockUserStorage, findByEmail(VALID_EMAIL)).WillOnce(Return(std::make_optional(mockUser)));

    EXPECT_CALL(*mockPasswordHasher, verify(RAW_PASSWORD, HASHED_PASSWORD)).WillOnce(Return(false));

    Service::User::LoginUserCommand command = createValidCommand();

    EXPECT_THROW(handler->execute(command), Service::User::InvalidEmailOrPasswordException);
}
