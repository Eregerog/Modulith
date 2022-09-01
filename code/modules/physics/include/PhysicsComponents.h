/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include "PhysicsModule.h"

namespace modulith {
    namespace physics {

        /**
         * Attaching this component to an entity with a GlobalTransformData enables kinematics on it.
         * It will be affected by gravity and forces can be applied to it.
         */
        struct PHYSICS_API RigidbodyData {

            RigidbodyData() = default;

            explicit RigidbodyData(const modulith::float3& appliedForce) : AppliedForce(appliedForce) {}

            RigidbodyData(float mass, const modulith::float3& appliedForce) : Mass(mass), AppliedForce(appliedForce) {}

            float Mass = 1.0f;
            /**
             * The force to be applied the next time physics is updated.
             * After that, this value is reset
             */
            modulith::float3 AppliedForce{};
        };

        /**
         * When attached to an entity with a RigidbodyData, it will not be affected by gravity
         */
        struct PHYSICS_API DisableGravityTag {

        };

        /**
         * When attached to an entity with a RigidbodyData, its collision checks will be continuous.
         * This means that collisions will not be missed when the entity is moving at fast speeds.
         */
        struct PHYSICS_API EnableContinuousCollisionDetectionTag{

        };

        /**
         * When attached to an entity with GlobalTransformData other colliders can collide with it.
         * When the entity also has a RigidbodyData, the rigidbody will use this collider for collisions.
         */
        struct PHYSICS_API BoxColliderData {

            BoxColliderData() = default;

            explicit BoxColliderData(const modulith::float3& size) : Size(size) {}

            BoxColliderData(const modulith::float3& size, const modulith::float3& offset) : Size(size), Offset(offset) {}

            /**
             * The size of the collider.
             * The overall size is already affected by the entitie's scale,
             * this is another factor on top of that.
             */
            modulith::float3 Size = modulith::float3(1.0f);
            modulith::float3 Offset = modulith::float3(0.0f);

        };

        /**
         * When attached to an entity with a BoxColliderData the entities it collides with will be stored in this component
         */
        struct PHYSICS_API PhysicsContactsData{
            /**
             * The amount of entities the entity started contact with this frame
             */
            size_t Count = 0;
            /**
             * The entities this entity started contact with this frame
             */
            std::vector<modulith::Entity> BeginContact{};
        };

        /**
         * When attached to an entity with GlobalTransformData it will
         * emulate a physics-based character controller.
         * It has kinematics using a "collide and slide" algorithm
         * and can collide with objects around them.
         *
         * This component assumes that the pivot of this object is at the
         * character controller's feet (e.g. the collider is placed above it)
         */
        struct PHYSICS_API CharacterControllerData{
            /**
             * @param radius The radius of the character's collider
             * @param height The height of the character's collider
             * @remark The overall height is 2 * radius + height, since the collider is a capsule.
             */
            CharacterControllerData(float radius, float height) : Radius(radius), Height(height) {}

            float Radius;
            float Height;
            /**
             * The current velocity of the character
             */
            modulith::float3 Velocity{};
            /**
             * The distance the character should move the next physics update.
             * The value will be reset after that.
             */
            modulith::float3 CurrentDisplacement{};
            /**
             * Whether the character has started touching the ground this frame
             */
            bool TouchedGround = false;
        };
    }

    /**
     * Allows serialization of the physics::RididbodyData
     */
    template<>
    struct Serializer<physics::RigidbodyData>{
        [[nodiscard]] static physics::RigidbodyData Construct() { return physics::RigidbodyData(); }

        [[nodiscard]] static SerializedObject Serialize(const physics::RigidbodyData& rigidbodyData) {
            return { "Root", {
                { "Mass", rigidbodyData.Mass },
                { "Applied Force", rigidbodyData.AppliedForce }
            }};
        }

        [[nodiscard]] static std::optional<physics::RigidbodyData> TryDeserialize(const SerializedObject& so) {
            auto mass = so.TryGet<float>("Mass");
            auto appliedForce = so.TryGet<float3>("Applied Force");

            if(mass && appliedForce)
                return physics::RigidbodyData(std::max(0.0f, mass.value()), appliedForce.value());

            return std::nullopt;
        }
    };


    /**
     * Allows serialization of the physics::BoxColliderData
     */
    template<>
    struct Serializer<physics::BoxColliderData>{
        [[nodiscard]] static physics::BoxColliderData Construct() { return physics::BoxColliderData(); }

        [[nodiscard]] static SerializedObject Serialize(const physics::BoxColliderData& boxColliderData) {
            return { "Root", {
                { "Size", boxColliderData.Size },
                { "Offset", boxColliderData.Offset }
            }};
        }

        [[nodiscard]] static std::optional<physics::BoxColliderData> TryDeserialize(const SerializedObject& so) {
            auto size = so.TryGet<float3>("Size");
            auto offset = so.TryGet<float3>("Offset");

            if(size && offset)
                return physics::BoxColliderData(size.value(), offset.value());

            return std::nullopt;
        }
    };


    /**
     * Allows serialization of the physics::CharacterControllerData
     */
    template<>
    struct Serializer<physics::CharacterControllerData>{
        [[nodiscard]] static physics::CharacterControllerData Construct() { return physics::CharacterControllerData(0.5f, 2.0f); }

        [[nodiscard]] static SerializedObject Serialize(const physics::CharacterControllerData& characterControllerData) {
            return { "Root", {
                { "Radius", characterControllerData.Radius },
                { "Height", characterControllerData.Height },
                { "State", {
                    {"Velocity", characterControllerData.Velocity},
                    {"Current Displacement", characterControllerData.CurrentDisplacement},
                    {"Touched Ground", characterControllerData.TouchedGround}
                }}
            }};
        }

        [[nodiscard]] static std::optional<physics::CharacterControllerData> TryDeserialize(const SerializedObject& so) {
            auto radius = so.TryGet<float>("Radius");
            auto height = so.TryGet<float>("Height");
            if(auto state = so.TryGet("State")){
                auto velocity = state->TryGet<float3>("Velocity");
                auto displacement = state->TryGet<float3>("Current Displacement");
                auto touchedGround = state->TryGet<bool>("Touched Ground");

                if(radius && height && velocity && displacement && touchedGround){
                    auto res = physics::CharacterControllerData(radius.value(), height.value());
                    res.Velocity = velocity.value();
                    res.CurrentDisplacement = displacement.value();
                    res.TouchedGround = touchedGround.value();
                    return res;
                }
            }


            return std::nullopt;
        }
    };
}