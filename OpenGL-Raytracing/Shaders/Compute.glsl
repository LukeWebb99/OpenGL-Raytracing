#version 440

const float PI = 3.14159265f;
const float infinity = 1.0 / 0.0;
const float EPSILON = 1e-8;

struct PointLight {
    vec4 colour;
    vec4 Pos;
};

struct SpotLight {
	vec3  m_Position;
	vec3  m_Direction;
	float m_CutOff;
};

layout (local_size_x = 1, local_size_y = 1) in;
layout (rgba32f, binding = 0) uniform image2D img_output;
layout (std140) uniform PointLights {

   PointLight u_pointLights[4];

};

uniform mat4 u_cameraToWorld;
uniform mat4 u_cameraInverseProjection;

uniform sampler2D u_skyboxTexture;

uniform vec2 u_pixelOffset;
uniform int u_rayBounceLimit;

//MESH
//uniform mat4x4 u_localToWorldMatrix;
//uniform int u_verticesSize;
//uniform vec3 u_vertices[MAX_VERTEX_ARRAY_SIZE];
//uniform int u_indciesSize;
//uniform int u_indices[MAX_INDICES_ARRAY_SIZE];

uniform sampler2D albedoMap;

//POINT LIGHT
uniform float u_metallic;
uniform float u_ao;
uniform float u_roughness;

//DIRECTIONAL LIGHT
uniform vec4 u_directionalLight;

//SHADOW
uniform bool u_toggleShadow;
uniform bool u_togglePlane;

uniform float u_seed;
float l_seed;
ivec2 pixel_coords;

bool hitsky = false;

uniform vec3 u_spotlightPos;
uniform vec3 u_spotlightDir;
uniform float u_spotlightCuttoff;
uniform float u_spotlightOuterCuttoff;
uniform float u_spotlightIntensity;
uniform vec3 u_spotlightColour;

float rand() {

    float result = fract(sin(l_seed / 100.0f * dot(pixel_coords, vec2(12.9898f, 78.233f))) * 43758.5453f);
    l_seed += 1.0f;
    return result;
}

float random (vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}

mat3x3 GetTangentSpace(vec3 normal) {
   // Choose a helper vector for the cross product
    vec3 helper = vec3(1, 0, 0);
    if (abs(normal.x) > 1.f)
        helper = vec3(0, 0, 1);

    // Generate vectors
    vec3 tangent = normalize(cross(normal, normal));
    vec3 binormal = normalize(cross(normal, tangent));
    return mat3x3(tangent, binormal, normal);
}

struct Sphere {
   float radius;
   vec3 position;
   vec3 albedo;
   vec3 specular;
   float roughtness;
};

uniform Sphere u_sphere;

struct Ray {
    vec3 origin;
    vec3 direction;
    vec3 energy;
};

Ray CreateRay(vec3 origin, vec3 direction) {
    Ray ray;
    ray.origin = origin;
    ray.direction = direction;
    ray.energy = vec3(1.0f, 1.0f, 1.0f);
    return ray;
}

Ray CreateCameraRay(vec2 uv) {

    //camera orgin to world space
    vec3 origin = (u_cameraToWorld * vec4(0.f, 0.f, 0.f, 1.f)).xyz;

    //invert prespective
    vec3 direction = (u_cameraInverseProjection * vec4(uv, 0.f, 1.f)).xyz;

    // Transform the direction from camera to world space and normalize
    direction = (u_cameraToWorld * vec4(direction, 0.0f)).xyz;
    direction = normalize(direction);

    return CreateRay(origin, direction);
}

struct RayHit {
    vec3 position;
    float distance;
    vec3 normal;
	vec3 specular;
    vec3 albedo;
    float roughtness;
};

RayHit CreateRayHit() {
    RayHit hit;
    hit.position = vec3(0.0f, 0.0f, 0.0f);
    hit.distance = infinity;
    hit.normal = vec3(0.0f, 0.0f, 0.0f);
    hit.specular = vec3(0.f, 0.f, 0.f);
    hit.albedo = vec3(0.f, 0.f, 0.f);
    return hit;
}

vec3 SampleHemisphere(vec3 normal, float alpha) {

    // Sample the hemisphere, where alpha determines the kind of the sampling
    float cosTheta = pow(rand(), 1.0f / (alpha));
    float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
    float phi = PI * rand();
    vec3 tangentSpaceDir = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
    // Transform direction to world space
    return (GetTangentSpace(normal) * tangentSpaceDir);
}

void IntersectGroundPlane(Ray ray, inout RayHit bestHit) {

    // Calculate distance along the ray where the ground plane is intersected
    float t = -ray.origin.y / ray.direction.y;
    if (t > 0 && t < bestHit.distance) {
        bestHit.distance = t;
        bestHit.position = ray.origin + t * ray.direction;
        bestHit.normal = vec3(0.0f, 1.0f, 0.0f);
        bestHit.specular = vec3(0.5);
        bestHit.albedo = vec3(0.1);
        bestHit.roughtness = 255.f;
    }
}

