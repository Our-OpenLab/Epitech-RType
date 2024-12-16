#include <stdexcept>

#include "client/core/renderer.hpp"

auto vertex_shader_source = R"(
#version 330 core

layout (location = 0) in vec2 position;

uniform mat4 projection;

void main() {
    gl_Position = projection * vec4(position, 0.0, 1.0);
}

)";

auto fragment_shader_source = R"(
#version 330 core
out vec4 FragColor;
uniform vec3 color;
void main() {
    FragColor = vec4(color, 1.0);
})";

auto neon_bar_horizontal_shader = R"(
#version 330 core

out vec4 FragColor;

uniform float time;

// Ajout des uniformes pour la position et la taille
uniform vec2 rect_position; // Position du rectangle
uniform vec2 rect_size;     // Taille du rectangle

const float zoom = 3.0; // Facteur de zoom pré-défini

void main(void) {
    // Position du pixel en cours
    vec2 pixel_pos = gl_FragCoord.xy;

    // Vérifier si le pixel est dans le rectangle
    if (pixel_pos.x < rect_position.x || pixel_pos.x > rect_position.x + rect_size.x ||
        pixel_pos.y < rect_position.y || pixel_pos.y > rect_position.y + rect_size.y) {
        // En dehors du rectangle : ne rien dessiner (transparent/noir)
        discard;
    }

    // Normaliser la position dans le rectangle
    vec2 uPos = (pixel_pos - rect_position) / rect_size;

    // Appliquer le zoom
    uPos /= zoom;

    uPos.x += (1.0 - 1.0 / zoom) * 0.5;
    uPos.y += (1.0 - 1.0 / zoom) * 0.5;

    // Centrer l'origine (0, 0) au centre du rectangle
    uPos.x -= 0.5;
    uPos.y -= 0.5;

    vec3 color = vec3(0.0); // Couleur initiale
    float horizColor = 0.0;

    // Appliquer le décalage sinusoïdal uniquement sur l'axe vertical
    float t = time * 0.5; // Ajustez la vitesse du temps
    float wave = sin(uPos.x * 10.0 + t) * 0.005; // Onde uniquement sur l'axe vertical
    uPos.y += wave;

    // Calculer une intensité en fonction de la position (dépend de uPos.y)
    float fTemp = pow(abs(1.0 / (uPos.y * 150.0)), 1.5);

    horizColor += fTemp;

    // Ajouter des composantes de couleur
    color += vec3(
        fTemp * 0.8,  // Rouge
        fTemp * 0.2,  // Vert
        pow(fTemp, 0.99) * 1.5 // Bleu
    );

    // Appliquer la couleur finale
    FragColor = vec4(color, 1.0);
}
)";

auto neon_bar_vertical_shader = R"(
#version 330 core

out vec4 FragColor;

uniform float time;

// Ajout des uniformes pour la position et la taille
uniform vec2 rect_position; // Position du rectangle
uniform vec2 rect_size;     // Taille du rectangle

const float zoom = 3.0; // Facteur de zoom pré-défini

void main(void) {
    // Position du pixel en cours
    vec2 pixel_pos = gl_FragCoord.xy;

    // Vérifier si le pixel est dans le rectangle
    if (pixel_pos.x < rect_position.x || pixel_pos.x > rect_position.x + rect_size.x ||
        pixel_pos.y < rect_position.y || pixel_pos.y > rect_position.y + rect_size.y) {
        // En dehors du rectangle : ne rien dessiner (transparent/noir)
        discard;
    }

    // Normaliser la position dans le rectangle
    vec2 uPos = (pixel_pos - rect_position) / rect_size;

    // Appliquer le zoom
    uPos /= zoom;

    uPos.x += (1.0 - 1.0 / zoom) * 0.5;
    uPos.y += (1.0 - 1.0 / zoom) * 0.5;


    // Centrer l'origine (0, 0) au centre du rectangle
    uPos.x -= 0.5;
    uPos.y -= 0.5;

    vec3 color = vec3(0.0); // Couleur initiale
    float horizColor = 0.0;

    // Appliquer le décalage sinusoïdal uniquement sur l'axe horizontal
    float t = time * 0.5; // Ajustez la vitesse du temps
    float wave = sin(uPos.y * 10.0 + t) * 0.005; // Onde uniquement sur l'axe horizontal
    uPos.x += wave;

    // Calculer une intensité en fonction de la position
    //  float fTemp = abs(1.0 / (uPos.x * 100.0));

    float fTemp = pow(abs(1.0 / (uPos.x * 150.0)), 1.5);

    horizColor += fTemp;

    // Ajouter des composantes de couleur
    color += vec3(
        fTemp * 0.8,  // Rouge
        fTemp * 0.2,  // Vert
        pow(fTemp, 0.99) * 1.5 // Bleu
    );

    // Appliquer la couleur finale
    FragColor = vec4(color, 1.0);
}

)";

auto projectile_shader_source = R"(
#version 330 core

out vec4 FragColor;

uniform float time;
uniform vec2 resolution;
uniform vec2 rect_position; // Position du projectile

float makePoint(float x, float y, float s)
{
    float distance = sqrt(x * x + y * y);
    return (s / 3.0) / ((0.007 / s) + distance);
}

vec3 grad(float f)
{
    // Gradient de couleur simplifié
    vec4 c01 = vec4(0.0, 0.0, 0.0, 0.00);
    vec4 c02 = vec4(0.5, 0.0, 0.0, 0.50);
    vec4 c03 = vec4(1.0, 0.0, 0.0, 0.55);
    vec4 c04 = vec4(1.0, 1.0, 0.0, 0.80);
    vec4 c05 = vec4(1.0, 1.0, 1.0, 1.00);

    return (f < c02.w) ? mix(c01.xyz, c02.xyz, f / c02.w)
         : (f < c03.w) ? mix(c02.xyz, c03.xyz, (f - c02.w) / (c03.w - c02.w))
         : (f < c04.w) ? mix(c03.xyz, c04.xyz, (f - c03.w) / (c04.w - c03.w))
         : mix(c04.xyz, c05.xyz, (f - c04.w) / (c05.w - c04.w));
}

