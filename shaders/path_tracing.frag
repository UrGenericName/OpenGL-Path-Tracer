#version 430 core

layout(early_fragment_tests) in;

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

layout(std430, binding = 2) readonly buffer MeshTextureBuffer {
    vec4 meshTextures[]; 
};

layout(std430, binding = 3) readonly buffer MeshHeaderBuffer {
    vec4 meshHeader[];
};

layout(std430, binding = 4) coherent buffer highlightedMeshBuffer {
    uint highlightedMesh;
};

// OUTPUT
layout(location = 0) out vec4 FragColor;

// INPUTS
in vec3 color;
in vec2 texCoord;
in vec3 geometricFaceNormal;
in vec3 intersectionPoint;
in vec3 rayOrientation;

layout(binding = 0) uniform sampler2DArray texturePool;
layout(binding = 1) uniform sampler2D colorNoise;
layout(rgba32f, binding = 2) uniform image2D frameBuffer;

// UNIFORMS
uniform uint u_debugMode;
uniform bool u_debugLambertian;
uniform bool u_debugUniversalRoughness;
uniform float u_debugUniversalRoughnessAmount;
uniform ivec2 u_debugMousePos;
uniform bool u_debugMouseLeftClick;

uniform uint u_maxBounces;  
uniform uint u_currentSample;

uniform uint u_albedo;
uniform uint u_normal;
uniform uint u_roughness;
uniform uint u_metallic;
uniform float u_emissive;
uniform uint u_currentMesh;

uniform uint u_seed;
uniform vec3 u_backgroundColor;

uniform vec3 u_camPos;
uniform vec3 u_camOrientation;     

#define MAX_BRIGHTNESS 1.0f
#define MIN_BRIGHTNESS 0.0f

#define MAX_ROUGHNESS 1.0f
#define MIN_ROUGHNESS 0.0f

#define EPSILON 0.000001
#define FAR_PLANE 999999

// DECLERATIONS
bool drawDebug(uint debugMode);
void calculateSample(out vec4 outputColor);
void randomizeNormal(uint seed, float roughness, inout vec3 normal);
void calculatePath(vec3 init_Intersection, vec3 init_Origin, vec3 init_FaceNormal, out float totalBrightness, out vec3 totalReflectionColor, out uint bounceCount);
void normalSpaceToWorldSpace(vec3 point, vec3 faceNormal, vec2 texCoord, vec3 inVector, out vec3 outVector);
void calculateWorldSpaceTangentBitagent(vec3 point, vec3 faceNormal, vec2 texCoord, out vec3 tangent, out vec3 bitangent);
bool intersect_triangle(vec3 orig, vec3 dir, vec3 vert0, vec3 vert1, vec3 vert2, out vec3 result);

// GLOBAL VARIABLES
const ivec2 pixelCoords = ivec2(gl_FragCoord.xy);
const uint pixelSeed = (pixelCoords.x * 1664525u + pixelCoords.y * 1013904223u) ^ floatBitsToUint(intersectionPoint.x) ^ floatBitsToUint(texCoord.y) ^ (u_seed * 2246822519u);

void main() {
    
    if (u_debugMouseLeftClick && u_debugMousePos == pixelCoords) highlightedMesh = u_currentMesh;

    if (drawDebug(u_debugMode)) return;

    if (u_emissive != 0.0f) return;

    vec4 sampleColor = vec4(0);
    calculateSample(sampleColor);

    if (u_currentSample == 0) {
        FragColor = sampleColor;
        imageStore(frameBuffer, pixelCoords, sampleColor);
        return;
    }

    vec4 accumulated = imageLoad(frameBuffer, pixelCoords);
    float weight = 1.0f / float(u_currentSample);

    vec4 outputColor = mix(accumulated, sampleColor, weight);
    FragColor = outputColor;

    imageStore(frameBuffer, pixelCoords, outputColor);

}

bool drawDebug(uint debugMode) {

    const uint DEBUG_DISABLED = 0;
    const uint DEBUG_ALBEDO = 1;
    const uint DEBUG_NORMAL = 2;
    const uint DEBUG_ROUGHNESS = 3;
    const uint DEBUG_METALLIC = 4;

    float brightness = 1.0f;
    if (u_debugLambertian) {

        const float lambertianMin = 0.5f;
        const float lambertianMax = 1.0f;

        brightness = abs( dot(normalize(u_camOrientation), normalize(geometricFaceNormal)) );
        brightness = brightness * (lambertianMax - lambertianMin) + lambertianMin;

    }

    switch ( debugMode ) {

        case DEBUG_ALBEDO:
            FragColor = vec4( texture(texturePool, vec3(texCoord, u_albedo)).rgb * brightness, 1.0f );
            imageStore(frameBuffer, pixelCoords, FragColor);
            return true;

        case DEBUG_NORMAL:
            FragColor = vec4( texture(texturePool, vec3(texCoord, u_normal)).rgb * brightness, 1.0f );
            imageStore(frameBuffer, pixelCoords, FragColor);
            return true;

        case DEBUG_ROUGHNESS:
            FragColor = vec4( texture(texturePool, vec3(texCoord, u_roughness)).rgb * brightness, 1.0f );
            imageStore(frameBuffer, pixelCoords, FragColor);
            return true;

        case DEBUG_METALLIC:
            FragColor = vec4( texture(texturePool, vec3(texCoord, u_metallic)).rgb * brightness, 1.0f );
            imageStore(frameBuffer, pixelCoords, FragColor);
            return true;

    }

    return false;

}

