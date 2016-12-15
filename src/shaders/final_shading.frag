#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define PIXELS_PER_TILE 16


#define MAX_NUM_LIGHTS_PER_TILE 128

struct Light {
	vec4 beginPos; // beginPos.w is intensity
	vec4 endPos; // endPos.w is radius
	vec4 color; // color.w is t
};

struct Frustum {
    vec4 planes[4];
};

struct Material {
    vec4 ambient; // Ka
    vec4 diffuse; // Kd
    //vec4 specular; // Ks

    float specularPower;
    int useTextureMap;
    int useNormMap;
    int useSpecMap;
    //float pad;
};


layout(binding = 1) uniform sampler2D depthTexSampler;

layout(binding = 10) uniform Mat {
    Material material;
} ubo_mat;

layout(binding = 11) uniform sampler2D texColorSampler;
layout(binding = 12) uniform sampler2D texNormalSampler;
layout(binding = 13) uniform sampler2D texSpecularSampler;

layout(binding = 3) buffer Lights {
    Light lights[];
};

layout(binding = 5) buffer Frustums {
    Frustum frustums[];
};

layout(binding = 6) uniform Params {
    int numLights;
    float time;
    int debugMode;
    // int padding;
    ivec2 numThreads;
    ivec2 screenDimensions;
} params;

layout(binding = 7) buffer LightIndex {
	int lightIndices[];
};

layout(binding = 8) buffer LightGrid {
	int lightGrid[];
};


layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPosWorldSpace;
layout(location = 4) in vec3 fragPosViewSpace;
layout(location = 5) in vec3 cameraPosWorldSpace;
layout(location = 6) in mat3 TBN;

layout(location = 0) out vec4 outColor;

// apply normal map
vec3 applyNormalMap(mat3 TBN, vec3 normap) {
    normap = normap * 2.0 - 1.0;
    vec3 mappedNor = TBN * normap;
    return normalize(mappedNor);
}

void main() {

    vec2 pixelCoord = vec2(gl_FragCoord.x, gl_FragCoord.y ) / params.screenDimensions;

	ivec2 tileID = ivec2(gl_FragCoord.x, params.screenDimensions.y - gl_FragCoord.y ) / PIXELS_PER_TILE;
	int tileIndex = tileID.y * params.numThreads.x + tileID.x;

    vec3 finalColor = vec3(0,0,0);

    // read material properties
    float specularPower = ubo_mat.material.specularPower;

    vec3 diffuseColor = ubo_mat.material.diffuse.xyz;
    if(ubo_mat.material.useTextureMap > 0)
        diffuseColor = texture(texColorSampler, fragTexCoord).xyz;

    vec3 normal = fragNormal;
    vec3 normalMap = vec3(0,0,0);
    if(ubo_mat.material.useNormMap > 0) {
        normalMap = texture(texNormalSampler, fragTexCoord).xyz;
        normal = applyNormalMap(TBN, normalMap);
    }

    vec3 specularColor = vec3(0,0,0);
    if(ubo_mat.material.useSpecMap > 0)
        specularColor = texture(texSpecularSampler, fragTexCoord).xyz;

    vec3 ambientColor = ubo_mat.material.ambient.xyz * diffuseColor;

    vec3 viewDir = normalize(cameraPosWorldSpace.xyz - fragPosWorldSpace);

    uint lightIndexBegin = tileIndex * MAX_NUM_LIGHTS_PER_TILE;
    uint lightNum = lightGrid[tileIndex];
    // lightGrid[TileIndex] = lights need to be considered in tile
    for(int i = 0; i < lightNum; ++i) {
        int lightIndex = lightIndices[i + lightIndexBegin];

        Light currentLight = lights[lightIndex];

		vec3 beginPos = currentLight.beginPos.xyz;
		vec3 endPos = currentLight.endPos.xyz;
		float t = sin(params.time * lightIndex * .001f);
        
        vec3 lightPos = (1 - t) * beginPos + t * endPos;
        vec3 lightColor = currentLight.color.xyz;
        vec3 lightDir = lightPos - fragPosWorldSpace;
        float lightIntensity = currentLight.beginPos.w;
        float lightRadius = currentLight.endPos.w;

        float dist = length(lightDir);
        lightDir = lightDir/dist;

        float NdotL = max(0, dot(normal, lightDir));

        vec3 halfDir = normalize(lightDir + viewDir);
        float specAngle = max(dot(halfDir, normal), 0.0);
        float specular = pow(specAngle, specularPower);

        vec3 irr = (NdotL * diffuseColor + specular * specularColor) * lightColor * lightIntensity;
        //vec3 irr = (specular * specularColor) * lightColor * lightIntensity;

        float att = max(0.0, lightRadius - dist);
        finalColor += att * irr;

    }
    finalColor += ambientColor * 0.5;
    //outColor = vec4(finalColor, 1.0);

    switch(params.debugMode){
        case 0: // basic lighting
            outColor = vec4(finalColor, 1.0);
            break;

        case 1: // texture map
            outColor = vec4(diffuseColor, 1.0);
            break;

        case 2: // original normal
            outColor = vec4(fragNormal, 1.0);
            break;

        case 3: // normal map
            outColor = vec4(normalMap, 1.0);
            break;

        case 4: // mapped normal
            outColor = vec4(normal, 1.0);
            break;

        case 5: // specular map
            outColor = vec4(specularColor, 1.0);
            break;

		case 6: // light heat map
            float tmp = lightGrid[tileIndex];
            if(tmp <= 30.f)
            {
                outColor = vec4( 0.f, 0.f, tmp / 30.f, 1.f );
            }
            else if(tmp <= 60.f) {
                outColor = vec4( 0.f, (tmp - 30.f) / 30.0f, 1.f, 1.f );
            }
            else {
                outColor = vec4( (tmp - 60.f) / 30.f, 1.f, 1.f, 1.f );
            }

            // outColor *= vec4(diffuseColor, 1.0);
			break;

        case 7:
            vec3 color = finalColor * finalColor;
            color = sqrt(pow(color, vec3(1.0 / 2.0)));
            outColor = vec4(color, 1.0);
            break;

        case 8:
            outColor = vec4(texture(depthTexSampler, pixelCoord).rgb, 1.0);
            break;

        default:
            outColor = vec4(finalColor, 1.0);
            break;

    }
}
