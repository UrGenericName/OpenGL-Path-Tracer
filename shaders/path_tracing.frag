#version 330 core

out vec4 FragColor;

in vec3 color;
in vec2 texCoord;
in vec3 faceNormal;
in vec3 intersectionPoint;

uniform vec3 sun;

uniform sampler2D albedo;
uniform sampler2D normal;
uniform sampler2D roughness;
uniform sampler2D metallic;
uniform float emissive;

#define MAX_BRIGHTNESS 0.8
#define MIN_BRIGHTNESS 0.4

void normalSpaceToWorldSpace(vec3 point, vec3 faceNormal, vec2 texCoord, vec3 inVector, out vec3 outVector);
void calculateWorldSpaceTangentBitagent(vec3 point, vec3 faceNormal, vec2 texCoord, out vec3 tangent, out vec3 bitangent);
bool intersect_triangle(vec3 orig, vec3 dir, vec3 vert0, vec3 vert1, vec3 vert2, out vec3 result);

void main()
{

	if (emissive == 1.0f) {
		FragColor = vec4(color, 1.0f);
		return;
	}

	vec3 normalMapVector;
	vec4 temp = texture(normal, texCoord);
	normalMapVector.x = 2 * temp.x - 1;
	normalMapVector.y = 2 * temp.y - 1;
	normalMapVector.z = 2 * temp.z - 1;
	normalMapVector = normalize(normalMapVector);


	vec3 newFaceNormal;
	normalSpaceToWorldSpace(intersectionPoint, normalize(faceNormal), texCoord, normalMapVector, newFaceNormal);

	vec3 result;
	if (intersect_triangle(intersectionPoint, newFaceNormal, vec3(-3.0201, 4.90484, 2), vec3(-3.0201, 4.90484, 0), vec3(-2.81988, 6.89479, 0), result)) {
		FragColor = vec4(0.0f, 1.0f, 1.0f, 1.0f);
		return;
	}

	float tempDot = dot(normalize(newFaceNormal), normalize(sun));
	float brightness = tempDot * (MAX_BRIGHTNESS - MIN_BRIGHTNESS) + MIN_BRIGHTNESS;

	FragColor = texture(albedo, texCoord) * vec4(color, 1.0f) * brightness;
}

void normalSpaceToWorldSpace(vec3 point, vec3 faceNormal, vec2 texCoord, vec3 inVector, out vec3 outVector) {

	vec3 tangent;
	vec3 bitangent;
	calculateWorldSpaceTangentBitagent(point, faceNormal, texCoord, tangent, bitangent);

	mat3 tangentToWorldSpaceMatrix = mat3(tangent, bitangent, faceNormal);

	outVector = normalize(tangentToWorldSpaceMatrix * inVector);

}

// finds world space tangent and bitangent; used in the normalToWorldSpace() function to find the transformation matrix
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


// implementation of the Moller-Trumbore intersection algorithm (https://cadxfem.org/inf/Fast%20MinimumStorage%20RayTriangle%20Intersection.pdf)
bool intersect_triangle(vec3 orig, vec3 dir, vec3 vert0, vec3 vert1, vec3 vert2, out vec3 result) {

	const float EPSILON = 0.000001f;

	vec3 edge1 = vert1 - vert0;
	vec3 edge2 = vert2 - vert0;

	vec3 pvec = cross(dir, edge2);
	float det = dot(edge1, pvec);

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

	result[0] = t;
	result[1] = u;
	result[2] = v;

	return true;
}

/*

-------------------------
PSUEDO-CODE FOR BOUNCES
-------------------------


BOUNCE = 4;

bounceColors[BOUNCES] = { -1, -1, -1, -1 };
brightness = 0;
ray = normalVector;

loop BOUNCE times {

	intersectionPoint = NULL;
	intersectedMesh = NULL;
	faceVerts = [NULL, NULL, NULL];
	zBuffer = 999999;

	loop through meshCollection:

		loop through thruple of vertices in mesh:
		
			if (intersection(vertices, ray) && zBuffer > distance):
				intersectionPoint = point;
				intersectedMesh = currentMesh;
				zBuffer = distance;
				faceVerts = vertices;

	if (intersectionPoint != NULL):

		if (mesh.emissive == true):
			brightness = 1;
			break;

		ray = normalFromPoint(point, vertices);
}
		
*/