void main(void)
{
    vec2 pixel_pos = gl_FragCoord.xy;

    if (pixel_pos.x < rect_position.x || pixel_pos.x > rect_position.x + resolution.x ||
        pixel_pos.y < rect_position.y || pixel_pos.y > rect_position.y + resolution.y)
    {
        discard;
    }

    // Position relative au rectangle
//    vec2 p = (gl_FragCoord.xy - rect_position - resolution.xy / 2.0) / resolution.y;

    vec2 p = (pixel_pos - rect_position) / resolution;

    // Étendre la position à [-1, 1] pour centrer le rendu
    p = p * 2.0 - 1.0;

    float x = p.x;
    float y = p.y;

    float a = makePoint(x, y, 55.0);
    vec3 a1 = grad(a / 183.0);

    // Couleur calculée
    vec3 col = vec3(a1.x, a1.y, a1.z);

    // Atténuation basée sur la distance au centre
    vec2 center = rect_position + resolution / 2.0; // Centre du projectile
    float distance_to_center = length((gl_FragCoord.xy - center) / resolution);

    // Limiter l'effet avec une atténuation exponentielle
    float attenuation = pow(1.0 - clamp(distance_to_center, 0.0, 1.0), 2.5);

    // Appliquer l'atténuation à la couleur finale
    col *= attenuation;

    FragColor = vec4(col, 1.0);
}
)";

auto starguy_shader_source = R"(
#version 330 core

out vec4 FragColor;

uniform float time;
uniform vec2 resolution;
uniform vec2 rect_position; // Position du rectangle


// hash without sine
float hash11(float p) {
    vec3 p3 = fract(vec3(p) * vec3(.1031, .11369, .13787));
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}

// 1D smooth noise
float snoise1d(float f) {
    return mix(
        hash11(floor(f)),
        hash11(floor(f + 1.0)),
        smoothstep(0.0, 1.0, fract(f))
    );
}

// Star shape (2D distance estimate)
float StarDE(vec2 p, float n, float r, float i) {
    float rep = floor(-atan(p.x, p.y) * (n / 6.28) + 0.5) / (n / 6.28);
    float s, c;
    s = sin(rep);
    c = cos(rep);
    p = mat2(c, -s, s, c) * p;
    float a = (i + 1.0) * 3.14 / n;
    s = sin(a);
    c = cos(a);
    p = mat2(c, -s, s, c) * vec2(-abs(p.x), p.y - r);
    return length(max(vec2(0.0), p));
}

// StarDE with eyes
float Starguy(vec2 p, float n, float r, float i, vec2 l) {
    float b = pow(abs(fract(0.087 * time + 0.1) - 0.5) * 2.0, 72.0);
    vec2 p2 = p + l;

    return max(
        StarDE(p, n, r, i),
        -length(vec2(
            min(0.0, -abs(abs(p2.x) - r * 0.2) + r * b * 0.1),
            min(0.0, -abs(p2.y) + r * (1.0 - b) * 0.1)
        )) + r * 0.11
    );
}

void main(void) {
    vec2 pixel_pos = gl_FragCoord.xy;

    if (pixel_pos.x < rect_position.x - 50.0 || pixel_pos.x > rect_position.x + resolution.x + 50.0 ||
      pixel_pos.y < rect_position.y - 50.0 || pixel_pos.y > rect_position.y + resolution.y + 50.0) {
      // En dehors de la zone élargie : ne rien dessiner (transparent/noir)
      discard;
    }

    // Position relative à rect_position
    vec2 p = (gl_FragCoord.xy - rect_position - resolution.xy / 2.0) / resolution.y;

    float t = 0.7 * time;
    vec2 p2 = p;
    p2.y += 0.025 * sin(4.0 * t);

    p2 = p2 / dot(p2, p2) - 0.17 * vec2(sin(t), cos(4.0 * t));
    p2 = p2 / dot(p2, p2);

    vec2 look = 0.02 * vec2(cos(0.71 * t), sin(0.24 * t));

    float star = Starguy(p2, 5.0, 0.27, 0.7, look);

    float rad = pow(Starguy(p, 5.0, 0.27, 0.7, look), 0.5);
    rad = snoise1d(24.0 * rad - 2.0 * time) + 0.5 * snoise1d(48.0 * rad - 4.0 * time);

    vec3 col = mix(
        vec3(1.0),
        vec3(-0.1, 0.0, 0.0),

     //   vec3(-0.1, 0.0, 0.2),
        clamp(star / 0.01, 0.0, 1.0)
    ) + 4.5 * vec3(1.0, 0.5, 0.23) * (1.05 - pow(star, 0.05)) * (1.0 - 0.04 * rad);


    // Distance relative au centre (normalisée)
    vec2 center = rect_position + resolution / 2.0;
    float distance_to_center = length((gl_FragCoord.xy - center) / resolution);
  //  float attenuation = 1 - clamp(distance_to_center, 0.0, 1.0); // Va de 1.0 (centre) à 0.0 (bord)
    float attenuation = pow(1.0 - clamp(distance_to_center, 0.0, 1.0), 2.5);


    // Appliquer l'atténuation
    col *= attenuation;

    FragColor = vec4(col, 1.0);
}

)";

auto enemy_shader_source = R"(
#version 330 core

out vec4 FragColor;

uniform float time;
uniform vec2 resolution;
uniform vec2 rect_position; // Position du rectangle

void main(void){
    vec2 pixel_pos = gl_FragCoord.xy;

    // Vérifier si le pixel est en dehors de la zone élargie
    if (pixel_pos.x < rect_position.x - 50.0 || pixel_pos.x > rect_position.x + resolution.x + 50.0 ||
        pixel_pos.y < rect_position.y - 50.0 || pixel_pos.y > rect_position.y + resolution.y + 50.0) {
        discard; // Ne rien dessiner pour les pixels en dehors de cette zone
    }

    // Position relative à rect_position, normalisée
    vec2 p = (gl_FragCoord.xy - rect_position - resolution.xy / 2.0) / resolution.y;

    float lambda = time * 2.5;

    // Calcul de la distance
    float u = sin((atan(p.y, p.x) - length(p)) * 5.0 + time * 2.0) * 0.3 + 0.2;
    float t = 0.01 / abs(0.5 + u - length(p));

    // Exemple de produit scalaire
    vec2 something = vec2(0.0, 1.0);
    float dotProduct = dot(vec2(t), something) / length(p);

   // FragColor = vec4(tan(dotProduct), 0.0, sin(t), 1.0);

// Ajuster les couleurs pour les rendre plus claires
    float brightness = 2; // Facteur de luminosité
    vec3 color = vec3(
        tan(dotProduct) * brightness,
        0.0,
        sin(t) * brightness
    );

    // Limiter les couleurs pour éviter les dépassements
    color = clamp(color, 0.0, 1.0);

    FragColor = vec4(color, 1.0);
}
)";

