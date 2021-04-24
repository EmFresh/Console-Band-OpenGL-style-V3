#version 420

uniform vec4 LightPosition;

// colour
uniform vec3 LightAmbient  = vec3(1);
uniform vec3 LightDiffuse  = vec3(1);
uniform vec3 LightSpecular = vec3(1);

// scalar
uniform float LightSpecularExponent;
uniform float Attenuation_Constant;
uniform float Attenuation_Linear;
uniform float Attenuation_Quadratic;

// other
uniform bool LightEnable = false;
uniform bool AmbiantEnable = true;
uniform bool DiffuseEnable = true;
uniform bool SpecularEnable = true;
uniform int LightType;
uniform vec3 LightDirection;
uniform float LightAngleConstraint;

// uniform sampler2D uTex;
uniform sampler2D uPosOP;
uniform sampler2D uPosTrans;
uniform sampler2D uNormOP;
uniform sampler2D uNormTrans;
uniform sampler2D uScene;
uniform sampler2D uRamp;
uniform vec4 uViewPos;

uniform bool toonActiveDiff;
uniform bool toonActiveSpec;

in vec2 texcoord;

out vec4 outColor;


vec3 blinnPhong(vec3 lightDir, vec3 viewDir, vec3 pos, vec3 norm ) {
  //vec3 pos   = (texture(uPosOP,  texcoord)).rgb;//frag position  
  //vec3 norm    = (texture(uNormOP, texcoord)).rgb;
  vec3 halfDir = normalize(lightDir + viewDir);
  vec3 diffuse, specular;

  //Blinn-Phong Lighting here:
  
  //Diffuse   
  diffuse = max(dot(norm, lightDir), 0.0)  * LightDiffuse * int(DiffuseEnable);
  diffuse *= ( toonActiveDiff ? texture(uRamp,vec2(max(dot(norm, lightDir),0.0))).rgb : vec3(1)) ;

  //Specular  
  specular = pow(max(dot(norm, halfDir), 0.0), LightSpecularExponent) * LightSpecular * int(SpecularEnable);
  specular *=  (toonActiveSpec ? texture(uRamp,vec2(pow(max(dot(norm, halfDir), 0.0), LightSpecularExponent))).rgb : vec3(1)) ;
 
  return (diffuse + specular) * int(LightEnable);
}

vec3 calculateDirectionalLight() {
 //variables  
  vec3 posOP          = (texture(uPosOP    , texcoord)).rgb;//frag position
  vec3 posTrans       = (texture(uPosTrans , texcoord)).rgb;//frag position
  vec3 normOP         = (texture(uNormOP   , texcoord)).rgb;
  vec3 normTrans      = (texture(uNormTrans, texcoord)).rgb;
  vec3 viewDirOP      = uViewPos.xyz - posOP;   
  vec3 viewDirTrans   = uViewPos.xyz - posTrans; 
  
 return 
 blinnPhong(LightDirection, LightDirection,    posOP,    normOP) +
 blinnPhong(LightDirection, LightDirection, posTrans, normTrans);
}

void directionalLight() { 
  outColor.rgb += calculateDirectionalLight();
}


void main() {

  vec3 colour = vec3(0);

  // Ambient Light
  float ambientStrength = 1.0;
  outColor = vec4(colour * LightAmbient * ambientStrength * int(AmbiantEnable), 1);
 
  
  //do light calculations
  directionalLight();
}