void IntersectSphere(Ray ray, inout RayHit bestHit, Sphere sphere) {

    // Calculate distance along the ray where the sphere is intersected
    vec3 d = ray.origin - sphere.position;
    float p1 = -dot(ray.direction, d);
    float p2sqr = p1 * p1 - dot(d, d) + sphere.radius * sphere.radius;

    if (p2sqr < 0)
        return;

    float p2 = sqrt(p2sqr);
    float t = p1 - p2 > 0 ? p1 - p2 : p1 + p2;
    if (t > 0 && t < bestHit.distance)
    {
        bestHit.distance = t;
        bestHit.position = ray.origin + t * ray.direction;
        bestHit.normal = normalize(bestHit.position - sphere.position);
        bestHit.albedo = sphere.albedo;
        bestHit.specular = sphere.specular;
        bestHit.roughtness = sphere.roughtness;
    }
}

bool IntersectTriangle(Ray ray, vec3 vert0, vec3 vert1, vec3 vert2, inout float t, inout float u, inout float v) {

    // find vectors for two edges sharing vert0
    vec3 edge1 = vert1 - vert0;
    vec3 edge2 = vert2 - vert0;
    // begin calculating determinant - also used to calculate U parameter
    vec3 pvec = cross(ray.direction, edge2);
    // if determinant is near zero, ray lies in plane of triangle
    float det = dot(edge1, pvec);
    // use backface culling
    if (det < EPSILON)
        return false;
    float inv_det = 1.0f / det;
    // calculate distance from vert0 to ray origin
    vec3 tvec = ray.origin - vert0;
    // calculate U parameter and test bounds
    u = dot(tvec, pvec) * inv_det;
    if (u < 0.0 || u > 1.0f)
        return false;
    // prepare to test V parameter
    vec3 qvec = cross(tvec, edge1);
    // calculate V parameter and test bounds
    v = dot(ray.direction, qvec) * inv_det;
    if (v < 0.0 || u + v > 1.0f)
        return false;
    // calculate t, ray intersects triangle
    t = dot(edge2, qvec) * inv_det;
    return true;
}

/*
void IntersectMeshObject(Ray ray, inout RayHit bestHit) {
   
    for (uint i = 0; i < u_indciesSize; i += 3)
    { 
        vec3 v0 = (vec4(u_vertices[u_indices[i+0]], 1.f)).xyz;
        vec3 v1 = (vec4(u_vertices[u_indices[i+1]], 1.f)).xyz;
        vec3 v2 = (vec4(u_vertices[u_indices[i+2]], 1.f)).xyz;
        //verts[u_indices[i+0]].m_vertices;
        float t, u, v;
        if (IntersectTriangle(ray, v0, v1, v2, t, u, v))
        {
            if (t > 0 && t < bestHit.distance)
            {
                bestHit.distance = t;
                bestHit.position = ray.origin + t * ray.direction;
                bestHit.normal = normalize(cross(v1 - v0, v2 - v0));
                bestHit.albedo = vec3(0.5f);
                bestHit.specular = vec3(0.65f, 0, 1);
                bestHit.roughtness = 25.f;
            }
        }
    }
}
*/

RayHit Trace(Ray ray) {
    RayHit bestHit = CreateRayHit();
    
    if(u_togglePlane){
        IntersectGroundPlane(ray, bestHit);
    }

    IntersectSphere(ray, bestHit, u_sphere);
    
    // Trace single triangle
    vec3 v0 = vec3(-15, 0, -15);
    vec3 v1 = vec3( 15, 0, -15);
    vec3 v2 = vec3(0, 15 * sqrt(2), -15);
    float t, u, v;
    if (IntersectTriangle(ray, v0, v1, v2, t, u, v)) {

       if (t > 0 && t < bestHit.distance) {
         bestHit.distance = t;
         bestHit.position = ray.origin + t * ray.direction;
         bestHit.normal = normalize(cross(v1 - v0, v2 - v0));
         bestHit.albedo = vec3(0.55f);
         bestHit.specular = 0.65f * vec3(1, 0.4f, 0.2f);
         bestHit.roughtness = 0.0f;
         }
     }

    return bestHit;

}

float sdot(vec3 x, vec3 y, float f) {
    return clamp(dot(x, y) * f, 0, 1);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
} 