auto score_shader_source = R"(
#version 330 core

out vec4 FragColor;

uniform vec2 resolution;
uniform int text_length; // Length of the text to display
uniform int text_data[128]; // Text as ASCII values (maximum 128 characters)

uniform vec2 position;

#define TEXT _S _c _o _r _e _col _spc _1 _2 _3 // Ex. "Score: 123"

#define CHAR_SIZE vec2(6, 7)
#define CHAR_SPACING vec2(6, 9)

#define DOWN_SCALE 2.0

vec2 res = resolution.xy / DOWN_SCALE;
vec2 start_pos = vec2(0);
vec2 print_pos = vec2(0);
vec2 print_pos_pre_move = vec2(0);
vec3 text_color = vec3(0, 255, 127);

/*
Top left pixel is the most significant bit.
Bottom right pixel is the least significant bit.

 ███  |
█   █ |
█   █ |
█   █ |
█████ |
█   █ |
█   █ |

000000
100010
100010
100010
111110
100010
100010

011100 (upper 21 bits)
100010 -> 011100 100010 100010 100 -> 935188
100010
100
   010 (lower 21 bits)
111110 -> 010 111110 100010 100010 -> 780450
100010
100010

vec2(935188.0,780450.0)
*/

//Text coloring
#define HEX(i) text_color = mod(vec3(i / 65536,i / 256,i),vec3(256.0))/255.0;
#define RGB(r,g,b) text_color = vec3(r,g,b);

#define STRWIDTH(c) (c * CHAR_SPACING.x)
#define STRHEIGHT(c) (c * CHAR_SPACING.y)
#define BEGIN_TEXT(x,y) print_pos = floor(vec2(x,y)); start_pos = floor(vec2(x,y));

