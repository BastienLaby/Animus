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

uniform float Time;

out fData
{
	vec2 uv;
    vec3 normal;
    vec3 position;
    vec3 color;
}frag;    

void main()
{

	int n;
	for (n = 0; n < gl_in.length(); n++)
	{
		gl_Position = gl_in[n].gl_Position;
		frag.normal = vertices[n].normal;
		frag.position = vertices[n].position;
		if(gl_PrimitiveIDIn%1 == 0)
		{
			gl_Position.x += cos((0.1*(gl_PrimitiveIDIn%4) * Time) + gl_PrimitiveIDIn);
			gl_Position.y += sin((0.1*(gl_PrimitiveIDIn%4) * Time) + gl_PrimitiveIDIn);
		}
		frag.color = vec3(0, 0, 0);
		if(gl_PrimitiveIDIn%2==0)
		{
			frag.color.r=1;	
		}
		if(gl_PrimitiveIDIn%3==0)
		{
			frag.color.g=1;	
		}
		if(gl_PrimitiveIDIn%7==0)
		{
			frag.color.b=1;	
		}
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
	Color = vec4(texture(Diffuse, frag.uv).rgb, texture(Spec, frag.uv).x);
	//Color = vec4(frag.color, 1);
	Normal = vec4(frag.normal, 1);

}

#endif