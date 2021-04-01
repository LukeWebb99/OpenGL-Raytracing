#version 450                                   

out vec4 colour;                               

in vec2 vs_texcoord;
in vec3 vs_normal;   

uniform sampler2D u_TextureA;
uniform vec2 u_Resolution;
uniform float u_sample;        

float kernel[9] = float[](
        0.0625,  0.125,  0.0625,
        0.125,  0.25, 0.125,
        0.0625,  0.125,  0.0625
);

vec4 ImageKernal(vec2 texcoords) {

	vec4 sum = vec4(0.f);
	vec2 stepSize = 1.1 / (u_Resolution);

	sum += texture2D(u_TextureA, vec2(texcoords.x - stepSize.x, texcoords.y - stepSize.y))   * kernel[0];
    sum += texture2D(u_TextureA, vec2(texcoords.x, texcoords.y - stepSize.y))                * kernel[1];
    sum += texture2D(u_TextureA, vec2(texcoords.x + stepSize.x, texcoords.y - stepSize.y))   * kernel[2];
																						
	sum += texture2D(u_TextureA, vec2(texcoords.x - stepSize.x, texcoords.y))                * kernel[3];
    sum += texture2D(u_TextureA, vec2(texcoords.x, texcoords.y))                             * kernel[4];
    sum += texture2D(u_TextureA, vec2(texcoords.x + stepSize.x, texcoords.y))                * kernel[5];
																							
    sum += texture2D(u_TextureA, vec2(texcoords.x - stepSize.x, texcoords.y + stepSize.y))   * kernel[6];
    sum += texture2D(u_TextureA, vec2(texcoords.x, texcoords.y + stepSize.y))                * kernel[7];
    sum += texture2D(u_TextureA, vec2(texcoords.x + stepSize.x, texcoords.y + stepSize.y))   * kernel[8];

	sum.a = 1.0;

	return sum;
}
     
void main() {      
    
    vec4 blur = ImageKernal(vs_texcoord);
    colour = vec4(blur.rgb, (1.f / (u_sample + 1.f)));

}