//Automatically generated from the sprite sheet here: http://uzebox.org/wiki/index.php?title=File:Font6x8.png
/*
#define _ col+=char(vec2(0.0,0.0),uv);
#define _spc col+=char(vec2(0.0,0.0),uv)*text_color;
#define _exc col+=char(vec2(276705.0,32776.0),uv)*text_color;
#define _quo col+=char(vec2(1797408.0,0.0),uv)*text_color;
#define _hsh col+=char(vec2(10738.0,1134484.0),uv)*text_color;
#define _dol col+=char(vec2(538883.0,19976.0),uv)*text_color;
#define _pct col+=char(vec2(1664033.0,68006.0),uv)*text_color;
#define _amp col+=char(vec2(545090.0,174362.0),uv)*text_color;
#define _apo col+=char(vec2(798848.0,0.0),uv)*text_color;
#define _lbr col+=char(vec2(270466.0,66568.0),uv)*text_color;
#define _rbr col+=char(vec2(528449.0,33296.0),uv)*text_color;
#define _ast col+=char(vec2(10471.0,1688832.0),uv)*text_color;
#define _crs col+=char(vec2(4167.0,1606144.0),uv)*text_color;
#define _per col+=char(vec2(0.0,1560.0),uv)*text_color;
#define _dsh col+=char(vec2(7.0,1572864.0),uv)*text_color;
#define _com col+=char(vec2(0.0,1544.0),uv)*text_color;
#define _lsl col+=char(vec2(1057.0,67584.0),uv)*text_color;
#define _0 col+=char(vec2(935221.0,731292.0),uv)*text_color;
#define _1 col+=char(vec2(274497.0,33308.0),uv)*text_color;
#define _2 col+=char(vec2(934929.0,1116222.0),uv)*text_color;
#define _3 col+=char(vec2(934931.0,1058972.0),uv)*text_color;
#define _4 col+=char(vec2(137380.0,1302788.0),uv)*text_color;
#define _5 col+=char(vec2(2048263.0,1058972.0),uv)*text_color;
#define _6 col+=char(vec2(401671.0,1190044.0),uv)*text_color;
#define _7 col+=char(vec2(2032673.0,66576.0),uv)*text_color;
#define _8 col+=char(vec2(935187.0,1190044.0),uv)*text_color;
#define _9 col+=char(vec2(935187.0,1581336.0),uv)*text_color;
#define _col col+=char(vec2(195.0,1560.0),uv)*text_color;
#define _scl col+=char(vec2(195.0,1544.0),uv)*text_color;
#define _les col+=char(vec2(135300.0,66052.0),uv)*text_color;
#define _equ col+=char(vec2(496.0,3968.0),uv)*text_color;
#define _grt col+=char(vec2(528416.0,541200.0),uv)*text_color;
#define _que col+=char(vec2(934929.0,1081352.0),uv)*text_color;
#define _ats col+=char(vec2(935285.0,714780.0),uv)*text_color;
#define _A col+=char(vec2(935188.0,780450.0),uv)*text_color;
#define _B col+=char(vec2(1983767.0,1190076.0),uv)*text_color;
#define _C col+=char(vec2(935172.0,133276.0),uv)*text_color;
#define _D col+=char(vec2(1983764.0,665788.0),uv)*text_color;
#define _E col+=char(vec2(2048263.0,1181758.0),uv)*text_color;
#define _F col+=char(vec2(2048263.0,1181728.0),uv)*text_color;
#define _G col+=char(vec2(935173.0,1714334.0),uv)*text_color;
#define _H col+=char(vec2(1131799.0,1714338.0),uv)*text_color;
#define _I col+=char(vec2(921665.0,33308.0),uv)*text_color;
#define _J col+=char(vec2(66576.0,665756.0),uv)*text_color;
#define _K col+=char(vec2(1132870.0,166178.0),uv)*text_color;
#define _L col+=char(vec2(1065220.0,133182.0),uv)*text_color;
#define _M col+=char(vec2(1142100.0,665762.0),uv)*text_color;
#define _N col+=char(vec2(1140052.0,1714338.0),uv)*text_color;
#define _O col+=char(vec2(935188.0,665756.0),uv)*text_color;
#define _P col+=char(vec2(1983767.0,1181728.0),uv)*text_color;
#define _Q col+=char(vec2(935188.0,698650.0),uv)*text_color;
#define _R col+=char(vec2(1983767.0,1198242.0),uv)*text_color;
#define _S col+=char(vec2(935171.0,1058972.0),uv)*text_color;
#define _T col+=char(vec2(2035777.0,33288.0),uv)*text_color;
#define _U col+=char(vec2(1131796.0,665756.0),uv)*text_color;
#define _V col+=char(vec2(1131796.0,664840.0),uv)*text_color;
#define _W col+=char(vec2(1131861.0,699028.0),uv)*text_color;
#define _X col+=char(vec2(1131681.0,84130.0),uv)*text_color;
#define _Y col+=char(vec2(1131794.0,1081864.0),uv)*text_color;
#define _Z col+=char(vec2(1968194.0,133180.0),uv)*text_color;
#define _lsb col+=char(vec2(925826.0,66588.0),uv)*text_color;
#define _rsl col+=char(vec2(16513.0,16512.0),uv)*text_color;
#define _rsb col+=char(vec2(919584.0,1065244.0),uv)*text_color;
#define _pow col+=char(vec2(272656.0,0.0),uv)*text_color;
#define _usc col+=char(vec2(0.0,62.0),uv)*text_color;
#define _a col+=char(vec2(224.0,649374.0),uv)*text_color;
#define _b col+=char(vec2(1065444.0,665788.0),uv)*text_color;
#define _c col+=char(vec2(228.0,657564.0),uv)*text_color;
#define _d col+=char(vec2(66804.0,665758.0),uv)*text_color;
#define _e col+=char(vec2(228.0,772124.0),uv)*text_color;
#define _f col+=char(vec2(401543.0,1115152.0),uv)*text_color;
#define _g col+=char(vec2(244.0,665474.0),uv)*text_color;
#define _h col+=char(vec2(1065444.0,665762.0),uv)*text_color;
#define _i col+=char(vec2(262209.0,33292.0),uv)*text_color;
#define _j col+=char(vec2(131168.0,1066252.0),uv)*text_color;
#define _k col+=char(vec2(1065253.0,199204.0),uv)*text_color;
#define _l col+=char(vec2(266305.0,33292.0),uv)*text_color;
#define _m col+=char(vec2(421.0,698530.0),uv)*text_color;
#define _n col+=char(vec2(452.0,1198372.0),uv)*text_color;
#define _o col+=char(vec2(228.0,665756.0),uv)*text_color;
#define _p col+=char(vec2(484.0,667424.0),uv)*text_color;
#define _q col+=char(vec2(244.0,665474.0),uv)*text_color;
#define _r col+=char(vec2(354.0,590904.0),uv)*text_color;
#define _s col+=char(vec2(228.0,114844.0),uv)*text_color;
#define _t col+=char(vec2(8674.0,66824.0),uv)*text_color;
#define _u col+=char(vec2(292.0,1198868.0),uv)*text_color;
#define _v col+=char(vec2(276.0,664840.0),uv)*text_color;
#define _w col+=char(vec2(276.0,700308.0),uv)*text_color;
#define _x col+=char(vec2(292.0,1149220.0),uv)*text_color;
#define _y col+=char(vec2(292.0,1163824.0),uv)*text_color;
#define _z col+=char(vec2(480.0,1148988.0),uv)*text_color;
#define _lpa col+=char(vec2(401542.0,66572.0),uv)*text_color;
#define _bar col+=char(vec2(266304.0,33288.0),uv)*text_color;
#define _rpa col+=char(vec2(788512.0,1589528.0),uv)*text_color;
#define _tid col+=char(vec2(675840.0,0.0),uv)*text_color;
#define _lar col+=char(vec2(8387.0,1147904.0),uv)*text_color;
#define _nl print_pos = start_pos - vec2(0,CHAR_SPACING.y);
*/

