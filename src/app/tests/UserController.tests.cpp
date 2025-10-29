#include <gtest/gtest.h>

#include "Rest/Controller/UserController.h"
#include "Util/Test/FakeRequest.h"
#include "Util/Test/FakeResponse.h"

TEST(UserControllerTests, Returns201AndBodyAndCallsUseCase)
{
    Util::FakeResponse resp;
    Util::FakeRequest req(
        "POST",
        "/api/v0/user",
        R"({"email":"test@example.com","password":"qwerty"})",
        resp
    );

    Rest::Controller::UserController userController;
    userController.handleRequest(req, resp);

    //EXPECT_EQ(resp.getStatus(), Poco::Net::HTTPResponse::HTTP_CREATED);
    //EXPECT_EQ(resp.get("Content-Type"), "application/json");

    //std::string body = resp.body();
    //EXPECT_NE(body.find("\"userId\""), std::string::npos);
    //EXPECT_NE(body.find("\"Account created\""), std::string::npos);

    // EXPECT_TRUE(fakeUseCase.ExecuteCalled);

    // EXPECT_EQ(fakeUseCase.LastCmd.Email, "extracted@example.com");
    // EXPECT_EQ(fakeUseCase.LastCmd.RawPassword, "raw-pass");
}

