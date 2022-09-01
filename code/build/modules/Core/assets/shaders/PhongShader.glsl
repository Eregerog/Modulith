#type vertex

#version 330 core

layout(location = 0)in vec3 a_Position;
in vec3 a_Normal;
in vec2 a_UV;

uniform mat4 u_P;
uniform mat4 u_V;
uniform mat4 u_M;
uniform mat4 u_N;

out vec3 v_Position;
out vec3 v_Normal;
out vec2 v_UV;

void main(){

    gl_Position = u_P * u_V * u_M * vec4(a_Position, 1.0);

    v_Position = (u_V * u_M * vec4(a_Position, 1.0)).xyz;
    v_Normal = (u_N * vec4(a_Normal, 1.0)).xyz;
    v_UV = a_UV;
}


#type fragment

#version 330 core

struct PointLight{
    vec3 Position;

    vec3 Color;
    float AmbientFactor;

    float Constant;
    float Linear;
    float Quadratic;
};

struct DirectionalLight{
    bool Exists;

    vec3 Direction;

    vec3 Color;
    float AmbientFactor;
};

struct Material{
    bool HasDiffuseTex;
    sampler2D DiffuseTex;
    vec3 DiffuseColor;

    bool HasSpecularTex;
    sampler2D SpecularTex;
    vec3 SpecularColor;

    float Shininess;
};

layout(location = 0) out vec4 color;

in vec3 v_Position;
in vec3 v_Normal;
in vec2 v_UV;

uniform Material u_Material;

uniform vec3 u_CameraPosition;

uniform DirectionalLight u_DirectionalLight;
uniform PointLight u_Lights[4];
uniform int u_LightCount;

const vec3 c_AmbientLightColor = vec3(1.0, 1.0, 1.0);

vec3 calcDirectionalLight(DirectionalLight light, vec3 fragNormal, vec3 fragToCamera, vec3 diffuseTexValue, vec3 specularTexValue){
    vec3 fragToLight = normalize(-light.Direction);

    float diffuseIntensity = max(0.0, dot(fragNormal, fragToLight));

    // We use blinn-phong for specular calculations
    vec3 halfwayDir = normalize(fragToLight + fragToCamera);
    float specularIntensity = pow(max(0.0, dot(fragNormal, halfwayDir)), max(1, u_Material.Shininess));

    vec3 ambient = light.AmbientFactor * light.Color * u_Material.DiffuseColor * diffuseTexValue;
    vec3 diffuse = diffuseIntensity * light.Color * u_Material.DiffuseColor * diffuseTexValue;
    vec3 specular = specularIntensity * light.Color * u_Material.SpecularColor * specularTexValue;

    return (ambient + diffuse + specular);
}

vec3 calcPointLight(PointLight light, vec3 fragNormal, vec3 fragToCamera, vec3 diffuseTexValue, vec3 specularTexValue){
    vec3 fragToLight = normalize(light.Position - v_Position);

    float diffuseIntensity = max(0.0, dot(fragNormal, fragToLight));

    // We use blinn-phong for specular calculations
    vec3 halfwayDir = normalize(fragToLight + fragToCamera);
    float specularIntensity = pow(max(0.0, dot(fragNormal, halfwayDir)), max(1, u_Material.Shininess));

    float distance = length(light.Position - v_Position);
    float attenuation = clamp(1.0 / (light.Constant + light.Linear * distance + light.Quadratic * distance * distance), 0.0, 1.0);

    vec3 ambient = attenuation * light.AmbientFactor * light.Color * u_Material.DiffuseColor * diffuseTexValue;
    vec3 diffuse = attenuation * diffuseIntensity * light.Color * u_Material.DiffuseColor * diffuseTexValue;
    vec3 specular = attenuation * specularIntensity * light.Color * u_Material.SpecularColor * specularTexValue;

    return (ambient + diffuse + specular);
}

void main(){
    vec3 fragNormal = normalize(v_Normal);
    vec3 fragToCamera = normalize(u_CameraPosition - v_Position);

    vec3 diffuseTexValue = (u_Material.HasDiffuseTex ? vec3(texture(u_Material.DiffuseTex, v_UV)) : vec3(1.0));
    vec3 specularTexValue =  (u_Material.HasSpecularTex ? vec3(texture(u_Material.SpecularTex, v_UV)) : vec3(1.0));

    // In case a texture has negative values, which cause undefined behaviour and differes per graphics card
    diffuseTexValue.x = max(0, diffuseTexValue.x);
    diffuseTexValue.y = max(0, diffuseTexValue.y);
    diffuseTexValue.z = max(0, diffuseTexValue.z);

    specularTexValue.x = max(0, specularTexValue.x);
    specularTexValue.y = max(0, specularTexValue.y);
    specularTexValue.z = max(0, specularTexValue.z);

    vec3 res = u_DirectionalLight.Exists ? calcDirectionalLight(u_DirectionalLight , fragNormal, fragToCamera, diffuseTexValue, specularTexValue) : vec3(0);

    for (int i = 0; i < u_LightCount; ++i){
        res += calcPointLight(u_Lights[i], fragNormal, fragToCamera, diffuseTexValue, specularTexValue);
    }

    float gamma = 2.2;

    color = vec4(pow(res, vec3(1.0/gamma)), 1.0);
}