// Static table for character coordinates (bit-packed sprites)
const vec2 char_table[127] = vec2[](
    // ASCII 0-31 (control characters, unused, default to vec2(0.0))
    vec2(0.0, 0.0), vec2(0.0, 0.0), vec2(0.0, 0.0), vec2(0.0, 0.0), // 0-3
    vec2(0.0, 0.0), vec2(0.0, 0.0), vec2(0.0, 0.0), vec2(0.0, 0.0), // 4-7
    vec2(0.0, 0.0), vec2(0.0, 0.0), vec2(0.0, 0.0), vec2(0.0, 0.0), // 8-11
    vec2(0.0, 0.0), vec2(0.0, 0.0), vec2(0.0, 0.0), vec2(0.0, 0.0), // 12-15
    vec2(0.0, 0.0), vec2(0.0, 0.0), vec2(0.0, 0.0), vec2(0.0, 0.0), // 16-19
    vec2(0.0, 0.0), vec2(0.0, 0.0), vec2(0.0, 0.0), vec2(0.0, 0.0), // 20-23
    vec2(0.0, 0.0), vec2(0.0, 0.0), vec2(0.0, 0.0), vec2(0.0, 0.0), // 24-27
    vec2(0.0, 0.0), vec2(0.0, 0.0), vec2(0.0, 0.0), vec2(0.0, 0.0), // 28-31

    // ASCII 32-47 (space, punctuation)
    vec2(0.0, 0.0),              // 32 (space)
    vec2(798848.0, 0.0),         // 33 '!'
    vec2(1797408.0, 0.0),        // 34 '"'
    vec2(10738.0, 1134484.0),    // 35 '#'
    vec2(538883.0, 19976.0),     // 36 '$'
    vec2(1664033.0, 68006.0),    // 37 '%'
    vec2(545090.0, 174362.0),    // 38 '&'
    vec2(798848.0, 0.0),         // 39 '\''
    vec2(270466.0, 66568.0),     // 40 '('
    vec2(528449.0, 33296.0),     // 41 ')'
    vec2(10471.0, 1688832.0),    // 42 '*'
    vec2(4167.0, 1606144.0),     // 43 '+'
    vec2(0.0, 1544.0),           // 44 ','
    vec2(7.0, 1572864.0),        // 45 '-'
    vec2(0.0, 1560.0),           // 46 '.'
    vec2(1057.0, 67584.0),       // 47 '/'

    // ASCII 48-57 (numbers '0'-'9')
    vec2(935221.0, 731292.0),    // 48 '0'
    vec2(274497.0, 33308.0),     // 49 '1'
    vec2(934929.0, 1116222.0),   // 50 '2'
    vec2(934931.0, 1058972.0),   // 51 '3'
    vec2(137380.0, 1302788.0),   // 52 '4'
    vec2(2048263.0, 1058972.0),  // 53 '5'
    vec2(401671.0, 1190044.0),   // 54 '6'
    vec2(2032673.0, 66576.0),    // 55 '7'
    vec2(935187.0, 1190044.0),   // 56 '8'
    vec2(935187.0, 1581336.0),   // 57 '9'

    // ASCII 58-64 (punctuation)
    vec2(195.0, 1560.0),         // 58 ':'
    vec2(195.0, 1544.0),         // 59 ';'
    vec2(135300.0, 66052.0),     // 60 '<'
    vec2(496.0, 3968.0),         // 61 '='
    vec2(528416.0, 541200.0),    // 62 '>'
    vec2(934929.0, 1081352.0),   // 63 '?'
    vec2(935285.0, 714780.0),    // 64 '@'

    // ASCII 65-90 (uppercase letters 'A'-'Z')
    vec2(935188.0, 780450.0),    // 65 'A'
    vec2(1983767.0, 1190076.0),  // 66 'B'
    vec2(935172.0, 133276.0),    // 67 'C'
    vec2(1983764.0, 665788.0),   // 68 'D'
    vec2(2048263.0, 1181758.0),  // 69 'E'
    vec2(2048263.0, 1181728.0),  // 70 'F'
    vec2(935173.0, 1714334.0),   // 71 'G'
    vec2(1131799.0, 1714338.0),  // 72 'H'
    vec2(921665.0, 33308.0),     // 73 'I'
    vec2(66576.0, 665756.0),     // 74 'J'
    vec2(1132870.0, 166178.0),   // 75 'K'
    vec2(1065220.0, 133182.0),   // 76 'L'
    vec2(1142100.0, 665762.0),   // 77 'M'
    vec2(1140052.0, 1714338.0),  // 78 'N'
    vec2(935188.0, 665756.0),    // 79 'O'
    vec2(1983767.0, 1181728.0),  // 80 'P'
    vec2(935188.0, 698650.0),    // 81 'Q'
    vec2(1983767.0, 1198242.0),  // 82 'R'
    vec2(935171.0, 1058972.0),   // 83 'S'
    vec2(2035777.0, 33288.0),    // 84 'T'
    vec2(1131796.0, 665756.0),   // 85 'U'
    vec2(1131796.0, 664840.0),   // 86 'V'
    vec2(1131861.0, 699028.0),   // 87 'W'
    vec2(1131681.0, 84130.0),    // 88 'X'
    vec2(1131794.0, 1081864.0),  // 89 'Y'
    vec2(1968194.0, 133180.0),   // 90 'Z'

    // ASCII 91-96 (miscellaneous)
    vec2(925826.0, 66588.0),     // 91 '['
    vec2(1057.0, 67584.0),       // 92 '\'
    vec2(919584.0, 1065244.0),   // 93 ']'
    vec2(272656.0, 0.0),         // 94 '^'
    vec2(0.0, 62.0),             // 95 '_'

    // ASCII 97-122 (lowercase letters 'a'-'z')
    vec2(224.0, 649374.0),       // 97 'a'
    vec2(1065444.0, 665788.0),   // 98 'b'
    vec2(228.0, 657564.0),       // 99 'c'
    vec2(66804.0, 665758.0),     // 100 'd'
    vec2(228.0, 772124.0),       // 101 'e'
    vec2(401543.0, 1115152.0),   // 102 'f'
    vec2(244.0, 665474.0),       // 103 'g'
    vec2(1065444.0, 665762.0),   // 104 'h'
    vec2(262209.0, 33292.0),     // 105 'i'
    vec2(131168.0, 1066252.0),   // 106 'j'
    vec2(1065253.0, 199204.0),   // 107 'k'
    vec2(266305.0, 33292.0),     // 108 'l'
    vec2(421.0, 698530.0),       // 109 'm'
    vec2(452.0, 1198372.0),      // 110 'n'
    vec2(228.0, 665756.0),       // 111 'o'
    vec2(484.0, 667424.0),       // 112 'p'
    vec2(244.0, 665474.0),       // 113 'q'
    vec2(354.0, 590904.0),       // 114 'r'
    vec2(228.0, 114844.0),       // 115 's'
    vec2(8674.0, 66824.0),       // 116 't'
    vec2(292.0, 1198868.0),      // 117 'u'
    vec2(276.0, 664840.0),       // 118 'v'
    vec2(276.0, 700308.0),       // 119 'w'
    vec2(292.0, 1149220.0),      // 120 'x'
    vec2(292.0, 1163824.0),      // 121 'y'
    vec2(480.0, 1148988.0),      // 122 'z'

    // ASCII 123-127 (miscellaneous)
    vec2(401542.0, 66572.0),     // 123 '{'
    vec2(266304.0, 33288.0),     // 124 '|'
    vec2(788512.0, 1589528.0),   // 125 '}'
    vec2(675840.0, 0.0),         // 126 '~'
    vec2(0.0, 0.0)               // 127 (DEL, unused)
);


//Extracts bit b from the given number.
float extract_bit(float n, float b)
{
	b = clamp(b,-1.0,22.0);
	return floor(mod(floor(n / pow(2.0,floor(b))),2.0));
}

