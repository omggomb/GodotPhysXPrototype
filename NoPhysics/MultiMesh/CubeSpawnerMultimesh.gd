extends MultiMeshInstance

export var grid_dimensions = 10
export var padding = 2

onready var half_dim = (grid_dimensions + padding * grid_dimensions) / 2.0


func _ready():
	multimesh.instance_count = pow(grid_dimensions, 3)
	
var i = 0
var j = 0 
var count = 0
		
func spawn_batch():
	for k in grid_dimensions:
		var t = Transform(Basis.IDENTITY, Vector3(i + (i*padding -half_dim ) , j + (j*padding), k + (k*padding-half_dim)))
		multimesh.set_instance_transform(count, t)
		count += 1
		multimesh.visible_instance_count = count
		
	i += 1
	if i > grid_dimensions:
		j += 1
		i = 0
	

func reset():
	count = 0
	i = 0
	j = 0
	multimesh.visible_instance_count = 0



