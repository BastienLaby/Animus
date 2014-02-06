#if defined(VERTEX)
uniform mat4 Projection;
uniform mat4 View;
uniform mat4 Object;

in vec3 VertexPosition;
in vec3 VertexNormal;
in vec2 VertexTexCoord;

uniform float Time;
uniform int RenderLightModel;
uniform int SpectrumOffset;

out vData
{
	vec2 uv;
    vec3 normal;
    vec3 position;
}vertex;

void main(void)
{

	vec4 new_position;
	// Rotate with a rotation matrix. The angle is based on the time.
	float theta = 0.4*Time;
	/*new_position.x = VertexPosition.x * cos(theta) - VertexPosition.z * sin(theta);
	new_position.y = VertexPosition.y;
	new_position.z = VertexPosition.x * sin(theta) + VertexPosition.z * cos(theta);
	new_position.w = 1;*/

	new_position = vec4(VertexPosition, 1);

	vertex.normal = vec3(Object * vec4((VertexNormal+1.0)*0.5, 1.0));
	vertex.uv = VertexTexCoord;
	vertex.position = vec3(Object * new_position);

	gl_Position = new_position;
}

#endif

#if defined(GEOMETRY)

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform mat4 Projection;
uniform mat4 View;
uniform mat4 Object;

uniform float PrimitiveRotationAngle;

in vData
{
	vec2 uv;
    vec3 normal;
    vec3 position;

}vertices[];

uniform float Time;

out fData
{
	vec2 uv;
    vec3 normal;
    vec3 position;
    vec3 color;
}frag;    

float rand(vec2 n)
{
  return 0.5 + 0.5 * fract(sin(dot(n.xy, vec2(12.9898, 78.233)))* 43758.5453);
}

mat3 rotationMatrix(vec3 axis, float angle)
{
	mat3 P = mat3(	axis.x*axis.x,	axis.x*axis.y,	axis.x*axis.z,
					axis.x*axis.y,	axis.y*axis.y,	axis.y*axis.y,
					axis.x*axis.z,	axis.y*axis.z,	axis.z*axis.z);
	mat3 I = mat3(1);
	mat3 Q = mat3(	0, 			-axis.z,	axis.y,
					axis.z,		0,			-axis.x,
					-axis.y,	axis.x,		0);
	return P + cos(angle)*(I - P) + sin(angle)*Q;
}

void main()
{

	int n;
	float random = rand(vec2(gl_PrimitiveIDIn, gl_PrimitiveIDIn));
	float random2 = rand(vec2(gl_PrimitiveIDIn+1, gl_PrimitiveIDIn));
	float random3 = rand(vec2(gl_PrimitiveIDIn, gl_PrimitiveIDIn+1));
	
	vec3 U = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
	vec3 V = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
	vec3 triangleNormal = normalize(cross(U, V));


	for (n = 0; n < gl_in.length(); n++)
	{
		frag.uv = vertices[n].uv;
		frag.normal = vertices[n].normal;
		
		vec3 pos = gl_in[n].gl_Position.xyz;

		frag.color = vec3(1, 0, 0);
		if(gl_PrimitiveIDIn%2 == 0)
		{
			pos *= rotationMatrix(triangleNormal, PrimitiveRotationAngle);
			frag.color.g = 1;
		}

		frag.position = vertices[n].position;
		gl_Position = Projection * View * Object * vec4(pos, 1);
		
		//frag.color = vec3(random, 1-random2, 1-random3);
		EmitVertex();
	}
	EndPrimitive();
}

#endif

#if defined(FRAGMENT)

uniform vec3 CameraPosition;
uniform float Time;
uniform int RenderLightModel;

in fData
{
	vec2 uv;
    vec3 normal;
    vec3 position;
    vec3 color;
}frag;

uniform sampler2D Diffuse;
uniform sampler2D Spec;

out vec4  Color;
out vec4  Normal;

void main(void)
{
	//Color = vec4(texture(Diffuse, frag.uv).rgb, texture(Spec, frag.uv).x);
	Color = vec4(frag.color, 1);
	Normal = vec4(frag.normal, 1);
}

#endif