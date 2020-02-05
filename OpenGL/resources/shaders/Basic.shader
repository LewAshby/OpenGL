#shader vertex
#version 330 core

layout (location = 0) in vec4 position;
layout(location = 1) in vec4 color;
layout (location = 2) in vec2 texCoord;
layout(location = 3) in vec3 normal;

out vec2 v_TexCoord;
out vec4 ourColor;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
   gl_Position = projection * view * model * position;
   ourColor = color;
   v_TexCoord = texCoord;
   FragPos = vec3(model * position); // since lighting is evaluated in world space
   Normal = mat3(transpose(inverse(model))) * normal;
};


#shader fragment
#version 330 core

layout (location = 0) out vec4 color;

in vec2 v_TexCoord;
in vec4 ourColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

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
    float ambientStrength = 0.1f;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse 
    vec3 norm = normalize(Normal);

    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular
    float specularStrength = 0.5f;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular);
    color = vec4(result, 1.0f) * ourColor;
};