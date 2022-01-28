#type vertex
#version 330 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Tangent;
layout (location = 3) in vec3 a_Binormal;
layout (location = 4) in vec2 a_TexCoord;
// TO DO: Upload model matrix to transform models

uniform mat4 u_ViewProjectionMatrix;
uniform mat4 u_Transform;

out VertexOutput
{
	vec3 WorldPosition;
    vec3 Normal;
	vec2 TexCoord;
	mat3 WorldNormals;
	mat3 WorldTransform;
	vec3 Binormal;
} vs_Output;

void main()
{
    // Calculate final gl (screen) position
    // TO DO: Include model matrix in calculation
	vs_Output.WorldPosition = vec3(u_Transform * vec4(a_Position, 1.0));
    vs_Output.Normal = a_Normal;
	vs_Output.TexCoord = vec2(a_TexCoord.x, 1.0 - a_TexCoord.y);
	vs_Output.WorldNormals = mat3(u_Transform) * mat3(a_Tangent, a_Binormal, a_Normal);
	vs_Output.WorldTransform = mat3(u_Transform);
    vs_Output.Binormal = a_Binormal;

    gl_Position = u_ViewProjectionMatrix * u_Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 330 core
out vec4 FragColor;

const float PI = 3.141592;
const float Epsilon = 0.00001;
vec3 Fdielectric = vec3(0.04);

const int LightCount = 1;

struct PointLight {
    vec3 Position;
    vec3 Radiance;
    float Intensity;
    float Radius;
    float Falloff;
};  


in VertexOutput
{
	vec3 WorldPosition;
    vec3 Normal;
	vec2 TexCoord;
	mat3 WorldNormals;
	mat3 WorldTransform;
	vec3 Binormal;
} vs_Input;

uniform sampler2D u_AlbedoTexture;
uniform sampler2D u_NormalTexture;
uniform sampler2D u_MetalnessTexture;
uniform sampler2D u_RoughnessTexture;
uniform sampler2D u_AoTexture;

uniform vec3 u_AlbedoColor;
uniform float u_Metalness;
uniform float u_Roughness;

uniform float u_AlbedoTexToggle;
uniform float u_NormalTexToggle;
uniform float u_MetalnessTexToggle;
uniform float u_RoughnessTexToggle;
uniform float u_AoTexToggle;

uniform PointLight u_PointLights[100];
uniform int u_PointLightCount;
uniform vec3 u_CameraPosition;

struct PBRParameters
{
	vec3 Albedo;
	float Roughness;
	float Metalness;
    float Ao;
	vec3 Normal;
	vec3 View;
	float NdotV;
};

PBRParameters m_Params;

float DistributionGGX(vec3 N, vec3 H, float a)
{
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float nom    = a2;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    denom        = PI * denom * denom;
	
    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float k)
{
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return nom / denom;
}
  
float GeometrySmith(vec3 N, vec3 V, vec3 L, float k)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, k);
    float ggx2 = GeometrySchlickGGX(NdotL, k);
	
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 Lighting(vec3 F0) {
    vec3 result = vec3(0.0);
    for(int i = 0; i < u_PointLightCount; i++) {

        PointLight light = u_PointLights[i];
        vec3 L = normalize(light.Position - vs_Input.WorldPosition);
        vec3 H = normalize(m_Params.View + L);

        float distance = length(light.Position - vs_Input.WorldPosition);

        float attenuation = clamp(1.0 - (distance * distance) / (light.Radius * light.Radius), 0.0, 1.0);
		attenuation *= mix(attenuation, 1.0, light.Falloff);

        vec3 radiance = light.Radiance * 20 * light.Intensity * attenuation;

        float NDF = DistributionGGX(m_Params.Normal,H,m_Params.Roughness);
        float G = GeometrySmith(m_Params.Normal,m_Params.View,L, m_Params.Roughness);
        vec3 F = fresnelSchlick(max(dot(H, m_Params.View), 0.0), F0);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - m_Params.Metalness;

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(m_Params.Normal,m_Params.View), 0.0) * max(dot(m_Params.Normal, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        float NdotL = max(dot(m_Params.Normal, L), 0.0);
        result += (kD * m_Params.Albedo / PI + specular) * radiance * NdotL;
    }
    return result;
}

void main()
{
    
    m_Params.Albedo = u_AlbedoTexToggle > 0.5 ? texture(u_AlbedoTexture, vs_Input.TexCoord).rgb : u_AlbedoColor;
    //m_Params.Metalness = 0.0;
    m_Params.Metalness = u_MetalnessTexToggle > 0.5 ? texture(u_MetalnessTexture, vs_Input.TexCoord).r : u_Metalness;
    m_Params.Roughness = u_RoughnessTexToggle > 0.5 ? texture(u_RoughnessTexture, vs_Input.TexCoord).r : u_Roughness;
    m_Params.Roughness = max(m_Params.Roughness, 0.05);
    m_Params.Ao = u_AoTexToggle > 0.5 ? texture(u_AoTexture, vs_Input.TexCoord).r : 1.0;

	m_Params.Normal = normalize(vs_Input.Normal);
	if (u_NormalTexToggle > 0.5)
	{
		m_Params.Normal = normalize(2.0 * texture(u_NormalTexture, vs_Input.TexCoord).rgb - 1.0);
		m_Params.Normal = normalize(vs_Input.WorldNormals * m_Params.Normal);
	}


    m_Params.View = normalize(u_CameraPosition - vs_Input.WorldPosition);
	m_Params.NdotV = max(dot(m_Params.Normal, m_Params.View), 0.0);

    vec3 F0 = mix(Fdielectric, m_Params.Albedo, m_Params.Metalness);

    vec3 ambient = vec3(0.03) * m_Params.Albedo * m_Params.Ao;

    vec3 lightContribution = Lighting(F0);
    //vec3 color = texture(u_AlbedoTexture, vs_Input.TexCoord).rgb;
    vec3 color = ambient + lightContribution;
	

    // Tone Mapping / HDR
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));  
    
    FragColor = vec4(color, 1.0);
    
}

