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
	new_position.x = VertexPosition.x * cos(theta) - VertexPosition.z * sin(theta);
	new_position.y = VertexPosition.y;
	new_position.z = VertexPosition.x * sin(theta) + VertexPosition.z * cos(theta);
	new_position.w = 1;

	vertex.normal = vec3(Object * vec4((VertexNormal+1.0)*0.5, 1.0));
	vertex.uv = VertexTexCoord;
	vertex.position = vec3(Object * new_position);

	gl_Position = Projection * View * Object * new_position;
}

#endif

#if defined(GEOMETRY)

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

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

void main()
{

	int n;
	float random = rand(vec2(gl_PrimitiveIDIn, gl_PrimitiveIDIn));
	float random2 = rand(vec2(gl_PrimitiveIDIn+1, gl_PrimitiveIDIn));
	float random3 = rand(vec2(gl_PrimitiveIDIn, gl_PrimitiveIDIn+1));
	for (n = 0; n < gl_in.length(); n++)
	{
		gl_Position = gl_in[n].gl_Position;
		frag.normal = vertices[n].normal;
		frag.position = vertices[n].position;
		frag.uv = vertices[n].uv;
		gl_Position.xy = gl_Position.xy * (random);
		frag.color = vec3(random, random2, random3);
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