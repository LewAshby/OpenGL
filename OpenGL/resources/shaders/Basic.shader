#shader vertex
#version 330 core

layout (location = 0) in vec4 position;
layout(location = 1) in vec4 color;
layout (location = 2) in vec2 texCoord;

out vec2 v_TexCoord;
out vec4 ourColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
   gl_Position = projection * view * position;
   ourColor = color;
   v_TexCoord = texCoord;
};


#shader fragment
#version 330 core

layout (location = 0) out vec4 color;

in vec2 v_TexCoord;
in vec4 ourColor;

uniform vec4 u_Color;
uniform sampler2D u_Texture;

void main()
{
    vec4 texColor = texture(u_Texture, v_TexCoord);
    color = ourColor;
    //color = u_Color;
    //color = texColor;
    //color = vec4(0.4f, 0.4f, 0.4f, 1.0f);
};