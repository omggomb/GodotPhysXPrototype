shader_type spatial;

// Taken from: https://thebookofshaders.com/10/
float random(float x)
{
	return fract(sin(x)*100000.0);
}

void vertex()
{
	float angle = TIME  + random(float(INSTANCE_ID));
	mat4 rot = mat4(vec4(cos(angle), 0, sin(angle),  0),
					vec4(0, 1 , 0, 0),
					vec4(-sin(angle), 0, cos(angle), 0),
					vec4(0, 0, 0, 1));
	
	VERTEX = (rot * vec4(VERTEX, 1)).xyz;
}