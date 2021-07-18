#include <Godot.hpp>
#include <Reference.hpp>
#include <Spatial.hpp>
#include <VisualServer.hpp>
#include <PhysicsServer.hpp>
#include <CubeMesh.hpp>
#include <World.hpp>
#include <vector>
#include <unordered_map>
#include <BoxShape.hpp>
#include <PhysicsDirectBodyState.hpp>
#include <Engine.hpp>

#include "PxPhysicsAPI.h"

#define PX_RELEASE(x)	if(x)	{ x->release(); x = nullptr;	}
#define PVD_HOST "127.0.0.1"


class PhysXSpawner : public godot::Spatial
{
    GODOT_CLASS(PhysXSpawner, godot::Spatial);

public:
    godot::CubeMesh *mesh;
    godot::BoxShape *shape;

    std::vector<godot::RID> instances{2000};
    std::unordered_map<physx::PxRigidDynamic*, godot::RID> pxBodies;

    physx::PxDefaultAllocator pxAllocator;
    physx::PxDefaultErrorCallback pxErrCallback;

    physx::PxFoundation *gFoundation = nullptr;
    physx::PxPhysics *gPhysics = nullptr;

    physx::PxDefaultCpuDispatcher *gDispatcher = nullptr;
    physx::PxScene *gScene = nullptr;

    physx::PxMaterial *gMaterial = nullptr;
    physx::PxShape* box_shape = nullptr;

    physx::PxPvd* gPvd = nullptr;

    float density = 1.0f;
    float halfExtends = 1.0f;
    godot::Vector3 impulse;

    float static_friction = 1.0f;
    float dynamic_friction = 1.0f;
    float restitution = 1.0f;



    PhysXSpawner()
    {
        mesh = godot::CubeMesh::_new();
        shape = godot::BoxShape::_new();
    }

    ~PhysXSpawner()
    {
    }

    /* _init must exist as it is called by Godot */
    void _init()
    {
      

    }

    void _enter_tree() {
        godot::Godot::print("PhysX Node entering tree!");

        if (!godot::Engine::get_singleton()->is_editor_hint()) {
            godot::Godot::print("Not in editor!");

            gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, pxAllocator, pxErrCallback);

            gPvd = physx::PxCreatePvd(*gFoundation);
	        physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	        if (gPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL))
                godot::Godot::print("Connected to PVD!");
            else
                godot::Godot::print("Failed to connect to PVD!");


            gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, physx::PxTolerancesScale());

            physx::PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
            sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
            gDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
            sceneDesc.cpuDispatcher	= gDispatcher;
            sceneDesc.filterShader	= physx::PxDefaultSimulationFilterShader;
            gScene = gPhysics->createScene(sceneDesc);

            physx::PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
            if(pvdClient)
            {
                pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
                pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
                pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
            }
            else
            {
                godot::Godot::print("No PVD client!");
            }
            
            
            gMaterial = gPhysics->createMaterial(static_friction, dynamic_friction, restitution);

            physx::PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, physx::PxPlane(0,1,0,0), *gMaterial);
	        gScene->addActor(*groundPlane);

            box_shape = gPhysics->createShape(physx::PxBoxGeometry(halfExtends, halfExtends, halfExtends), *gMaterial);

            float distToCenter = 10;
            auto p = PxCreatePlane(*gPhysics, physx::PxPlane(physx::PxVec3(distToCenter, 0, 0), physx::PxVec3(-1, 0, 0)), *gMaterial);
            gScene->addActor(*p);

            p = PxCreatePlane(*gPhysics, physx::PxPlane(physx::PxVec3(-distToCenter, 0, 0), physx::PxVec3(1, 0, 0)), *gMaterial);
            gScene->addActor(*p);

            p = PxCreatePlane(*gPhysics, physx::PxPlane(physx::PxVec3(0, 0, distToCenter), physx::PxVec3(0, 0, -1)), *gMaterial);
            gScene->addActor(*p);

            p = PxCreatePlane(*gPhysics, physx::PxPlane(physx::PxVec3(0, 0, -distToCenter), physx::PxVec3(0, 0, 1)), *gMaterial);
            gScene->addActor(*p);
        }	
    }

     void _process(float dt) {
        
        physx::PxU32 nbActors = gScene->getNbActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC);
        if(nbActors)
        {
            std::vector<physx::PxRigidActor*> actors(nbActors);
            gScene->getActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC, reinterpret_cast<physx::PxActor**>(&actors[0]), nbActors);

            for (auto b : actors) {
                auto pose = b->getGlobalPose();
                float angle = 0.0f;
                physx::PxVec3 axis;

                pose.q.toRadiansAndUnitAxis(angle, axis);

                godot::Vector3 gd_axis{axis.x, axis.y, axis.z};
                godot::Quat q{gd_axis,  angle};
                godot::Basis gd_b{q};
                godot::Vector3 gd_pos{pose.p.x, pose.p.y, pose.p.z};

                godot::Transform t{gd_b, gd_pos};

                godot::VisualServer::get_singleton()->instance_set_transform(pxBodies[(physx::PxRigidDynamic*)b], t);
                // get pos, get quat, construct transform use pxBodies to map tot vis rid ... profit
            }
        }

    }

    void _physics_process(float dt) {
        gScene->simulate(dt);
        gScene->fetchResults(true);
    }

    void spawn_physx_cube()
    {
        godot::RID inst_id = godot::VisualServer::get_singleton()->instance_create2(mesh->get_rid(), get_world()->get_scenario());
        instances.push_back(inst_id);

        auto pos = get_global_transform().origin;
        physx::PxTransform t{physx::PxVec3(pos.x, pos.y, pos.z)};
        auto body = gPhysics->createRigidDynamic(t);
        body->attachShape(*box_shape);
        physx::PxRigidBodyExt::updateMassAndInertia(*body, density);
        gScene->addActor(*body);
        body->addForce(physx::PxVec3(impulse.x, impulse.y, impulse.z), physx::PxForceMode::eIMPULSE);

        pxBodies.insert({body, inst_id});
    }

    godot::Variant get_number_of_physx_rigid_dynamics() {
        physx::PxU32 nbActors = gScene->getNbActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC);

        return godot::Variant(nbActors);
    }

    void _exit_tree()
    {
        godot::Godot::print("Exiting");

        for (auto i : instances)
        {
            godot::VisualServer::get_singleton()->free_rid(i);
        }

        instances.clear();

        if (!godot::Engine::get_singleton()->is_editor_hint()) {
            PX_RELEASE(box_shape);
            PX_RELEASE(gScene);
            PX_RELEASE(gDispatcher);
            PX_RELEASE(gPhysics);
            if(gPvd)
            {
                physx::PxPvdTransport* transport = gPvd->getTransport();
                PX_RELEASE(gPvd);
                PX_RELEASE(transport);
            }
            PX_RELEASE(gFoundation);
        }
    }

    static void _register_methods()
    {
        godot::register_method("spawn_physx_cube", &PhysXSpawner::spawn_physx_cube);
        godot::register_method("_process", &PhysXSpawner::_process);
        godot::register_method("_physics_process", &PhysXSpawner::_physics_process);
        godot::register_method("_exit_tree", &PhysXSpawner::_exit_tree);
        godot::register_method("_enter_tree", &PhysXSpawner::_enter_tree);
        godot::register_method("get_number_of_physx_rigid_dynamics", &PhysXSpawner::get_number_of_physx_rigid_dynamics);

      
        godot::register_property<PhysXSpawner, float>("cube/density", &PhysXSpawner::density, 1.0f);
        godot::register_property<PhysXSpawner, float>("cube/halfextends", &PhysXSpawner::halfExtends, 1.0f);
        godot::register_property<PhysXSpawner, godot::Vector3>("cube/impulse", &PhysXSpawner::impulse, godot::Vector3(0, 0, 0));

        godot::register_property<PhysXSpawner, float>("material/static_friction", &PhysXSpawner::static_friction, 1.0f);
        godot::register_property<PhysXSpawner, float>("material/dynamic_friction", &PhysXSpawner::dynamic_friction, 1.0f);
        godot::register_property<PhysXSpawner, float>("material/restitution", &PhysXSpawner::restitution, 1.0f);
    }
};

