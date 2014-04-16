#if defined(VERTEX)

uniform mat4 Projection;
uniform mat4 View;
uniform mat4 Object;

in vec3 VertexPosition;
in vec3 VertexNormal;
in vec2 VertexTexCoord;

out vData
{
	vec2 uv;
    vec3 normal;
    vec3 position;
}vertex;

void main(void)
{

	vec4 new_position = vec4(VertexPosition, 1);

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
}frag;    

void main()
{
	for (int n = 0; n < gl_in.length(); n++)
	{
		frag.uv = vertices[n].uv;
		frag.normal = vertices[n].normal;
		vec3 pos = gl_in[n].gl_Position.xyz;
		frag.position = vertices[n].position;
		gl_Position = Projection * View * Object * vec4(pos, 1);
		EmitVertex();
	}
	EndPrimitive();
}

#endif

#if defined(FRAGMENT)

uniform vec3 CameraPosition;

in fData
{
	vec2 uv;
    vec3 normal;
    vec3 position;
}frag;

uniform sampler2D Diffuse;
uniform sampler2D Spec;

out vec4  Color;
out vec4  Normal;

void main(void)
{
	Color = vec4(texture(Diffuse, frag.uv).rgb, texture(Spec, frag.uv).x);
	Normal = vec4(frag.normal, 1);
}

#endif