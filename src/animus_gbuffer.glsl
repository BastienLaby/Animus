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

	vec4 new_position = vec4(VertexPosition, 1.0);
	new_position.x += 2*gl_InstanceID;

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

out fData
{
	vec2 uv;
    vec3 normal;
    vec3 position;
}frag;    

void main()
{

	int n;
	
	for (n = 0; n < gl_in.length(); n++)
	{
		gl_Position = gl_in[n].gl_Position;
		frag.normal = vertices[n].normal;
		frag.position = vertices[n].position;
		gl_Position.x += 3*cos(gl_PrimitiveIDIn) + gl_Position.x;
		gl_Position.y += 3*sin(gl_PrimitiveIDIn) + gl_Position.y;
		//gl_Position.z += gl_Position.z;
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
}frag;

uniform sampler2D Diffuse;
uniform sampler2D Spec;

out vec4  Color;
out vec4  Normal;

void main(void)
{
	//Color = vec4(texture(Diffuse, frag.uv).rgb, texture(Spec, frag.uv).x);
	Color = vec4(frag.position, 1);
	Normal = vec4(frag.normal, 1);

}

#endif