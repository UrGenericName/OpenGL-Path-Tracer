#version 430 core

out vec3 color;

void main()
{
    float x = 2 * (-0.5 + float((gl_VertexID & 1)));
    float y = 2 * (-0.5 + float((gl_VertexID & 2) >> 1));
    
    gl_Position = vec4(x, y, 0.0, 1.0);

    color = vec3(1.0f, 0.0f, 0.0f); 
    
}