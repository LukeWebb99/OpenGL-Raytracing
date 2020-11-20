#version 440
const float PI = 3.14159265f;
const float infinity = 1.0 / 0.0;

layout (local_size_x = 1, local_size_y = 1) in;
layout (rgba32f, binding = 0) uniform image2D img_output;

uniform mat4 u_cameraToWorld;
uniform mat4 u_cameraInverseProjection;

uniform sampler2D u_skyboxTexture;

uniform int u_rayBounceLimit;

uniform vec3 u_specular;

uniform vec3 u_albedo;
uniform vec4 u_directionalLight;

uniform bool u_toggleShadow;
uniform bool u_togglePlane;

bool hitsky = false;


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
};

RayHit CreateRayHit() {
    RayHit hit;
    hit.position = vec3(0.0f, 0.0f, 0.0f);
    hit.distance = infinity;
    hit.normal = vec3(0.0f, 0.0f, 0.0f);
    return hit;
}

void IntersectGroundPlane(Ray ray, inout RayHit bestHit) {

    // Calculate distance along the ray where the ground plane is intersected
    float t = -ray.origin.y / ray.direction.y;
    if (t > 0 && t < bestHit.distance) {
        bestHit.distance = t;
        bestHit.position = ray.origin + t * ray.direction;
        bestHit.normal = vec3(0.0f, 1.0f, 0.0f);
    }
}

void IntersectSphere(Ray ray, inout RayHit bestHit, vec4 sphere) {

    // Calculate distance along the ray where the sphere is intersected
    vec3 d = ray.origin - sphere.xyz;
    float p1 = -dot(ray.direction, d);
    float p2sqr = p1 * p1 - dot(d, d) + sphere.w * sphere.w;

    if (p2sqr < 0) return;

    float p2 = sqrt(p2sqr);
    float t = p1 - p2 > 0 ? p1 - p2 : p1 + p2;
    if (t > 0 && t < bestHit.distance)
    {
        bestHit.distance = t;
        bestHit.position = ray.origin + t * ray.direction;
        bestHit.normal = normalize(bestHit.position - sphere.xyz);
    }
}

RayHit Trace(Ray ray) {
    RayHit bestHit = CreateRayHit();
    if(u_togglePlane){
        IntersectGroundPlane(ray, bestHit);
    }
    IntersectSphere(ray, bestHit, vec4(0, 3.0f, 0, 0.5f));
    return bestHit;
}

vec3 Shade(inout Ray ray, RayHit hit) {

    if (hit.distance < infinity) {
        
        vec3 specular = u_specular;
        vec3 albedo = u_albedo;

        // Reflect the ray and multiply energy with specular reflection
        ray.origin = hit.position + hit.normal * 0.001f;
        ray.direction = reflect(ray.direction, hit.normal);
        ray.energy *= specular;
       
       // Shadow test ray
       Ray shadowRay = CreateRay(hit.position + hit.normal * 0.001f, -1 * u_directionalLight.xyz);
       RayHit shadowHit = Trace(shadowRay);
       if (shadowHit.distance != infinity && u_toggleShadow)
       {
           return vec3(0.0f);
       }

        // Return the normal
        return vec3(clamp(dot(hit.normal, u_directionalLight.xyz) * -1, 0.0, 1.0) * u_directionalLight.w * albedo);

    } else {
        
        hitsky = true;

        float theta = acos(ray.direction.y) / -PI;
        float phi = atan(ray.direction.x, -ray.direction.z) / -PI * 0.5f;
        
        return (texture(u_skyboxTexture, vec2(-phi, -theta))).xyz;
    }

}



void main () {
   
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);

    // Transform pixel to [-1,1] range
    vec2 uv = vec2((pixel_coords.xy + vec2(0.5f, 0.5f)) / imageSize(img_output) * 2.0f - 1.0f);

    // Create View Ray
    Ray ray = CreateCameraRay(uv);

    // Trace and shade
    vec3 result = vec3(0.f);
    for (int i = 0; i < u_rayBounceLimit; i++) {

        RayHit hit = Trace(ray); // Test if ray hit anything
        result += ray.energy * Shade(ray, hit); // Gen pixal colour 

        if(hitsky)  // If ray hits skybox set energy to null
           ray.energy = vec3(0);
  
        if (ray.energy.x == 0 && ray.energy.y == 0 && ray.energy.z == 0) // If ray-enery.xyz has no energy break;
            break;

    }
    
    // Store in outgoing texture
    imageStore(img_output, pixel_coords, vec4(result, 0.1));
}