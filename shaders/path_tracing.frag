#version 430 core

struct Vertex {
    vec4 position;
    vec4 color;
    vec4 normal;
    vec2 texUV;
    vec2 _pad; // padding to match CPU
};

// LAYOUTS
layout(std430, binding = 0) readonly buffer VertexBuffer {
    Vertex vertices[];
};

layout(std430, binding = 1) readonly buffer IndexBuffer {
    uint indices[];
};
/*
layout(std430, binding = 2) readonly buffer TextureHandlesBuffer { 
    uint64_t textureHandles[]; 
};*/

layout(std430, binding = 3) readonly buffer MeshHeaderBuffer {
    vec4 meshHeader[];
};

// OUTPUT
out vec4 FragColor;

// INPUTS
in vec3 color;
in vec2 texCoord;
in vec3 geometricFaceNormal;
in vec3 intersectionPoint;

// UNIFORMS
uniform uint currentMesh;
uniform vec3 sun;
uniform vec3 camPos;
uniform sampler2D albedo;
uniform sampler2D normal;
uniform sampler2D roughness;
uniform sampler2D metallic;
uniform float emissive;

// DECLERATIONS
void normalSpaceToWorldSpace(vec3 point, vec3 faceNormal, vec2 texCoord, vec3 inVector, out vec3 outVector);
void calculateWorldSpaceTangentBitagent(vec3 point, vec3 faceNormal, vec2 texCoord, out vec3 tangent, out vec3 bitangent);
bool intersect_triangle(vec3 orig, vec3 dir, vec3 vert0, vec3 vert1, vec3 vert2, out vec3 result);

#define MAX_BRIGHTNESS 0.8
#define MIN_BRIGHTNESS 0.4

//#define DOUBLE_SIDED_REFLECTION

#define EPSILON 0.000001f

void main() {

    // Early exit for emissive geometry
    if (emissive == 1.0f) {
        FragColor = vec4(color, 1.0f);
        return;
    }

    // Normal Map Local Vector
    vec3 normalMapVector;
    {
        vec4 temp = texture(normal, texCoord);
        normalMapVector.x = 2.0f * temp.x - 1.0f;
        normalMapVector.y = 2.0f * temp.y - 1.0f;
        normalMapVector.z = 2.0f * temp.z - 1.0f;
        normalMapVector = normalize(normalMapVector);
    }

    // Calculates face normal from normal map
    vec3 faceNormal;
    {
        normalSpaceToWorldSpace(intersectionPoint, normalize(geometricFaceNormal), texCoord, normalMapVector, faceNormal);
        faceNormal = normalize(faceNormal);
    }

    // Calculates reflection
    {

        vec3 incidentRay = normalize(intersectionPoint - camPos);
        vec3 reflectionBounceDir = reflect(incidentRay, faceNormal);

        uint meshCount = meshHeader.length();
        for (uint i = 0u; i < meshCount; ++i) {
            
            if (i == currentMesh) continue;

            uint trigCount = uint(meshHeader[i].y) / 3u;
            for (uint j = 0u; j < trigCount; ++j) {

                uint indexStartPointer = uint(meshHeader[i].x);

                vec3 v0 = vertices[indexStartPointer + (3*j) + 0].position.xyz;
                vec3 v1 = vertices[indexStartPointer + (3*j) + 1].position.xyz;
                vec3 v2 = vertices[indexStartPointer + (3*j) + 2].position.xyz;

                vec3 result;
                if (intersect_triangle(intersectionPoint + (-faceNormal * EPSILON), reflectionBounceDir, v0, v1, v2, result)) {

                    float tempDot = dot(faceNormal, normalize(sun));
                    float brightness = tempDot * (MAX_BRIGHTNESS - MIN_BRIGHTNESS) + MIN_BRIGHTNESS;
                    FragColor = (texture(albedo, texCoord) * vec4(color, 1.0f) * brightness) * vec4(1.3f, 1.0f, 1.0f, 1.0f);

                    return;
                }

            }
        }
    }



    /*
                vec3 v0 = vertices[0].position.xyz;
            vec3 v1 = vertices[1].position.xyz;
            vec3 v2 = vertices[2].position.xyz;

            vec3 result;
            if (intersect_triangle(intersectionPoint, reflectionBounceDir, v0, v1, v2, result)) {
                FragColor = vec4(0.0f, 1.0f, 1.0f, 1.0f);
                return;
            }*/

    float tempDot = dot(faceNormal, normalize(sun));
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

    vec3 edge1 = vert1 - vert0;
    vec3 edge2 = vert2 - vert0;
    vec3 pvec = cross(dir, edge2);
    float det = dot(edge1, pvec);
    
    #ifdef DOUBLE_SIDED_REFLECTION
        if (abs(det) < EPSILON) return false;
    #else
        if (det < EPSILON) return false;
    #endif
    
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