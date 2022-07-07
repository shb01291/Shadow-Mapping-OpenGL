#version 150 core

uniform vec3 cameraPos;
uniform vec3 lightColor=vec3(1);
uniform vec3 ambientLight=vec3(0.1);
uniform vec3 lightPos=vec3(3, 3, 3);
uniform vec4 diffuseMaterial=vec4(1, 0.4, 0, 1);
uniform vec4 specularMaterial=vec4(1);

uniform sampler2D diffTex;
uniform sampler2D bumpTex;
uniform sampler2D shadowMap;

in vec3 normal;
in vec3 worldPos;
in vec2 texCoord;

in vec4 shadowCoord;

out vec4 out_Color;

const vec2 poissonDisk[64] = vec2[](

vec2(-0.613392,  0.617481),vec2( 0.170019, -0.040254),vec2(-0.299417,  0.791925),vec2( 0.645680,  0.493210),

vec2(-0.651784,  0.717887),vec2( 0.421003,  0.027070),vec2(-0.817194, -0.271096),vec2(-0.705374, -0.668203),

vec2( 0.977050, -0.108615),vec2( 0.063326,  0.142369),vec2( 0.203528,  0.214331),vec2(-0.667531,  0.326090),

vec2(-0.098422, -0.295755),vec2(-0.885922,  0.215369),vec2( 0.566637,  0.605213),vec2( 0.039766, -0.396100),

vec2( 0.751946,  0.453352),vec2( 0.078707, -0.715323),vec2(-0.075838, -0.529344),vec2( 0.724479, -0.580798),

vec2( 0.222999, -0.215125),vec2(-0.467574, -0.405438),vec2(-0.248268, -0.814753),vec2( 0.354411, -0.887570),

vec2( 0.175817,  0.382366),vec2( 0.487472, -0.063082),vec2(-0.084078,  0.898312),vec2( 0.488876, -0.783441),

vec2( 0.470016,  0.217933),vec2(-0.696890, -0.549791),vec2(-0.149693,  0.605762),vec2( 0.034211,  0.979980),

vec2( 0.503098, -0.308878),vec2(-0.016205, -0.872921),vec2( 0.385784, -0.393902),vec2(-0.146886, -0.859249),

vec2( 0.643361,  0.164098),vec2( 0.634388, -0.049471),vec2(-0.688894,  0.007843),vec2( 0.464034, -0.188818),

vec2(-0.440840,  0.137486),vec2( 0.364483,  0.511704),vec2( 0.034028,  0.325968),vec2( 0.099094, -0.308023),

vec2( 0.693960, -0.366253),vec2( 0.678884, -0.204688),vec2( 0.001801,  0.780328),vec2( 0.145177, -0.898984),

vec2( 0.062655, -0.611866),vec2( 0.315226, -0.604297),vec2(-0.780145,  0.486251),vec2(-0.371868,  0.882138),

vec2( 0.200476,  0.494430),vec2(-0.494552, -0.711051),vec2( 0.612476,  0.705252),vec2(-0.578845, -0.768792),

vec2(-0.772454, -0.090976),vec2( 0.504440,  0.372295),vec2( 0.155736,  0.065157),vec2( 0.391522,  0.849605),

vec2(-0.620106, -0.328104),vec2( 0.789239, -0.419965),vec2(-0.545396,  0.538133),vec2(-0.178564, -0.596057));

mat3 getTBN( vec3 N ) {
    vec3 Q1 = dFdx(worldPos), Q2 = dFdy(worldPos);
    vec2 st1 = dFdx(texCoord), st2 = dFdy(texCoord);
    float D = st1.s*st2.t-st2.s*st1.t;
    return mat3(normalize(( Q1*st2.t - Q2*st1.t )*D),
                       normalize((-Q1*st2.s + Q2*st1.s )*D), N);
}

#define TEX_DELTA 0.0001
void main(void)
{

vec3 N=normalize(normal);
vec3 L=normalize(lightPos-worldPos);

mat3 TBN = getTBN( N );
float Bu = texture( bumpTex, texCoord+vec2(TEX_DELTA,0) ).r
         - texture( bumpTex, texCoord-vec2(TEX_DELTA,0) ).r;
float Bv = texture( bumpTex, texCoord+vec2(0,TEX_DELTA) ).r
         - texture( bumpTex, texCoord-vec2(0,TEX_DELTA) ).r;
vec3 bumpVec = vec3(-Bu*15., -Bv*15., 1 );
N = normalize( TBN* bumpVec );

vec3 V=normalize(cameraPos-worldPos);
vec3 R=N*dot(N, L)*2.-L;

vec4 diffuseTexMaterial = texture(diffTex, texCoord);
diffuseTexMaterial.rgb=pow(diffuseTexMaterial.rgb, vec3(2.2));

vec3 color=vec3(0);

float visibility=1.0;
vec3 sCoord=shadowCoord.xyz/shadowCoord.w;

float bias = 0.005 * tan(acos(dot(N,L)));

for (int i=0;i<64;i++){
    if ( texture( shadowMap, sCoord.xy
        + poissonDisk[i]/300.0 ).r < sCoord.z-bias )
        visibility-=1./64.;}

  
vec3 lColor=lightColor*visibility;


float diffuseFactor=clamp(dot(N, L), 0, 1);
float specularFactor=pow(clamp(dot(R, V), 0, 1), 10);
color += diffuseTexMaterial.rgb*diffuseFactor*lColor;
color += specularMaterial.rgb * specularFactor * lColor;
//color += diffuseTexMaterial.rgb * ambientLight;

out_Color = vec4(pow( color,vec3(1/2.2)),  diffuseMaterial.a);
}