//Returns the pixel at uv in the given bit-packed sprite.
float sprite(vec2 spr, vec2 size, vec2 uv)
{
	uv = floor(uv);
	float bit = (size.x-uv.x-0.0) + uv.y * size.x;
	bool bounds = all(greaterThanEqual(uv,vec2(0)))&& all(lessThan(uv,size));
	return bounds ? extract_bit(spr.x, bit - 21.0) + extract_bit(spr.y, bit) : 0.0;
}

/*
//Prints a character and moves the print position forward by 1 character width.
vec3 char(vec2 ch, vec2 uv)
{
	float px = sprite(ch, CHAR_SIZE, uv - print_pos);
	print_pos.x += CHAR_SPACING.x;
	// print_pos.y += -0.04*floor(5.*sin(298.9113*print_pos.x-time*10.));
	return vec3(px);
}
*/

// Function to draw a character
vec3 renderChar(int ascii, vec2 uv) {
    // Default to blank sprite if the character is not printable
    if (ascii < 32 || ascii > 126) {
        return vec3(0.0); // Non-printable characters return blank
    }

    // Lookup bit-packed sprite for the given ASCII character
    vec2 ch = char_table[ascii]; // char_table maps ASCII to sprite data

    // Render the character's sprite
    float px = sprite(ch, CHAR_SIZE, uv - print_pos);

    // Move the print position forward for the next character
    print_pos.x += CHAR_SPACING.x;

    // Return the color of the character
    return vec3(px);
}

/*
vec3 renderText(vec2 uv)
{
    	vec3 col = vec3(0.0);

    	vec2 center_pos = vec2(res.x/2.0 - STRWIDTH(17.0)/2.0,res.y/2.0 - STRHEIGHT(1.0)/2.0);

    	BEGIN_TEXT(center_pos.x,center_pos.y)
	HEX(0x00FF7F) TEXT
	//BEGIN_TEXT(res.x/2.0-STRWIDTH(11.0)/2.0,res.y/2.0)
	//print_pos += vec2(cos(time)*96.,sin(time)*96.);

	// RGB(1,0,0) _M RGB(1,.5,0)_o RGB(1,1,0)_v RGB(0,1,0)_i RGB(0,.5,1)_n RGB(0.5,0,1)_g _ RGB(1,0,0)_T RGB(1,.5,0)_e RGB(1,1,0)_x RGB(0,1,0)_t

    	return col;
}
*/

// Function to render the text
vec3 renderText(vec2 uv) {
    vec3 col = vec3(0.0);
   // vec2 center_pos = vec2(res.x / 2.0 - text_length * CHAR_SPACING.x / 2.0, res.y / 2.0 - CHAR_SIZE.y / 2.0);

    vec2 center_pos = vec2(
        position.x - (text_length * CHAR_SPACING.x) / 2.0,
        position.y - (CHAR_SIZE.y) / 2.0
    );

    print_pos = center_pos;

    for (int i = 0; i < text_length; i++) {
        col += renderChar(text_data[i], uv);
    }

    return col * text_color;
}

void main( void )
{
    vec2 uv = gl_FragCoord.xy / DOWN_SCALE;

    vec3 col = renderText(uv);

    FragColor = vec4(col, 1.0);
}
)";

Renderer::Renderer(const int width, const int height, const std::string& title)
    : width_(width), height_(height) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    throw std::runtime_error("Failed to initialize SDL: " + std::string(SDL_GetError()));
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  window_ = SDL_CreateWindow(
      title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

  if (!window_) {
    throw std::runtime_error("Failed to create SDL window: " + std::string(SDL_GetError()));
  }

  gl_context_ = SDL_GL_CreateContext(window_);
  if (!gl_context_) {
    throw std::runtime_error("Failed to create OpenGL context: " + std::string(SDL_GetError()));
  }

  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK) {
    throw std::runtime_error("Failed to initialize GLEW");
  }

  glViewport(0, 0, width_, height_);
  glEnable(GL_BLEND);
//  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);

  // Initialize OpenGL resources
  InitOpenGL();

  // Initialize camera
 // camera_.projection_matrix = glm::ortho(0.0f, static_cast<float>(width_), 0.0f, static_cast<float>(height_));

  camera_.projection_matrix = glm::ortho(0.0f, static_cast<float>(width_), static_cast<float>(height_), 0.0f);
  camera_.position = glm::vec2(0.0f, 0.0f);
}


void Renderer::InitOpenGL() {
  shader_program_ = LoadShaders(vertex_shader_source, fragment_shader_source);

  neon_bar_horizontal_program_ = LoadShaders(vertex_shader_source, neon_bar_horizontal_shader);
  neon_bar_vertical_program_ = LoadShaders(vertex_shader_source, neon_bar_vertical_shader);
  starguy_program_ = LoadShaders(vertex_shader_source, starguy_shader_source);
  projectile_program_ = LoadShaders(vertex_shader_source, projectile_shader_source);
  enemy_program_ = LoadShaders(vertex_shader_source, enemy_shader_source);
  score_shader_program_ = LoadShaders(vertex_shader_source, score_shader_source);

  glGenVertexArrays(1, &vao_);
  glGenBuffers(1, &vbo_);

  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, nullptr, GL_DYNAMIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

Renderer::~Renderer() {
  Shutdown();
}

GLuint Renderer::LoadShaders(const char* vertex_source, const char* fragment_source) {
  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_source, nullptr);
  glCompileShader(vertex_shader);
  CheckShaderCompileError(vertex_shader);

  GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_source, nullptr);
  glCompileShader(fragment_shader);
  CheckShaderCompileError(fragment_shader);

  GLuint program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);
  CheckProgramLinkError(program);

  std::cout << "Shaders compiled and linked successfully!" << std::endl;

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  return program;
}


void Renderer::CheckShaderCompileError(const GLuint shader) {
  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char info_log[512];
    glGetShaderInfoLog(shader, 512, nullptr, info_log);
    throw std::runtime_error("Shader compile error: " + std::string(info_log));
  }
}

void Renderer::CheckProgramLinkError(const GLuint program) {
  GLint success;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    char info_log[512];
    glGetProgramInfoLog(program, 512, nullptr, info_log);
    throw std::runtime_error("Program link error: " + std::string(info_log));
  }
}

void Renderer::UpdateCamera(const std::pair<float, float>& position) {
  camera_.position = glm::vec2(position.first - width_ / 2.0f, -position.second - height_ / 2.0f);
}

