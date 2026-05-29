#version 430 core

// 1. Explicitly layout the structures to match C++ alignment exactly (std430 rules)
struct Vertex {
    vec4 position; // 16 bytes (xyz + pad)
    vec4 color;    // 16 bytes (rgb + pad)
    vec4 normal;   // 16 bytes (xyz + pad)
};

layout(std430, binding = 0) readonly buffer VertexBuffer {
    Vertex vertices[];
};

layout(std430, binding = 1) readonly buffer IndexBuffer {
    uint indices[];
};

layout(std430, binding = 2) readonly buffer MeshHeaderBuffer {
    vec4 headers[];
};

out vec4 FragColor;

// Varyings from the Vertex Shader
in vec3 color;
in vec2 texCoord;
in vec3 faceNormal;
in vec3 intersectionPoint; // CRITICAL: This MUST be in World Space from the Vertex Shader!

// Uniforms
uniform vec3 sun;
uniform vec3 camPos; // CRITICAL: Ensure this updates every frame in C++!
uniform sampler2D albedo;
uniform sampler2D normal;
uniform sampler2D roughness;
uniform sampler2D metallic;
uniform float emissive;

#define MAX_BRIGHTNESS 0.8
#define MIN_BRIGHTNESS 0.4

// Function Forward Declarations
void normalSpaceToWorldSpace(vec3 point, vec3 faceNormal, vec2 texCoord, vec3 inVector, out vec3 outVector);
void calculateWorldSpaceTangentBitagent(vec3 point, vec3 faceNormal, vec2 texCoord, out vec3 tangent, out vec3 bitangent);
bool intersect_triangle(vec3 orig, vec3 dir, vec3 vert0, vec3 vert1, vec3 vert2, out vec3 result);

void main() {
    // -------------------------------------------------------------------------
    // CRITICAL FIX: Keep screen derivatives (dFdx/dFdy) at the absolute top of main().
    // Running texture/normal maps after an early 'return' branches pixel pipelines 
    // and produces corrupted/randomly offset normal vectors.
    // -------------------------------------------------------------------------
    vec3 normalMapVector;
    vec4 temp = texture(normal, texCoord);
    normalMapVector.x = 2.0f * temp.x - 1.0f;
    normalMapVector.y = 2.0f * temp.y - 1.0f;
    normalMapVector.z = 2.0f * temp.z - 1.0f;
    normalMapVector = normalize(normalMapVector);

    vec3 newFaceNormal;
    normalSpaceToWorldSpace(intersectionPoint, normalize(faceNormal), texCoord, normalMapVector, newFaceNormal);
    newFaceNormal = normalize(newFaceNormal);

    // Early exit for emissive geometry
    if (emissive == 1.0f) {
        FragColor = vec4(color, 1.0f);
        return;
    }

    // -------------------------------------------------------------------------
    // CAMERA SPECULAR REFLECTION LOGIC (FIXED)
    // -------------------------------------------------------------------------
    // 1. Calculate incoming vector pointing directly from the camera to the surface
    vec3 incidentRay = normalize(intersectionPoint - camPos);
    
    // 2. Reflect the incoming ray across the normal-mapped face vector
    vec3 reflectionBounceDir = reflect(incidentRay, newFaceNormal);

    vec3 result;
    // Hardcoded target triangle coordinates (Ensure these are in the same World Space!)
    vec3 v0 = vec3(1.538572, 6.705862, 0.444164);
    vec3 v1 = vec3(1.701151, 6.462544, 0.257621);
    vec3 v2 = vec3(1.538572, 6.478556, 0.257621);

    // 3. Cast the reflection ray out into the scene to see if it intersects the triangle
    // Offset the ray origin slightly along the shading normal to avoid self-intersection
    float ORIGIN_BIAS = 0.001;
    vec3 rayOrigin = intersectionPoint + newFaceNormal * ORIGIN_BIAS;

    if (intersect_triangle(rayOrigin, reflectionBounceDir, v0, v1, v2, result)) {
        FragColor = vec4(0.0f, 1.0f, 1.0f, 1.0f); // True specular hit highlights in Cyan
        return;
    }

    // Standard fallback lighting calculations if reflection misses
    float tempDot = dot(newFaceNormal, normalize(sun));
    float brightness = tempDot * (MAX_BRIGHTNESS - MIN_BRIGHTNESS) + MIN_BRIGHTNESS;
    FragColor = texture(albedo, texCoord) * vec4(color, 1.0f) * brightness;
}

// Transforms normal map tangents into world space vectors
void normalSpaceToWorldSpace(vec3 point, vec3 faceNormal, vec2 texCoord, vec3 inVector, out vec3 outVector) {
    vec3 tangent;
    vec3 bitangent;
    calculateWorldSpaceTangentBitagent(point, faceNormal, texCoord, tangent, bitangent);
    mat3 tangentToWorldSpaceMatrix = mat3(tangent, bitangent, faceNormal);
    outVector = normalize(tangentToWorldSpaceMatrix * inVector);
}

// Finds world space tangent and bitangent using stable screen derivatives
void calculateWorldSpaceTangentBitagent(vec3 point, vec3 faceNormal, vec2 texCoord, out vec3 tangent, out vec3 bitangent) {
    vec3 pointDx = dFdx(point);
    vec3 pointDy = dFdy(point);
    vec2 texCoordDx = dFdx(texCoord);
    vec2 texCoordDy = dFdy(texCoord);
    vec3 crossDx = cross(faceNormal, pointDx);
    vec3 crossDy = cross(pointDy, faceNormal);
    tangent = (crossDy * texCoordDx.x) + (crossDx * texCoordDy.x);
    bitangent = (crossDy * texCoordDx.y) + (crossDx * texCoordDy.y);
    float invmax = inversesqrt(max(dot(tangent, tangent), dot(bitangent, bitangent)));
    tangent = tangent * invmax;
    bitangent = bitangent * invmax;
}

// Pure Möller-Trumbore ray-triangle intersection algorithm
bool intersect_triangle(vec3 orig, vec3 dir, vec3 vert0, vec3 vert1, vec3 vert2, out vec3 result) {
    const float EPSILON = 0.000001f;
    vec3 edge1 = vert1 - vert0;
    vec3 edge2 = vert2 - vert0;
    vec3 pvec = cross(dir, edge2);
    float det = dot(edge1, pvec);
    
    // Note: If you want a two-sided mirror reflection test, change this line to: 
    // if (abs(det) < EPSILON) return false;
    if (det < EPSILON) return false;
    
    vec3 tvec = orig - vert0;
    float u = dot(tvec, pvec);
    if (u < 0.0f || u > det) return false;
    
    vec3 qvec = cross(tvec, edge1);
    float v = dot(dir, qvec);
    if (v < 0.0f || (u + v) > det) return false;
    
    float t = dot(edge2, qvec);
    float inv_det = 1.0 / det;
    t *= inv_det;
    u *= inv_det;
    v *= inv_det;

    // Only accept intersections in front of the ray origin and beyond a small threshold
    const float MIN_T = 0.001;
    if (t <= MIN_T) return false;

    result[0] = t;
    result[1] = u;
    result[2] = v;
    return true;
}