float DistributionGGX(vec3 N, vec3 H, float roughness) {

    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {

    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {

    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 CalculateSpotLights(RayHit hit){
  
    vec3 dir = normalize(hit.position - u_spotlightPos);
    float theta = dot(dir, u_spotlightDir);
    float epsilon   = u_spotlightCuttoff - u_spotlightOuterCuttoff;
    float hardness = clamp((theta - u_spotlightCuttoff) / epsilon, 0.0, 1.0);

    if(theta > u_spotlightCuttoff){
        return u_spotlightColour * hardness * u_spotlightIntensity;
    } else {
        return vec3(0.f);
    }

}

vec3 CalculatePointLights(vec3 albedo, vec3 origin, vec3 position, vec3 normal){

           vec3 N = normalize(normal); 
           vec3 V = normalize(origin - position);

           vec3 F0 = vec3(0.04); 
           F0 = mix(F0, albedo, u_metallic);

           vec3 Lo = vec3(0.f);

           for(uint i = 0; i < 4; i++) {
               
               vec3 L = normalize(u_pointLights[i].Pos.xyz - position);
               vec3 H = normalize(V + L);
               
               float distance    = length(u_pointLights[i].Pos.xyz - position);
               float attenuation = 1.0 / (distance * distance);
               vec3 radiance     = u_pointLights[i].colour.xyz * attenuation; 

               // cook-torrance brdf
               float NDF = DistributionGGX(N, H, u_roughness);        
               float G   = GeometrySmith(N, V, L, u_roughness);      
               vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);  

               vec3 kS = F;
               vec3 kD = vec3(1.0) - kS;
               kD *= 1.0 - u_metallic;	  
               
               vec3 numerator    = NDF * G * F;
               float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
               vec3 specular     = numerator / max(denominator, 0.001);

               // add to outgoing radiance Lo
               float NdotL = max(dot(N, L), 0.0);                
               Lo += (kD * albedo / PI + specular) * radiance * NdotL * albedo * u_pointLights[i].colour.w*u_pointLights[i].colour.w; 

           }
           vec3 ambient = vec3(0.3) * albedo * u_ao;
           vec3 color = ambient + Lo;
	       
           //color = color / (color + vec3(1.0));
           //color = pow(color, vec3(1.0/2.2));

           return color;

}

vec3 CalculateDirectLight(RayHit hit){
      return vec3(clamp(dot(hit.normal, u_directionalLight.xyz) * -1, 0.0, 1.0) * u_directionalLight.w * hit.albedo);
}

bool Shadow(inout Ray ray, RayHit hit) {

     Ray shadowRay0 = CreateRay(hit.position + hit.normal * 0.001f, u_pointLights[0].Pos.xyz);
     RayHit shadowHit0;                                              
     
     if(u_pointLights[0].colour.w > 0){
         shadowHit0 = Trace(shadowRay0);                                              
     } 

     Ray shadowRay4    = CreateRay(hit.position + hit.normal * 0.001f, -1 * u_directionalLight.xyz);
     RayHit shadowHit4;

     if(u_directionalLight.w > 0){
         shadowHit4 = Trace(shadowRay4);
     } 

     if (shadowHit4.distance != infinity && shadowHit0.distance != infinity)
     {
         return true;
     }   
     else {
         return false;
     }

     
}

vec3 Shade(inout Ray ray, RayHit hit) {

  
   if (hit.distance < infinity) {
       
       if(Shadow(ray, hit) && u_toggleShadow){
           return vec3(0.f);
       }

      vec3 rColour = vec3(1.f);
      rColour += CalculateDirectLight(hit);
      rColour += CalculatePointLights(hit.albedo, ray.origin, hit.position, hit.normal);
      rColour += CalculateSpotLights(hit);

       if(hit.roughtness < 249.9){
          ray.origin = hit.position + hit.normal * 0.001f;
          ray.direction = reflect(ray.direction, hit.normal) + vec3(rand()*hit.roughtness*0.001, rand()*hit.roughtness*0.001, rand()*hit.roughtness*0.001);
          ray.energy *=  hit.specular * sdot(hit.normal, ray.direction, 1) * rColour;
       } else {
           ray.origin = hit.position + hit.normal * 0.001f;
           ray.direction = vec3(0.f);
           ray.energy *=  hit.specular * rColour;
       }

       return vec3(0.f);
       
    } else {
        
        hitsky = true;
        float theta = acos(ray.direction.y) / -PI;
        float phi = atan(ray.direction.x, -ray.direction.z) / -PI * 0.5f;
       
        //return vec3(textureLod(u_skyboxTexture, vec2(-phi, -theta), 3)); 
        return vec3(texture(u_skyboxTexture, vec2(-phi, -theta)).xyz);
    }

}

void main () {

    l_seed = u_seed;
    pixel_coords = ivec2(gl_GlobalInvocationID.xy);

   // Transform pixel to [-1,1] range
   vec2 uv = vec2((pixel_coords.xy + u_pixelOffset) / imageSize(img_output) * 2.0f - 1.0f);

   // Create View Ray
   Ray ray = CreateCameraRay(uv);

   // Trace and Shade
   vec3 result = vec3(0.f, 0.f, 0.f);
   for (uint i = 0; i < u_rayBounceLimit; i++) {
   
       RayHit hit = Trace(ray); // Test if ray hit anything
       result += ray.energy * Shade(ray, hit);// Gen pixal colour 

       if(hitsky == true){
          ray.energy = vec3(0.f);
          break;
       }  // If ray hits skybox set energy to null
   }
    
    // Store in outgoing texture
    imageStore(img_output, pixel_coords, vec4(result.xyz, 1.0f));
}