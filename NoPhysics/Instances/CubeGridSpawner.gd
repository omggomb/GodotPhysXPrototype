extends Spatial

export var grid_dimensions = 1000
export var padding = 2

signal count_changed(new_count)

var count = 0
var cube_scn = preload("res://NoPhysics/Instances/CubeNoPhysics.tscn")

onready var half_dim = (grid_dimensions + padding * grid_dimensions) / 2.0

func spawn_yielding():
	for i in grid_dimensions:
		for j in grid_dimensions:
			for k in grid_dimensions:
				var inst = cube_scn.instance()
				inst.translate(Vector3(i + (i*padding -half_dim ) , j + (j*padding), k + (k*padding-half_dim)))
				count += 1
				emit_signal("count_changed", count)
				add_child(inst)
			yield()
	
	
func reset():
	for c in get_children():
		remove_child(c)
		c.queue_free()
		
	count = 0