void calculateSample(out vec4 outputColor) {

    // Normal Map Local Vector
    vec3 normalMapVector;
    {
        vec3 temp = texture(texturePool, vec3(texCoord, u_normal)).xyz;
        normalMapVector = 2.0f * temp - 1.0f;
        normalMapVector = normalize(normalMapVector);
    }

    // Calculates face normal from normal map
    vec3 faceNormal;
    {
        float roughnessValue = texture(texturePool, vec3(texCoord, u_roughness)).r;
        uint localSeed = pixelSeed + uint(rayOrientation.x * rayOrientation.y * rayOrientation.z * 10000);    // generally random for each calculateSample call

        randomizeNormal(localSeed, roughnessValue, normalMapVector);

        normalSpaceToWorldSpace(intersectionPoint, normalize(geometricFaceNormal), texCoord, normalMapVector, faceNormal);
        faceNormal = normalize(faceNormal);

    }

    // Calculate reflection
    uint bounceCount;
    float brightness;
    vec3 accumulatedAlbedo;
    {
        calculatePath(intersectionPoint, u_camPos, faceNormal, brightness, accumulatedAlbedo, bounceCount);
    }
    
    // Combine all texture maps to single color
    vec3 albedoColor = texture(texturePool, vec3(texCoord, u_albedo)).rgb * color;
    vec3 tint = vertices[ indices[ uint(meshHeader[u_currentMesh].x) ] ].color.xyz;
    float roughnessValue = texture(texturePool, vec3(texCoord, u_roughness)).r;
    
    vec3 finalColor = (accumulatedAlbedo * albedoColor * tint * brightness);
    //float finalBrightness = (totalBrightness * (MAX_BRIGHTNESS - MIN_BRIGHTNESS)) + MIN_BRIGHTNESS;
    outputColor = vec4(finalColor, 1.0f);

}

void randomizeNormal(uint seed, float roughness, inout vec3 normal) {
    
    if (u_debugUniversalRoughness) roughness = u_debugUniversalRoughnessAmount;

    // clamp min and max
    roughness = (roughness * (MAX_ROUGHNESS - MIN_ROUGHNESS)) + MIN_ROUGHNESS;

    ivec2 dimensions = textureSize(colorNoise, 0);
    uint width = uint(dimensions.x);
    uint height = uint(dimensions.y);

    seed %= (width * height);
    uint y = seed / width;
    uint x = seed - (y * width);

    vec3 randomVector = texelFetch(colorNoise, ivec2(x, y), 0).rgb;
    randomVector = 2.0f * randomVector - 1.0f;
    randomVector = normalize(randomVector);

    normal = normalize( ((1 - roughness) * normal) + (roughness * randomVector) );

}

