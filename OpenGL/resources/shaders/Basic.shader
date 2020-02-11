#shader vertex
#version 330 core

layout (location = 0) in vec3 position;
layout(location = 1) in float lavaH;
layout(location = 2) in vec4 color;
layout (location = 3) in vec2 texCoord;
layout(location = 4) in vec3 normal;

out vec2 v_TexCoord;
out vec4 ourColor;
out vec3 Normal;
out vec3 FragPos;
out float lava;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
   FragPos = vec3(model * vec4(position.x, position.y + lavaH, position.z, 1.0f)); 
   gl_Position = projection * view * vec4(FragPos, 1.0);
   ourColor = color;
   v_TexCoord = texCoord;
   
   Normal = mat3(transpose(inverse(model))) * normal;
   lava = lavaH;
};


#shader fragment
#version 330 core

layout (location = 0) out vec4 color;

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec2 v_TexCoord;
in vec4 ourColor;

in vec3 FragPos;
in vec3 Normal;
in float lava;

uniform vec3 viewPos;
uniform Light light;

uniform vec4 u_Color;
uniform sampler2D u_Texture;

void main()
{
    vec4 texColor = texture(u_Texture, v_TexCoord) * ourColor;
    //color = ourColor;
    //color = u_Color;
    //color = texColor;
    //color = vec4(0.4f, 0.4f, 0.4f, 1.0f);

    // Ambient
    vec3 ambient = light.ambient;

    // Diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff;

    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 1);
    vec3 specular = light.specular * spec;

    vec3 result = ambient + diffuse + specular;

    float thicknessRatio = lava / 15 / 0.2f;
    float RColor = 1.0f;
    float GColor = 0.6 * (1 - thicknessRatio);
    float BColor = 0.0f;
    if (lava < 0.001) {
        RColor = 0.5f;
        GColor = 0.5f;
        BColor = 0.5f;
    }
    vec3 tempColor = vec3(RColor, GColor, BColor) * result;

    color = vec4(tempColor, 1.0f);
};