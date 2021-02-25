#version 450                                   

out vec4 colour;                               

in vec2 vs_texcoord;
in vec3 vs_normal;   

uniform float u_sample;        
uniform bool u_reset; 
uniform sampler2D u_TextureA;

          
void main() {      
//0.1f / (u_sample + 1.0f)
    colour = vec4(texture(u_TextureA, vs_texcoord).xyz, (1.f / (u_sample + 1.f)));
    
}