// Calculates render information for a single path (brightness, color, number of completed bounces)
void calculatePath(vec3 init_Intersection, vec3 init_Origin, vec3 init_FaceNormal, out float brightness, out vec3 accumulatedAlbedo, out uint bounceCount) {

    vec3 ref_intersection = init_Intersection;
    vec3 ref_origin = init_Origin;
    vec3 ref_faceNormal = init_FaceNormal;

    accumulatedAlbedo = vec3(1);

    brightness = 0;
    bounceCount = 0;
    while (bounceCount < u_maxBounces) 
    {

        // Calculates reflection
        float minFoundT = FAR_PLANE;        //
        vec3 ref_v0, ref_v1, ref_v2;        //  data on the reflection found
        vec2 ref_uv;                        //

        uint ref_Mesh;                      // 
        uint ref_Mesh_triangle;             //  data on the mesh that the ray reflected with
        uint ref_Mesh_indexStartPointer;    // 

        bool ref_Mesh_found = false;        //  whether we found a reflection or not

        vec3 incidentRay = normalize(ref_intersection - ref_origin);
        vec3 reflectionBounceDir = reflect(incidentRay, ref_faceNormal);

        {

            uint meshCount = meshHeader.length();
            for (uint i = 0u; i < meshCount; ++i) {

                uint trigCount = uint(meshHeader[i].y) / 3u;
                for (uint j = 0u; j < trigCount; ++j) {

                    uint indexStartPointer = uint(meshHeader[i].x);

                    vec3 v0 = vertices[indexStartPointer + (3*j) + 0].position.xyz;
                    vec3 v1 = vertices[indexStartPointer + (3*j) + 1].position.xyz;
                    vec3 v2 = vertices[indexStartPointer + (3*j) + 2].position.xyz;

                    vec3 result;
                    if (!intersect_triangle(ref_intersection, reflectionBounceDir, v0, v1, v2, result)) continue;

                    float t = result.x;
                    if (t < minFoundT) {    // mesh is closest mesh found so far
                    
                        // update all reflection mesh data
                        minFoundT = t;
                        ref_uv = vec2(result.yz);

                        ref_v0 = v0;
                        ref_v1 = v1;
                        ref_v2 = v2;

                        ref_Mesh = i;
                        ref_Mesh_triangle = j;
                        ref_Mesh_indexStartPointer = indexStartPointer;
                        ref_Mesh_found = true;

                    }
                }
            }
        }

        // If mesh is found, take reflection mesh data and calculate color
        if (ref_Mesh_found) {
            
            vec2 ref_t0 = vertices[ref_Mesh_indexStartPointer + (3 * ref_Mesh_triangle) + 0].texUV;
            vec2 ref_t1 = vertices[ref_Mesh_indexStartPointer + (3 * ref_Mesh_triangle) + 1].texUV;
            vec2 ref_t2 = vertices[ref_Mesh_indexStartPointer + (3 * ref_Mesh_triangle) + 2].texUV;
                    
            // grabs uv weights from moller-trumbore
            float u = ref_uv.x; 
            float v = ref_uv.y;
            float w = 1.0f - u - v;
            vec2 ref_uv = (w * ref_t0) + (u * ref_t1) + (v * ref_t2);   // applies weights

            vec3 albedoColor = (texture(texturePool, vec3(ref_uv, meshTextures[ref_Mesh].x)) ).rgb;
            vec3 tint = vertices[ indices[ uint(meshHeader[ref_Mesh].x) ] ].color.xyz;
            float roughnessValue = texture(texturePool, vec3(ref_uv, u_roughness)).r;

            accumulatedAlbedo *= albedoColor * tint;

            // calculate new values for next iteration
            ref_origin = ref_intersection;
            ref_intersection = ref_intersection + ( minFoundT * reflectionBounceDir );
            
            vec3 ref_GeometricFaceNormal;
            {
                vec3 normal_v0 = vertices[ref_Mesh_indexStartPointer + (3 * ref_Mesh_triangle) + 0].normal.xyz;
                vec3 normal_v1 = vertices[ref_Mesh_indexStartPointer + (3 * ref_Mesh_triangle) + 1].normal.xyz;
                vec3 normal_v2 = vertices[ref_Mesh_indexStartPointer + (3 * ref_Mesh_triangle) + 2].normal.xyz;
                ref_GeometricFaceNormal = (w * normal_v0) + (u * normal_v1) + (v * normal_v2);
            }

            vec3 ref_NormalMapVector;
            {
                uint ref_uv_normalMap = uint(meshTextures[ref_Mesh].y);
                vec4 temp = texture(texturePool, vec3(ref_uv, ref_uv_normalMap));
                ref_NormalMapVector.x = 2.0f * temp.x - 1.0f;
                ref_NormalMapVector.y = 2.0f * temp.y - 1.0f;
                ref_NormalMapVector.z = 2.0f * temp.z - 1.0f;
                ref_NormalMapVector = normalize(ref_NormalMapVector);
            }

            randomizeNormal(pixelSeed, roughnessValue, ref_NormalMapVector);

            normalSpaceToWorldSpace(ref_intersection, normalize(ref_GeometricFaceNormal), ref_uv, ref_NormalMapVector, ref_faceNormal);
            ref_faceNormal = normalize(ref_faceNormal);

            if (meshHeader[ref_Mesh].z != 0) {
                brightness = meshHeader[ref_Mesh].z / (bounceCount + 1);
            }

            ++bounceCount;

        } else {
            break;  // ray didn't hit anything, so break out of loop
        }
    }

    if (bounceCount == 0) accumulatedAlbedo = u_backgroundColor;

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

// Implementation of the Möller-Trumbore ray-triangle intersection algorithm
bool intersect_triangle(vec3 orig, vec3 dir, vec3 vert0, vec3 vert1, vec3 vert2, out vec3 result) {

    const bool DOUBLE_SIDED_REFLECTION = false;

    vec3 edge1 = vert1 - vert0;
    vec3 edge2 = vert2 - vert0;
    vec3 pvec = cross(dir, edge2);
    float det = dot(edge1, pvec);
    
    if (DOUBLE_SIDED_REFLECTION) {
        if (abs(det) < EPSILON) return false;
    } else {
        if (det < EPSILON) return false;
    }

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