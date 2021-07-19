extends Position3D

enum SpawnType {
	SERVER,
	SERVER_GDSCRIPT,
	INSTANCE,
	PHYS_X,
	CUBES,
	MULTIMESH
}

export(SpawnType) var spawn_type
export(NodePath) var phys_x_spawner_path
export(NodePath) var cpp_server_spawner_path
export var force = 1000

var meshes = []
var shapes = []
var scenes = []
var instances = []
var bodies = []
var count = 0

var cube_scene = preload("res://BulletOrBuiltin/Box.tscn")

onready var spawn_timer = $"../../SpawnTimer"
onready var pxSpawner = get_node(phys_x_spawner_path) as PhysXSpawner
onready var pxParent = pxSpawner.get_parent() as Node
onready var cpp_spawner = get_node(cpp_server_spawner_path) as ServerSpawner
onready var non_phys_instance_spawner = $"../../../NonPhysicsSpawners/CubeGridSpawner"
onready var non_phys_multimesh_spawner = $"../../../NonPhysicsSpawners/CubeSpawnerMultimesh"

export(NodePath) var object_count_label_path
onready var object_count_label = get_node(object_count_label_path) as Label


func _ready():
	meshes.append(CubeMesh.new())
	shapes.append(BoxShape.new())
	scenes.append(cube_scene)
	
	
func _on_SpawnTimer_timeout():
	match(spawn_type):
		SpawnType.INSTANCE:
			_spawn_cube_instance()
		SpawnType.SERVER:
			_spawn_cube_server_cpp()
		SpawnType.SERVER_GDSCRIPT:
			_spawn_cube_server()
		SpawnType.PHYS_X:
			_spawn_physx_cube()
		SpawnType.CUBES:
			_spawn_cubes_no_physics()
		SpawnType.MULTIMESH:
			_spawn_multimesh()
		
	if (Performance.get_monitor(Performance.TIME_FPS) < 60 and count > 100):
		spawn_timer.stop()
	
	
func _spawn_cube_instance():
	var ins = scenes[count % len(scenes)].instance()
	ins.sleeping = false
	$SpawnedInstances.add_child(ins)
	ins.global_transform = global_transform
	_increment_count()
	ins.add_central_force(Vector3.UP * force)
	
	
func _spawn_cube_server_cpp():
	cpp_spawner.spawn_cube()
	_increment_count()
	
	
func _spawn_cube_server():
	var inst_rid = VisualServer.instance_create()
	VisualServer.instance_set_scenario(inst_rid, get_world().scenario)
	VisualServer.instance_set_base(inst_rid, meshes[count % len(meshes)])
	VisualServer.instance_set_transform(inst_rid, global_transform)
	instances.append(inst_rid)
	
	var bdy_id = PhysicsServer.body_create(PhysicsServer.BODY_MODE_RIGID)
	PhysicsServer.body_add_shape(bdy_id, shapes[count % len(shapes)])
	PhysicsServer.body_set_space(bdy_id, get_world().space)
	PhysicsServer.body_set_state(bdy_id, PhysicsServer.BODY_STATE_TRANSFORM, global_transform)
	PhysicsServer.body_set_force_integration_callback(bdy_id, self, "on_body_moved", inst_rid)
	PhysicsServer.body_add_central_force(bdy_id, Vector3.UP * force)
	bodies.append(bdy_id)
	
	_increment_count()
	
	
func _spawn_physx_cube():
	pxSpawner.spawn_physx_cube()
	_increment_count()
	
	
func _spawn_cubes_no_physics():
	non_phys_instance_spawner.spawn_single()
	count += 1
	_increment_count()
	
	
func _spawn_multimesh():
	non_phys_multimesh_spawner.spawn_batch()
	count += 1
	_update_count_label(non_phys_multimesh_spawner.grid_dimensions * count)
	
	
func on_body_moved(state, inst_rid):
	VisualServer.instance_set_transform(inst_rid, state.transform)
	
	
func reset():
	spawn_timer.stop()
	
	for b in bodies:
		PhysicsServer.free_rid(b)
		
	for i in instances:
		VisualServer.free_rid(i)
		
	if pxParent.get_child_count() > 0:
		pxParent.remove_child(pxSpawner)
	
	for c in $SpawnedInstances.get_children():
		$SpawnedInstances.remove_child(c)
		
	cpp_spawner.reset()
		
	bodies = []
	instances = []
	count = 0
	
	non_phys_instance_spawner.reset()
	non_phys_multimesh_spawner.reset()
	
	yield(get_tree().create_timer(0.1), "timeout")
	
	
func _unhandled_input(event):
	if event is InputEventKey and event.is_pressed():
		if event.scancode == KEY_F:
			OS.window_fullscreen = not OS.window_fullscreen
	
	
func _increment_count():
	count += 1
	_update_count_label(count)
	
	
func _update_count_label(new_count):
	object_count_label.text = str(new_count)
	

func _exit_tree():
	reset()


func _on_InstanceBujtton_pressed():
	reset()
	spawn_type = SpawnType.INSTANCE
	spawn_timer.start()
	

func _on_ServerButton_pressed():
	reset()
	spawn_type = SpawnType.SERVER
	spawn_timer.start()
	
	
func _on_PhysXButton_pressed():
	reset()
	pxParent.add_child(pxSpawner)
	spawn_type = SpawnType.PHYS_X
	spawn_timer.start()
	
	
func _on_MultiMeshInstanceButton_pressed():
	reset()
	spawn_type = SpawnType.MULTIMESH
	spawn_timer.start()


func _on_CubesButton_pressed():
	reset()
	spawn_type = SpawnType.CUBES
	spawn_timer.start()

	
func _on_GDScriptServerButton_pressed():
	reset()
	spawn_type = SpawnType.SERVER_GDSCRIPT
	spawn_timer.start()
	
