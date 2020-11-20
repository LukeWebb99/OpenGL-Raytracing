#version 450                                   

out vec4 colour;                               

in vec2 vs_texcoord;
in vec3 vs_normal;   
          
uniform sampler2D u_TextureA;
uniform sampler2D u_TextureB;
          
void main() {                                              
    colour = texture(u_TextureA, vs_texcoord); 
}