void Renderer::DrawHorizontalNeonBar(const glm::vec2& map_position, const glm::vec2& size) const {
  glUseProgram(neon_bar_horizontal_program_);

  glUniformMatrix4fv(glGetUniformLocation(neon_bar_horizontal_program_, "projection"), 1, GL_FALSE, glm::value_ptr(camera_.projection_matrix));

  glUniform1f(glGetUniformLocation(neon_bar_horizontal_program_, "time"), static_cast<float>(SDL_GetTicks()) / 500.0f);
  glUniform2f(glGetUniformLocation(neon_bar_horizontal_program_, "resolution"),
              static_cast<float>(width_), static_cast<float>(height_));

  const glm::vec2 screen_position = map_position - camera_.position;

  glUniform2f(glGetUniformLocation(neon_bar_horizontal_program_, "rect_position"), screen_position.x, screen_position.y);

  glUniform2f(glGetUniformLocation(neon_bar_horizontal_program_, "rect_size"), size.x, size.y);

  const float vertices[] = {
    0.0f, static_cast<float>(height_),
    static_cast<float>(width_), static_cast<float>(height_),
    static_cast<float>(width_), 0.0f,
    0.0f, 0.0f,
  };


  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glUseProgram(0);
}

void Renderer::DrawVerticalNeonBar(const glm::vec2& map_position, const glm::vec2& size) const {
  glUseProgram(neon_bar_vertical_program_);

  glUniformMatrix4fv(glGetUniformLocation(neon_bar_vertical_program_, "projection"), 1, GL_FALSE, glm::value_ptr(camera_.projection_matrix));

  glUniform1f(glGetUniformLocation(neon_bar_vertical_program_, "time"), static_cast<float>(SDL_GetTicks()) / 500.0f);
  glUniform2f(glGetUniformLocation(neon_bar_vertical_program_, "resolution"),
              static_cast<float>(width_), static_cast<float>(height_));

  const glm::vec2 screen_position = map_position - camera_.position;

  glUniform2f(glGetUniformLocation(neon_bar_vertical_program_, "rect_position"), screen_position.x, screen_position.y);

  glUniform2f(glGetUniformLocation(neon_bar_vertical_program_, "rect_size"), size.x, size.y);

  const float vertices[] = {
    0.0f, static_cast<float>(height_),  // Coin supérieur gauche
    static_cast<float>(width_), static_cast<float>(height_),  // Coin supérieur droit
    static_cast<float>(width_), 0.0f,  // Coin inférieur droit
    0.0f, 0.0f,  // Coin inférieur gauche
  };


  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glUseProgram(0);
}

void Renderer::DrawVisibleHorizontalBar(const glm::vec2& position, const glm::vec2& size) const {
  const float visible_start_y = std::max(position.y, camera_.position.y);
  const float visible_end_y = std::min(position.y + size.y, camera_.position.y + height_);
  const float visible_height = visible_end_y - visible_start_y;

  if (visible_height > 0.0f) {
    const glm::vec2 rect_position = position;
    const glm::vec2 rect_size = size;

    //std::cout << "Visible horizontal bar: " << rect_position.x << ", " << rect_position.y
    //          << ", " << rect_size.x << ", " << rect_size.y << '\n';

    DrawHorizontalNeonBar(rect_position, rect_size);
  } else {
    // std::cout << "Horizontal bar is not visible, skipping draw.\n";
  }
}

void Renderer::DrawVisibleVerticalBar(const glm::vec2& position, const glm::vec2& size) const {
  const float visible_start_x = std::max(position.x, camera_.position.x);
  const float visible_end_x = std::min(position.x + size.x, camera_.position.x + width_);
  const float visible_width = visible_end_x - visible_start_x;

  if (visible_width > 0.0f) {
    const glm::vec2 rect_position = position;
    const glm::vec2 rect_size = size;

    // std::cout << "Visible vertical bar: " << rect_position.x << ", " << rect_position.y
    //          << ", " << rect_size.x << ", " << rect_size.y << '\n';

    DrawVerticalNeonBar(rect_position, rect_size);
  } else {
    // std::cout << "Vertical bar is not visible, skipping draw.\n";
  }
}


void Renderer::DrawVisibleBar(const glm::vec2& position,
                              const glm::vec2& size) const {
  const float visible_start_y = std::max(position.y, camera_.position.y);
  const float visible_end_y = std::min(position.y + size.y, camera_.position.y + height_);

  if (const float visible_height = visible_end_y - visible_start_y ;visible_height > 0.0f) {
    const glm::vec2 rect_position = position;
    const glm::vec2 rect_size = size;

    // std::cout << "Visible bar: " << rect_position.x << ", " << rect_position.y
    //          << ", " << rect_size.x << ", " << rect_size.y << '\n';

    DrawHorizontalNeonBar(rect_position, rect_size);
  } else {
    std::cout << "Bar is not visible, skipping draw.\n";
  }
}

void Renderer::DrawStarguy(const glm::vec2& map_position, const glm::vec2& size) const {
  glUseProgram(starguy_program_);

  glUniformMatrix4fv(glGetUniformLocation(starguy_program_, "projection"), 1, GL_FALSE, glm::value_ptr(camera_.projection_matrix));

  glUniform1f(glGetUniformLocation(starguy_program_, "time"), static_cast<float>(SDL_GetTicks()) / 1000.0f);
  glUniform2f(glGetUniformLocation(starguy_program_, "resolution"), static_cast<float>(size.x), static_cast<float>(size.y));

  glm::vec2 screen_position = map_position - camera_.position;

  screen_position.x -= size.x / 2.0f;
  screen_position.y -= size.y / 2.0f;

  glUniform2f(glGetUniformLocation(starguy_program_, "rect_position"), screen_position.x, screen_position.y);

  const float vertices[] = {
    0.0f, static_cast<float>(height_),
    static_cast<float>(width_), static_cast<float>(height_),
    static_cast<float>(width_), 0.0f,
    0.0f, 0.0f
};

  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glUseProgram(0);
}