class ServerSpawner : public godot::Spatial
{
    GODOT_CLASS(ServerSpawner, godot::Spatial);

    godot::BoxShape* shape;
    godot::CubeMesh* mesh;

    float force;

    std::vector<godot::RID> visual_instances;
    std::vector<godot::RID> physics_instances;
public:
    /* _init must exist as it is called by Godot */
    void _init()
    {
        visual_instances.reserve(4000);
        physics_instances.reserve(4000);
    }

    void _ready()
    {
        shape = godot::BoxShape::_new();
        mesh = godot::CubeMesh::_new();
    }
    void spawn_cube()
    {
        auto visual_id = godot::VisualServer::get_singleton()->instance_create();
        godot::VisualServer::get_singleton()->instance_set_scenario(visual_id, get_world()->get_scenario());
        godot::VisualServer::get_singleton()->instance_set_base(visual_id, mesh->get_rid());
        godot::VisualServer::get_singleton()->instance_set_transform(visual_id, get_global_transform());
        visual_instances.push_back(visual_id);

        auto body_id = godot::PhysicsServer::get_singleton()->body_create(godot::PhysicsServer::BODY_MODE_RIGID);
        godot::PhysicsServer::get_singleton()->body_add_shape(body_id, shape->get_rid());
        godot::PhysicsServer::get_singleton()->body_set_space(body_id, get_world()->get_space());
        godot::PhysicsServer::get_singleton()->body_set_state(body_id, godot::PhysicsServer::BODY_STATE_TRANSFORM, get_global_transform());
        godot::PhysicsServer::get_singleton()->body_set_force_integration_callback(body_id, this, "_on_body_moved", visual_id);
        godot::PhysicsServer::get_singleton()->body_apply_central_impulse(body_id, godot::Vector3(0,1,0) * force);
        physics_instances.push_back(body_id);

    }

    void _on_body_moved(godot::PhysicsDirectBodyState* state, godot::Variant instance_id)
    {
        godot::VisualServer::get_singleton()->instance_set_transform(instance_id, state->get_transform());
    }

    void reset()
    {
        for (const auto& id : visual_instances)
        {
            godot::VisualServer::get_singleton()->free_rid(id);
        }
        visual_instances.clear();

        for (const auto& id : physics_instances)
        {
            godot::PhysicsServer::get_singleton()->free_rid(id);
        }
        physics_instances.clear();
    }

    

    static void _register_methods()
    {
        godot::register_method("_init", &ServerSpawner::_init);
        godot::register_method("_ready", &ServerSpawner::_ready);
        godot::register_method("spawn_cube", &ServerSpawner::spawn_cube);
        godot::register_method("_on_body_moved", &ServerSpawner::_on_body_moved);
        godot::register_method("reset", &ServerSpawner::reset);

        godot::register_property<ServerSpawner, float>("force", &ServerSpawner::force, 1.0f);
    }
};

/** GDNative Initialize **/
extern "C" void GDN_EXPORT godot_gdnative_init(godot_gdnative_init_options *o)
{
    godot::Godot::gdnative_init(o);
}

/** GDNative Terminate **/
extern "C" void GDN_EXPORT godot_gdnative_terminate(godot_gdnative_terminate_options *o)
{
    godot::Godot::gdnative_terminate(o);
}

/** NativeScript Initialize **/
extern "C" void GDN_EXPORT godot_nativescript_init(void *handle)
{
    godot::Godot::nativescript_init(handle);

    godot::register_class<PhysXSpawner>();
    godot::register_class<ServerSpawner>();
}