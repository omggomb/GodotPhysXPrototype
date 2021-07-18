extends MultiMeshInstance

export var grid_dimensions = 10
export var padding = 2

onready var half_dim = (grid_dimensions + padding * grid_dimensions) / 2.0


func _ready():
	multimesh.instance_count = pow(grid_dimensions, 3)
	
	
func spawn_yielding():
	var count = 0
	for i in grid_dimensions:
		for j in grid_dimensions:
			for k in grid_dimensions:
				var t = Transform(Basis.IDENTITY, Vector3(i + (i*padding -half_dim ) , j + (j*padding), k + (k*padding-half_dim)))
				multimesh.set_instance_transform(count, t)
				count += 1
				multimesh.visible_instance_count = count
		yield()


func reset():
	multimesh.visible_instance_count = 0



