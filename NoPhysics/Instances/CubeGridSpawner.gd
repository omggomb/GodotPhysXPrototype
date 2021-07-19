extends Spatial

export var grid_dimensions = 1000
export var padding = 2

signal count_changed(new_count)

var cube_scn = preload("res://NoPhysics/Instances/CubeNoPhysics.tscn")

onready var half_dim = (grid_dimensions + padding * grid_dimensions) / 2.0

var count = 0
var i = 0
var j = 0
var k = 0

			
func spawn_single():
	var inst = cube_scn.instance()
	inst.translate(Vector3(i + (i*padding -half_dim ) , j + (j*padding), k + (k*padding-half_dim)))
	count += 1
	emit_signal("count_changed", count)
	add_child(inst)
	
	k += 1
	
	if k > grid_dimensions:
		i += 1
		k = 0
		if i > grid_dimensions:
			j += 1
			i = 0
	
func reset():
	for c in get_children():
		remove_child(c)
		c.queue_free()
		
	count = 0
	i = 0
	j = 0
	k = 0


