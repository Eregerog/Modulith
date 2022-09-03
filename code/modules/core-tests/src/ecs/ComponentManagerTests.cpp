/**
 * \brief
 * \author Daniel Götz
 */

#include "Core.h"
#include "catch.hpp"
#include "ECSTestUtils.h"
#include <ecs/EntityManager.h>

SCENARIO("Components can be copied from and to std::any"){
    auto componentManager = CreateComponentManager();

    GIVEN("A component with data and no RAII / Destructors"){

        auto info = componentManager->GetInfoOf(typeid(NumberData));

        REQUIRE(info.IsSerializable());

        auto* data = new NumberData(12);

        REQUIRE(data->Number == 12);

        WHEN("A new number data is copied into data"){
            info.CopyFromAnyToPointer(NumberData(13), data);

            THEN("The data's number changes"){
                REQUIRE(data->Number == 13);
            }

            AND_WHEN("The data is copied back into a std::any"){
                auto asAny = info.CopyFromPointerToAny(data);
                auto number = std::any_cast<NumberData>(asAny);

                THEN("The any contains the number that was set"){
                    REQUIRE(number.Number == 13);
                }
            }
        }

        delete data;
    }

    GIVEN("A component with a shared pointer"){

        auto info = componentManager->GetInfoOf(typeid(FirstSharedResourceData));

        REQUIRE(info.IsSerializable());

        auto firstSharedPtr = std::make_shared<int>(666);

        auto* data = new FirstSharedResourceData(firstSharedPtr);

        REQUIRE(firstSharedPtr.use_count() == 2);

        WHEN("New data is copied into the pointer"){

            auto secondSharedPtr = std::make_shared<int>(1337);

            REQUIRE(secondSharedPtr.use_count() == 1);

            info.CopyFromAnyToPointer(FirstSharedResourceData(secondSharedPtr), data);

            THEN("The use count of the old shared pointer and new pointer are changed accordingly"){
                REQUIRE(firstSharedPtr.use_count() == 1);
                REQUIRE(secondSharedPtr.use_count() == 2);
            }

            AND_WHEN("The data is copied into a std::any"){

                {
                    auto asAny = info.CopyFromPointerToAny(data);
                    auto resourceData = std::any_cast<FirstSharedResourceData>(asAny);
                    asAny.reset();

                    THEN("The use count and value inside the resource data are correct") {
                        REQUIRE(secondSharedPtr.use_count() == 3);
                        REQUIRE(*resourceData.Resource == 1337);
                        REQUIRE(resourceData.Resource == secondSharedPtr);
                    }
                }

                AND_WHEN("The copied values go out-of-scope"){
                    THEN("The resource count is back to what is was before"){
                        REQUIRE(secondSharedPtr.use_count() == 2);
                    }
                }
            }
        }

        delete data;
    }
}