void Renderer::DrawProjectile(const glm::vec2& map_position, const glm::vec2& size) const {
  glUseProgram(projectile_program_);

  glUniformMatrix4fv(glGetUniformLocation(projectile_program_, "projection"), 1, GL_FALSE, glm::value_ptr(camera_.projection_matrix));

  glm::vec2 screen_position = map_position - camera_.position;

  screen_position.x -= size.x / 2.0f;
  screen_position.y -= size.y / 2.0f;

  glUniform1f(glGetUniformLocation(projectile_program_, "time"), static_cast<float>(SDL_GetTicks()) / 1000.0f);
  glUniform2f(glGetUniformLocation(projectile_program_, "resolution"), static_cast<float>(size.x), static_cast<float>(size.y));
  glUniform2f(glGetUniformLocation(projectile_program_, "rect_position"), screen_position.x, screen_position.y);

  const float vertices[] = {
    0.0f, static_cast<float>(height_),
    static_cast<float>(width_), static_cast<float>(height_),
    static_cast<float>(width_), 0.0f,
    0.0f, 0.0f
};

  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glUseProgram(0);
}

void Renderer::DrawEnemy(const glm::vec2& map_position, const glm::vec2& size) const {
  glUseProgram(enemy_program_);

  glUniformMatrix4fv(glGetUniformLocation(enemy_program_, "projection"), 1, GL_FALSE, glm::value_ptr(camera_.projection_matrix));

  glm::vec2 screen_position = map_position - camera_.position;

  screen_position.x -= size.x / 2.0f;
  screen_position.y -= size.y / 2.0f;

  glUniform1f(glGetUniformLocation(enemy_program_, "time"), static_cast<float>(SDL_GetTicks()) / 200.0f);
  glUniform2f(glGetUniformLocation(enemy_program_, "resolution"), static_cast<float>(size.x), static_cast<float>(size.y));
  glUniform2f(glGetUniformLocation(enemy_program_, "rect_position"), screen_position.x, screen_position.y);

  const float vertices[] = {
    0.0f, static_cast<float>(height_),
    static_cast<float>(width_), static_cast<float>(height_),
    static_cast<float>(width_), 0.0f,
    0.0f, 0.0f
};

  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glUseProgram(0);
}

void Renderer::DrawScore(const int score, const glm::vec2& position) const {
  glUseProgram(score_shader_program_);

  const std::string score_str = std::to_string(score);
  const int text_length = static_cast<int>(score_str.size());
  int text_data[128] = {};

  for (int i = 0; i < text_length; ++i) {
    text_data[i] = static_cast<int>(score_str[i]);
  }

  glUniformMatrix4fv(glGetUniformLocation(score_shader_program_, "projection"), 1, GL_FALSE, glm::value_ptr(camera_.projection_matrix));
  glUniform2f(glGetUniformLocation(score_shader_program_, "resolution"),
              static_cast<float>(width_), static_cast<float>(height_));
  glUniform1i(glGetUniformLocation(score_shader_program_, "text_length"), text_length);

  const GLuint text_data_location = glGetUniformLocation(score_shader_program_, "text_data");
  glUniform1iv(text_data_location, 128, text_data);

  glUniform2f(glGetUniformLocation(score_shader_program_, "position"), position.x, position.y);

  const float vertices[] = {
    0.0f, static_cast<float>(height_),
    static_cast<float>(width_), static_cast<float>(height_),
    static_cast<float>(width_), 0.0f,
    0.0f, 0.0f
};

  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glUseProgram(0);
}


void Renderer::DrawGame(const client::GameState& game_state) const {
    constexpr glm::vec2 left_bar_position = {-200.0f, -2100.0f};
    constexpr glm::vec2 left_bar_size = {400.0f, 2200.0f};

    DrawVisibleVerticalBar(left_bar_position, left_bar_size);

    constexpr glm::vec2 right_bar_position = {1800.0f, -2100.0f};
    constexpr glm::vec2 right_bar_size = {400.0f, 2200.0f};

    DrawVisibleVerticalBar(right_bar_position, right_bar_size);

    constexpr glm::vec2 bottom_bar_position = {-100.0f, -2200.0f};
    constexpr glm::vec2 bottom_bar_size = {2200.0f,
                                 400.0f};

    DrawVisibleHorizontalBar(bottom_bar_position, bottom_bar_size);

    constexpr glm::vec2 top_bar_position = {-100.0f, -200.0f};
    constexpr glm::vec2 top_bar_size = {2200.0f, 400.0f};

    DrawVisibleHorizontalBar(top_bar_position, top_bar_size);

    auto& registry = game_state.GetRegistry();
    auto& positions = registry.get_components<Position>();
    auto& players = registry.get_components<ClientPlayer>();
    auto& enemies = registry.get_components<Enemy>();
    auto& projectiles = registry.get_components<Projectile>();

    for (size_t i = 0; i < positions.size(); ++i) {
      if (!positions[i].has_value()) {
        continue;
      }

      const auto& [x, y] = *positions[i];

      glm::vec2 screen_position = {
        x,
        -y
      };

      if (players[i].has_value()) {
        DrawStarguy(screen_position, {120.0f, 120.0f});
      } else if (enemies[i].has_value()) {
        DrawEnemy(screen_position, {30.0f, 30.0f});
      } else if (projectiles[i].has_value()) {
        DrawProjectile(screen_position, {120.0f, 120.0f});
      }
    }
    DrawScore(game_state.GetLocalPlayerScore(), {600.0f, 20.0f});
    DrawScore(game_state.GetLocalPlayerHealth(), {600.0f, 50.0f});
}

void Renderer::Clear() const {
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::Present() const {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  SDL_GL_SwapWindow(window_);
}

void Renderer::Shutdown() {
  glDeleteVertexArrays(1, &vao_);
  glDeleteBuffers(1, &vbo_);
  glDeleteProgram(shader_program_);
  glDeleteProgram(neon_bar_horizontal_program_);
  glDeleteProgram(neon_bar_vertical_program_);
  glDeleteProgram(starguy_program_);
  glDeleteProgram(projectile_program_);

  if (gl_context_) {
    SDL_GL_DeleteContext(gl_context_);
    gl_context_ = nullptr;
  }
  if (window_) {
    SDL_DestroyWindow(window_);
    window_ = nullptr;
  }
}
