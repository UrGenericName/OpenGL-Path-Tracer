#version 430 core

layout(early_fragment_tests) in;

struct BoundingBox {
    vec4 minBounds;
    vec4 maxBounds;
};

struct Vertex {
    vec4 position;
    vec4 color;
    vec4 normal;
    vec2 texUV;
    vec2 _pad;
};

struct MeshTextures {
	uint albedo;
	uint normal;
	uint roughness;
	uint metallic;
};

struct MeshHeader {
	uint indicesStartPointer;
	uint indicesSize;
	float emissiveValue;
};


// LAYOUTS
layout(std430, binding = 0) readonly buffer VertexBuffer {
    Vertex vertices[];
};

layout(std430, binding = 1) readonly buffer IndexBuffer {
    uint indices[];
};

layout(std430, binding = 2) readonly buffer MeshTextureBuffer {
    MeshTextures meshTextures[]; 
};

layout(std430, binding = 3) readonly buffer MeshHeaderBuffer {
    MeshHeader meshHeader[];
};

layout(std430, binding = 4) coherent buffer highlightedMeshBuffer {
    uint highlightedMesh;
};

layout(std430, binding = 5) readonly buffer boundingBoxesBuffer {
    BoundingBox boundingBoxes[];
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
uniform uint u_maxSamples;
uniform uint u_currentSample;

uniform uint u_albedo;
uniform uint u_normal;
uniform uint u_roughness;
uniform uint u_metallic;
uniform vec3 u_tint;
uniform float u_emissive;
uniform uint u_currentMesh;

uniform uint u_seed;
uniform vec3 u_backgroundColor;

uniform vec3 u_camPos;
uniform vec3 u_camOrientation;     

#define MAX_BRIGHTNESS 1.0f
#define MIN_BRIGHTNESS 0.0f

#define EPSILON 0.00001
#define FAR_PLANE 999999

struct Barycentric {
    float w;
    float u;
    float v;
};

struct HitInfo {

    int mesh;
    int trig;

    vec3 v0;
    vec3 v1;
    vec3 v2;

    float t;
    Barycentric weights;

    vec3 albedo;
    vec3 normal;
    float roughness;
    float metallic;
    float emissive;

    vec3 intersectionPoint;
    vec3 faceNormal;

    vec2 texCoord;

    vec3 reflectionDir;
};

// DECLERATIONS
bool drawDebug(uint debugMode);
void calculate_sample(out vec4 outputColor);
bool intersect_scene(vec3 origin, vec3 dir, out HitInfo hitInfo);
void caluclate_normal_map_vector(in vec3 normalMap, out vec3 normalMapVector);
void calculate_geometric_face_normal(in int mesh, in int trig, Barycentric weights, out vec3 result);
void randomize_normal(in uint seed, in float roughness, inout vec3 normal);
void normal_space_to_world_space(in vec3 normalMap, in vec3 geometricFaceNormal, out vec3 faceNormal);
void calculate_world_space_tangent_bitangent(vec3 faceNormal, out vec3 tangent, out vec3 bitangent);
bool intersect_bounding_box(vec3 orig, vec3 dir, BoundingBox box);
bool intersect_triangle(vec3 orig, vec3 dir, vec3 vert0, vec3 vert1, vec3 vert2, out Barycentric weights, out float t);

// GLOBAL VARIABLES
const ivec2 pixelCoords = ivec2(gl_FragCoord.xy);
const uint pixelSeed = (pixelCoords.x * 1664525u + pixelCoords.y * 1013904223u) ^ floatBitsToUint(intersectionPoint.x) ^ floatBitsToUint(texCoord.y) ^ (u_seed * 2246822519u) - u_currentSample;
const float normalMapScalingAdjustment = 255.0f / 254.0f;

void main() {

    if (u_debugMouseLeftClick && u_debugMousePos == pixelCoords) highlightedMesh = u_currentMesh;

    if (u_currentSample == u_maxSamples) return;

    if (drawDebug(u_debugMode)) return;

    if (u_emissive != 0.0f) return;

    vec4 sampleColor = vec4(0);
    calculate_sample(sampleColor);

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
    const uint DEBUG_VERTEX_NORMAL = 5;

    float brightness = 1.0f;
    if (u_debugLambertian) {

        const float lambertianMin = 0.5f;
        const float lambertianMax = 1.0f;

        brightness = abs( dot(normalize(u_camOrientation), normalize(geometricFaceNormal)) );
        brightness = brightness * (lambertianMax - lambertianMin) + lambertianMin;

    }

    switch ( debugMode ) {

        case DEBUG_ALBEDO:
            FragColor = vec4( texture(texturePool, vec3(texCoord, u_albedo)).rgb * brightness * u_tint, 1.0f );
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

        case DEBUG_VERTEX_NORMAL:
            FragColor = vec4( geometricFaceNormal, 1.0f );
            imageStore(frameBuffer, pixelCoords, FragColor);
            return true;

    }

    return false;

}

void calculate_sample(out vec4 outputColor) {

    HitInfo hitInfo;
    hitInfo.intersectionPoint = intersectionPoint;
    
    // MAP FETCHING
    vec3 albedoMap = textureLod(texturePool, vec3(texCoord.x, texCoord.y, u_albedo), 0.0f).rgb * vertices[indices[ uint(meshHeader[u_currentMesh].indicesStartPointer)]].color.rgb;
    vec3 normalMap = textureLod(texturePool, vec3(texCoord.x, texCoord.y, u_normal), 0.0f).rgb * normalMapScalingAdjustment;
    float roughnessMap = textureLod(texturePool, vec3(texCoord.x, texCoord.y, u_roughness), 0.0f).r;
    float metallicMap = textureLod(texturePool, vec3(texCoord.x, texCoord.y, u_metallic), 0.0f).r;

    // FACE NORMAL CALCULATIONS
    vec3 normalMapVector;
    caluclate_normal_map_vector(normalMap, normalMapVector);

    randomize_normal(pixelSeed, roughnessMap, normalMapVector);

    normal_space_to_world_space(normalize(geometricFaceNormal), normalMapVector, hitInfo.faceNormal);

    // REFLECTION DIRECTION
    hitInfo.reflectionDir = normalize(reflect((intersectionPoint - u_camPos), hitInfo.faceNormal));
    float cosineFactor = max(0.0f, dot(hitInfo.faceNormal, hitInfo.reflectionDir));

    outputColor = vec4(albedoMap, 1.0f);

    float totalBrightness = 0.0f;

    for (int i = 0; i < u_maxBounces; ++i) {
        
        vec3 intersectionPoint = hitInfo.intersectionPoint;
        vec3 reflectionDir = hitInfo.reflectionDir;
        if (intersect_scene(intersectionPoint, reflectionDir, hitInfo)) {

            cosineFactor *= max(0.0f, dot(hitInfo.faceNormal, hitInfo.reflectionDir));
            outputColor.xyz *= hitInfo.albedo;

            if (hitInfo.emissive != 0.0f) {
                totalBrightness = hitInfo.emissive;
                break;
            }

        } else {
            outputColor.xyz *= vec3(0.07f, 0.13f, 0.17f);
            break;
        }
    }

    outputColor.xyz *= totalBrightness * cosineFactor;

}

bool intersect_scene(vec3 origin, vec3 dir, out HitInfo hitInfo) {
    
    bool hit = false;

    float depth = FAR_PLANE;
    for (int i_mesh = 0; i_mesh < meshHeader.length(); ++i_mesh) {
        
        if (!intersect_bounding_box(origin, dir, boundingBoxes[i_mesh])) continue;

        uint i_mesh_indicesStartPointer = uint(meshHeader[i_mesh].indicesStartPointer);

        for (int j_trig = 0; j_trig < (meshHeader[i_mesh].indicesSize / 3); ++j_trig) {
            
            vec3 v0 = vertices[indices[ i_mesh_indicesStartPointer + (j_trig * 3) + 0 ]].position.xyz;
            vec3 v1 = vertices[indices[ i_mesh_indicesStartPointer + (j_trig * 3) + 1 ]].position.xyz;
            vec3 v2 = vertices[indices[ i_mesh_indicesStartPointer + (j_trig * 3) + 2 ]].position.xyz;

            float t;
            Barycentric weights;
            if (intersect_triangle(origin, dir, v0, v1, v2, weights, t)) {
                
                if (depth <= t) continue; 

                depth = t;
                hit = true;

                // CACHCE USEFUL DATA
                hitInfo.v0 = v0;
                hitInfo.v1 = v1;
                hitInfo.v2 = v2;

                hitInfo.weights = weights;
                hitInfo.mesh = i_mesh;
                hitInfo.trig = j_trig;
                hitInfo.t = t;

            }
        }
    }

    if (hit) {
        
        hitInfo.intersectionPoint = (hitInfo.v0 * hitInfo.weights.w) + (hitInfo.v1 * hitInfo.weights.u) + (hitInfo.v2 * hitInfo.weights.v);
        
        vec2 t0 = vertices[indices[ uint(meshHeader[hitInfo.mesh].indicesStartPointer) + (hitInfo.trig * 3) + 0 ]].texUV;
        vec2 t1 = vertices[indices[ uint(meshHeader[hitInfo.mesh].indicesStartPointer) + (hitInfo.trig * 3) + 1 ]].texUV;
        vec2 t2 = vertices[indices[ uint(meshHeader[hitInfo.mesh].indicesStartPointer) + (hitInfo.trig * 3) + 2 ]].texUV;

        hitInfo.texCoord = (hitInfo.weights.w * t0) + (hitInfo.weights.u * t1) + (hitInfo.weights.v * t2);

        // CACHE TEXTURES
        hitInfo.albedo = textureLod(texturePool, vec3(hitInfo.texCoord.x, hitInfo.texCoord.y, meshTextures[hitInfo.mesh].albedo), 0.0f).rgb * vertices[indices[ uint(meshHeader[hitInfo.mesh].indicesStartPointer) + (hitInfo.trig * 3) + 0 ]].color.rgb;
        hitInfo.normal = textureLod(texturePool, vec3(hitInfo.texCoord.x, hitInfo.texCoord.y, meshTextures[hitInfo.mesh].normal), 0.0f).rgb * normalMapScalingAdjustment;
        hitInfo.roughness = textureLod(texturePool, vec3(hitInfo.texCoord.x, hitInfo.texCoord.y, meshTextures[hitInfo.mesh].roughness), 0.0f).r;
        hitInfo.metallic = textureLod(texturePool, vec3(hitInfo.texCoord.x, hitInfo.texCoord.y, meshTextures[hitInfo.mesh].metallic), 0.0f).r;
        hitInfo.emissive = meshHeader[hitInfo.mesh].emissiveValue;

        // FACE NORMAL CALCULATIONS
        vec3 geometricFaceNormal;
        calculate_geometric_face_normal(hitInfo.mesh, hitInfo.trig, hitInfo.weights, geometricFaceNormal);

        vec3 normalMapVector;
        caluclate_normal_map_vector(hitInfo.normal, normalMapVector);

        randomize_normal(pixelSeed + hitInfo.mesh, hitInfo.roughness, normalMapVector);

        normal_space_to_world_space(normalize(geometricFaceNormal), normalMapVector, hitInfo.faceNormal);

        // REFLECTION DIRECTION
        hitInfo.reflectionDir = normalize(reflect(dir, hitInfo.faceNormal));

        return true;

    } else {
        
        return false;

    }

}

void randomize_normal(in uint seed, in float roughness, inout vec3 normal) {
    
    if (u_debugUniversalRoughness) roughness = u_debugUniversalRoughnessAmount;

    roughness = min(roughness, 1.0f);

    ivec2 dimensions = textureSize(colorNoise, 0);
    uint width = uint(dimensions.x);
    uint height = uint(dimensions.y);
    
    seed = seed % (width * height);
    int x = int(seed % width);
    int y = int(seed / width);
    
    vec3 randomVector = texelFetch(colorNoise, ivec2(x, y), 0).rgb * normalMapScalingAdjustment;
    randomVector = randomVector * 2.0 - 1.0; 
    randomVector = normalize(randomVector);

    if (dot(randomVector, normal) < 0.0) {
        randomVector = -randomVector;
    }

    normal = normalize(mix(normal, randomVector, roughness));
}

void caluclate_normal_map_vector(in vec3 normalMap, out vec3 normalMapVector) {

    normalMapVector.x = 2.0f * normalMap.r - 1.0f;
    normalMapVector.y = 2.0f * normalMap.g - 1.0f;
    normalMapVector.z = 0.5f * normalMap.b;
    normalMapVector = normalize(normalMapVector);

}

void calculate_geometric_face_normal(in int mesh, in int trig, in Barycentric weights, out vec3 result) {
    
    uint mesh_indicesStartPointer = uint(meshHeader[mesh].indicesStartPointer);

    vec3 n0 = vertices[indices[mesh_indicesStartPointer + (trig * 3) + 0]].normal.xyz;
    vec3 n1 = vertices[indices[mesh_indicesStartPointer + (trig * 3) + 1]].normal.xyz;
    vec3 n2 = vertices[indices[mesh_indicesStartPointer + (trig * 3) + 2]].normal.xyz;
    result = normalize((n0 * weights.w) + (n1 * weights.u) + (n2 * weights.v));

}

// Transforms normal map tangents into world space vectors
void normal_space_to_world_space(in vec3 normalMap, in vec3 geometricFaceNormal, out vec3 faceNormal) {

    vec3 tangent;
    vec3 bitangent;

    calculate_world_space_tangent_bitangent(normalMap, tangent, bitangent);

    mat3 tangentToWorldSpaceMatrix = mat3(tangent, bitangent, normalMap);

    faceNormal = normalize(tangentToWorldSpaceMatrix * geometricFaceNormal);

}

void calculate_world_space_tangent_bitangent(vec3 faceNormal, out vec3 tangent, out vec3 bitangent) {

    vec3 helperAxis = (abs(faceNormal.x) > 0.9) ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
    
    tangent = normalize(cross(faceNormal, helperAxis));
    bitangent = normalize(cross(faceNormal, tangent));

}

bool intersect_bounding_box(vec3 orig, vec3 dir, BoundingBox box)
{
    vec3 invD = 1.0 / dir;

    vec3 t1 = (box.minBounds.xyz - orig) * invD;
    vec3 t2 = (box.maxBounds.xyz - orig) * invD;

    vec3 tMinAxes = min(t1, t2);
    vec3 tMaxAxes = max(t1, t2);

    float tmin = max(tMinAxes.x, max(tMinAxes.y, tMinAxes.z));
    float tmax = min(tMaxAxes.x, min(tMaxAxes.y, tMaxAxes.z));

    return (tmin <= tmax) && (tmax >= 0.001);
}

// Implementation of the Möller-Trumbore ray-triangle intersection algorithm
bool intersect_triangle(vec3 orig, vec3 dir, vec3 vert0, vec3 vert1, vec3 vert2, out Barycentric weights, out float outT) {

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

    outT = t;
    weights.u = u;
    weights.v = v;
    weights.w = 1.0f - u - v;

    return true;
}