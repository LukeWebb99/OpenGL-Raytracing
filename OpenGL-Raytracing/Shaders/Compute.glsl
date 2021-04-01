#version 440

const float PI = 3.14159265f;
const float infinity = 1.0 / 0.0;
const float EPSILON = 1e-8;

struct PointLight {
    vec4 Colour;
    vec4 Pos;
};

struct Spotlight {
    vec4 Colour;
    vec3 Pos;
    vec3 Direction;
    float Cuttoff;
    float OuterCuttoff;
};

layout (local_size_x = 1, local_size_y = 1) in;
layout (rgba32f, binding = 0) uniform image2D img_output;
layout (std140) uniform PointLights {

   PointLight u_pointLights[4];

};

uniform mat4 u_cameraToWorld;
uniform mat4 u_cameraInverseProjection;

uniform vec2 u_pixelOffset;
uniform int u_rayBounceLimit;

//skybox textures
uniform sampler2D   u_skyboxTexture;
uniform sampler2D u_irradianceMap;

//PRB
uniform sampler2D u_AlbedoMap;
uniform sampler2D u_NormalMap;
uniform sampler2D u_RoughnessMap;
uniform sampler2D u_AOMap;
uniform sampler2D u_MetallicMap;

//SPOT LIGHT
uniform Spotlight u_spotlight;

//DIRECTIONAL LIGHT
uniform vec4 u_directionalLight;

//SHADOW
uniform bool u_toggleShadow;
uniform bool u_togglePlane;

uniform float u_seed;
float l_seed;
ivec2 pixel_coords;

bool hitsky = false;

float rand() {

    float result = fract(sin(l_seed / 100.0f * dot(pixel_coords, vec2(12.9898f, 78.233f))) * 43758.5453f);
    l_seed += 1.0f;
    return result * 0.25;
}

struct Sphere {
   float radius;
   vec3 position;
   vec3 albedo;
   vec3 specular;
   float roughness;
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
    vec3 albedo;
    vec3 normal;
	vec3 specular;
    float roughness;
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

void IntersectGroundPlane(Ray ray, inout RayHit bestHit) {

    // Calculate distance along the ray where the ground plane is intersected
    float t = -ray.origin.y / ray.direction.y;
    if (t > 0 && t < bestHit.distance) {
        bestHit.distance = t;
        bestHit.position = ray.origin + t * ray.direction;
        bestHit.normal = vec3(0.0f, 1.0f, 0.0f);
        bestHit.specular = vec3(0.5);
        bestHit.albedo = vec3(1.0);
        bestHit.roughness = 1.f;
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
        bestHit.albedo = pow(texture(u_AlbedoMap, bestHit.normal.xy).rgb, vec3(2.2));
        bestHit.specular = sphere.specular;
        bestHit.roughness = texture(u_RoughnessMap, bestHit.normal.xy).r;
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

RayHit Trace(Ray ray) {
    RayHit bestHit = CreateRayHit();
    
    if(u_togglePlane){
        IntersectGroundPlane(ray, bestHit);
    }

    IntersectSphere(ray, bestHit, u_sphere);
    
    // Trace single triangle
    //vec3 v0 = vec3(-15, 0, -15);
    //vec3 v1 = vec3( 15, 0, -15);
    //vec3 v2 = vec3(0, 15 * sqrt(2), -15);
    //float t, u, v;
    //if (IntersectTriangle(ray, v0, v1, v2, t, u, v)) {
    //
    //   if (t > 0 && t < bestHit.distance) {
    //     bestHit.distance = t;
    //     bestHit.position = ray.origin + t * ray.direction;
    //     bestHit.normal = normalize(cross(v1 - v0, v2 - v0));
    //     bestHit.albedo = vec3(0.55f);
    //     bestHit.specular = 0.65f * vec3(1, 0.4f, 0.2f);
    //     bestHit.roughtness = 0.0f;
    //     }
    // }

    return bestHit;

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

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {

    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}   

//LIGHT CALC
vec3 CalcDirectLight(RayHit hit) {

    Ray shadowRay = CreateRay(hit.position + hit.normal * 0.001f, -1 * u_directionalLight.xyz);
    RayHit shadowHit;     
    shadowHit = Trace(shadowRay); 

    if(shadowHit.distance == infinity) {
        return vec3(clamp(dot(hit.normal, u_directionalLight.xyz) * -1, 0.0, 1.0) * u_directionalLight.w);
    } else {
        return vec3(0.f);
    }
}

vec3 CalcPointLights(vec3 albedo, vec3 normal, float metallic, float roughness, float ao, inout Ray ray, RayHit hit) {

    vec3 N = normal;
    vec3 V = normalize(ray.origin - hit.position);
    vec3 R = reflect(-V, N); 

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);


    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4; i++) {
        // calculate per-light radiance
        vec3 L = normalize(u_pointLights[i].Pos.xyz -  hit.position);
        vec3 H = normalize(V + L);
        float distance = length(u_pointLights[i].Pos.xyz -  hit.position);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = u_pointLights[i].Colour.rgb * u_pointLights[i].Colour.w * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);   
        float G   = GeometrySmith(N, V, L, roughness);    
        vec3 F    = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);    
        
        vec3 nominator    = NDF * G * F;
        float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; // 0.001 to prevent divide by zero.
        vec3 specular = nominator / denominator;
        
         // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic;	                
            
        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);        

        //shadow
        Ray shadowRay = CreateRay(hit.position + hit.normal * 0.001f, u_pointLights[i].Pos.xyz);
        RayHit shadowHit = Trace(shadowRay);
        shadowHit.distance;
   
        // add to outgoing radiance Lo
        if(shadowHit.distance == infinity){
            Lo += (kD * albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
        } else {
            Lo += vec3(0.f);
        }
    }
      
    
    return Lo;

} 

//MAIN CALC
vec3 Shade(inout Ray ray, RayHit hit) {

   //https://youtu.be/WfLnEgxFj0M
   vec2 SphereNormals = vec2( acos( hit.normal.y) / -PI, atan(hit.normal.x, hit.normal.z) / -PI * 0.5f);

   if (hit.distance < infinity) {

        float l_metallic  = texture(u_MetallicMap, SphereNormals).r;
        float l_ao        = texture(u_AOMap, SphereNormals).r;

        ray.origin = hit.position + hit.normal * 0.001f;
        if(hit.roughness < 0.5f) {
            ray.direction = reflect(ray.direction, hit.normal) + (normalize(vec3(rand(), rand(), rand()))*hit.roughness*0.5);
        } else {
            ray.direction = vec3(0.f);
        }

        ray.energy *= (hit.albedo * l_ao)
        + CalcPointLights(hit.albedo, hit.normal, l_metallic, hit.roughness, l_ao, ray, hit)
        + CalcDirectLight(hit);

        vec3 F0 = vec3(0.04); 
        F0 = mix(F0, hit.albedo, l_metallic);

        return vec3(0.0);

    } else {
        
        hitsky = true;

        float theta = acos(ray.direction.y) / -PI;
        float phi = atan(ray.direction.x, -ray.direction.z) / -PI * 0.5f;

        // HDR tonemapping
        vec3 colour = texture(u_skyboxTexture, vec2(phi, theta)).xyz;
        colour = colour / (colour + vec3(1.0));
        // gamma correct
        colour = pow(colour, vec3(1.0/2.2)); 

        return colour;

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
    imageStore(img_output, pixel_coords, vec4(result, 1.0f));
}