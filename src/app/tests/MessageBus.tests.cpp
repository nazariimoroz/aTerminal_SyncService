#include <gtest/gtest.h>
#include <memory>
#include <string>

#include "Service/MessageBus.h"

using Service::MessageBus;

// Commands

struct PingCmd
{
    int value;
    using Result = int;
    using Error = int;
};

struct AddCmd
{
    int x;
    using Result = int;
    using Error = int;
};

struct MulCmd
{
    int x;
    using Result = int;
    using Error = int;
};

// Services

class AccumulatorService
{
public:
    explicit AccumulatorService(int base)
        : _base(base)
    {}

    std::expected<int, int> Add(const AddCmd& cmd)
    {
        _lastArg = cmd.x;
        return _base + cmd.x;
    }

    int lastArg() const { return _lastArg; }

private:
    int _base = 0;
    int _lastArg = 0;
};

class MultiplierService
{
public:
    explicit MultiplierService(int factor)
        : _factor(factor)
    {}

    std::expected<int, int> Mul(const MulCmd& cmd) const
    {
        return _factor * cmd.x;
    }

private:
    int _factor = 1;
};

// Tests

TEST(MessageBusTests, LambdaHandler_Call_ReturnsExpectedResult)
{
    MessageBus bus;

    bus.registerHandler<PingCmd>(
        [](const PingCmd& c) -> std::expected<PingCmd::Result, PingCmd::Error> {
            return c.value + 1;
        }
    );

    PingCmd cmd{41};
    auto result = bus.call(cmd);

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(*result, 42);
}

TEST(MessageBusTests, ObjectHandler_NonConstMethod_WorksAndMutatesServiceState)
{
    MessageBus bus;

    auto service = AccumulatorService(10);

    bus.registerHandler<AddCmd>(
        service,
        &AccumulatorService::Add
    );

    AddCmd cmd{32};
    auto result = bus.call(cmd);

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(*result, 42);
    EXPECT_EQ(service.lastArg(), 32);
}

TEST(MessageBusTests, ObjectHandler_ConstMethod_Works)
{
    MessageBus bus;

    auto service = MultiplierService(7);

    bus.registerHandler<MulCmd>(
        service,
        &MultiplierService::Mul
    );

    MulCmd cmd{9};
    auto result = bus.call(cmd);

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result, 63);
}

TEST(MessageBusTests, Call_ThrowsIfNoHandlerRegistered)
{
    MessageBus bus;

    PingCmd cmd{5};

    EXPECT_THROW(
        {
            (void)bus.call(cmd);
        },
        Util::LogicException
    );
}

TEST(MessageBusTests, ReRegister_OverridesPreviousHandler)
{
    MessageBus bus;

    bus.registerHandler<PingCmd>(
        [](const PingCmd& c) -> std::expected<PingCmd::Result, PingCmd::Error> {
            return c.value + 100;
        }
    );

    bus.registerHandler<PingCmd>(
        [](const PingCmd& c) -> std::expected<PingCmd::Result, PingCmd::Error> {
            return c.value + 1;
        }
    );

    PingCmd cmd{41};
    auto result = bus.call(cmd);

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result, 42);
}
