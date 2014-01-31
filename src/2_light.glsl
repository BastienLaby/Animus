#if defined(VERTEX)

in vec2 VertexPosition;

out vec2 uv;

void main(void)
{	
	uv = VertexPosition * 0.5 + 0.5;
	gl_Position = vec4(VertexPosition.xy, 0.0, 1.0);
}

#endif

#if defined(FRAGMENT)

in vec2 uv;

uniform sampler2D Material;
uniform sampler2D Normal;
uniform sampler2D Depth;

uniform vec3 CameraPosition;
uniform float Time;

uniform vec3  LightPosition;
uniform vec3  LightDiffuseColor;
uniform vec3  LightSpecularColor;
uniform float LightIntensity;

uniform mat4 Projection;
uniform mat4 InverseViewProjection;

out vec4  Color;

//
// Light computations functions prototypes
//

vec3 computePointLight(	vec3 cameraPosition,
						vec3 fragPosition,
						vec3 fragNormal,
						vec3 fragDiffuseTextureColor,
						float fragSpecularFactor,
						vec3 lightPosition,
						vec3 lightDiffuseColor,
						vec3 lightSpecularColor,
						float lightIntensity);

vec3 computeDirectionnalLight(	vec3 cameraPosition,
								vec3 fragPosition,
								vec3 fragNormal,
								vec3 fragDiffuseTextureColor,
								float fragSpecularFactor,
								vec3 lightDirection,
								vec3 lightDiffuseColor,
								vec3 lightSpecularColor,
								float lightIntensity);

vec3 computeSpotLight(	vec3 cameraPosition,
						vec3 fragPosition,
						vec3 fragNormal,
						vec3 fragDiffuseTextureColor,
						float fragSpecularFactor,
						vec3 lightPosition,
						vec3 lightDirection,
						vec3 lightExternalAngle,
						vec3 lightInternalAngle,
						vec3 lightDiffuseColor,
						vec3 lightSpecularColor,
						float lightIntensity);

//
// Main
//

void main(void)
{
	// Recover fragment parameters
	vec3 fragNormal = texture(Normal, uv).xyz;
	vec4 material = texture (Material, uv);
		// Split material to recover diffuse & specular
		vec3 fragDiffuse = material.xyz;
		float fragSpecular = material.w;
	float depth = texture(Depth, uv).x;
	vec2  xy = uv * 2.0 -1.0;
	vec4  wPosition =  vec4(xy, depth * 2.0 -1.0, 1.0) * InverseViewProjection;
	vec3  fragPosition = vec3(wPosition/wPosition.w);

	// Apply light(s)
	//vec3 dirLight = computeDirectionnalLight(CameraPosition, fragPosition, fragNormal, fragDiffuse, fragSpecular, vec3(0.5, -0.5, -0.5), vec3(0.7, 0.4, 0.6), vec3(1, 1, 1), 1);
	vec3 pointLight = computePointLight(CameraPosition, fragPosition, fragNormal, fragDiffuse, fragSpecular, LightPosition, LightDiffuseColor, LightSpecularColor, LightIntensity);
	
	// Set the output color
	Color = vec4(pointLight, 1);
}

vec3 computePointLight(		vec3 cameraPosition,
							vec3 fragPosition,
							vec3 fragNormal,
							vec3 fragDiffuseTextureColor,
							float fragSpecularFactor,
							vec3 lightPosition,
							vec3 lightDiffuseColor,
							vec3 lightSpecularColor,
							float lightIntensity)
{
	// Calculate pointlight attenuation
	vec3 l =  lightPosition - fragPosition;
	float lightDistanceAttenuation = 16.f/pow(length(l), 2);
	// Pointlight calculation formula
	vec3 n = normalize(fragNormal);
	vec3 v = fragPosition - cameraPosition;
	vec3 h = normalize(l-v);
	float n_dot_l = clamp(dot(n, l), 0, 1.0);
	float n_dot_h = clamp(dot(n, h), 0, 1.0);
	return lightDistanceAttenuation * lightDiffuseColor * lightIntensity * (fragDiffuseTextureColor * n_dot_l + fragSpecularFactor * lightSpecularColor *  pow(n_dot_h, fragSpecularFactor * 100));
}

//
// Light computations functions definitions
// Note : In the light calculation formulas, we can also provide a LightSpecularFactor to replace the arbitrary value "100"

vec3 computeDirectionnalLight(	vec3 cameraPosition,
								vec3 fragPosition,
								vec3 fragNormal,
								vec3 fragDiffuseTextureColor,
								float fragSpecularFactor,
								vec3 lightDirection,
								vec3 lightDiffuseColor,
								vec3 lightSpecularColor,
								float lightIntensity)
{
	// Directionnallight calculation formula
	vec3 n = normalize(fragNormal);
	vec3 l =  -normalize(lightDirection);
	vec3 v = fragPosition - cameraPosition;
	vec3 h = normalize(l-v);
	float n_dot_l = clamp(dot(n, l), 0, 1.0);
	float n_dot_h = clamp(dot(n, h), 0, 1.0);
	return lightDiffuseColor * lightIntensity * (fragDiffuseTextureColor * n_dot_l + fragSpecularFactor * lightSpecularColor *  pow(n_dot_h, fragSpecularFactor * 100));
}

vec3 computeSpotLight(	vec3 cameraPosition,
						vec3 fragPosition,
						vec3 fragNormal,
						vec3 fragDiffuseTextureColor,
						float fragSpecularFactor,
						vec3 lightPosition,
						vec3 lightDirection,
						float lightExternalAngle,
						float lightInternalAngle,
						vec3 lightDiffuseColor,
						vec3 lightSpecularColor,
						float lightIntensity)					
{
	// Calculate spotlight attenuation
	vec3 light_to_frag = normalize(lightPosition - fragPosition);
	float cosA = dot(light_to_frag, -normalize(lightDirection)) / length(light_to_frag);
	float spotlightAttenuation;
	if(cosA < cos(lightExternalAngle))
		spotlightAttenuation = 0.f;
	else if(cosA > cos(lightExternalAngle) && cosA < cos(lightInternalAngle))
		spotlightAttenuation = pow((cosA - cos(lightExternalAngle)) / (cos(lightInternalAngle) - cos(lightExternalAngle)), 4);
	else
		spotlightAttenuation = 1.f;
	// Spotlight calculation formula
	vec3 n = normalize(fragNormal);
	vec3 l =  -normalize(lightDirection);
	vec3 v = fragPosition - cameraPosition;
	vec3 h = normalize(l-v);
	float n_dot_l = clamp(dot(n, l), 0, 1.0);
	float n_dot_h = clamp(dot(n, h), 0, 1.0);
	return spotlightAttenuation * lightDiffuseColor * lightIntensity * (fragDiffuseTextureColor * n_dot_l + fragSpecularFactor * lightSpecularColor *  pow(n_dot_h, fragSpecularFactor * 100));
}

#endif
