#version 450                                  

layout (location = 0) in vec3 position; 
layout (location = 1) in vec2 textureCoords; 
layout (location = 2) in vec3 normals; 

out vec3 vs_position;
out vec2 vs_texcoord;
out vec3 vs_normal;            
                                              
uniform mat4 u_Model;                         
uniform mat4 u_Projection;                    
                                              
void main()                                   
{       
    gl_Position = vec4(position, 1.f);       
    vs_texcoord = textureCoords;
}