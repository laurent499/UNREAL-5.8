const float R0 = 637100000.0;
float3 C = float3(0.0, 0.0, -R0);
float3 up = normalize(P - C);
float alt = OriginH + (length(P - C) - R0);
// Normale geometrique au pixel : les tuiles Google n'ont pas de normales fiables
float3 gn = normalize(cross(ddx(P), ddy(P)));
gn *= sign(dot(gn, up));
// Pente : surtout la normale geometrique (fiable mais a facettes), adoucie par la normale de sommet
float slope = lerp(saturate(dot(gn, up)), saturate(dot(normalize(N), up)), 0.4);
float land = 1.0 - saturate(Water);
float3 col = max(BC, 0.0);
float lum = dot(col, float3(0.2126, 0.7152, 0.0722));

// Ciel couvert : on adoucit les ombres et le contraste incrustes dans la photo
float overcast = saturate(Cloud / 10.0);
col = lerp(col, pow(col, 0.8) * 0.92, overcast * 0.6);
col = lerp(col, lerp(lum.xxx, col, 0.8), overcast * 0.5);

// Nuit : albedo assombri pour ne pas rallumer une photo prise en plein jour
col *= lerp(1.0, 0.5, saturate(Night));

// Sol mouille : plus sombre et plus brillant
float wet = saturate(Wet) * land;
col *= lerp(1.0, 0.65, wet);
float rough = lerp(R, R * 0.45, wet);

// Neige : au-dessus de la ligne de neige, sur les pentes de moins de ~40 degres, hors eau, bord casse par un bruit
float2 q = P.xy / 4000.0;
float2 fi = floor(q);
float2 ff = frac(q);
ff = ff * ff * (3.0 - 2.0 * ff);
float n00 = frac(sin(dot(fi, float2(127.1, 311.7))) * 43758.5453);
float n10 = frac(sin(dot(fi + float2(1, 0), float2(127.1, 311.7))) * 43758.5453);
float n01 = frac(sin(dot(fi + float2(0, 1), float2(127.1, 311.7))) * 43758.5453);
float n11 = frac(sin(dot(fi + float2(1, 1), float2(127.1, 311.7))) * 43758.5453);
float n = lerp(lerp(n00, n10, ff.x), lerp(n01, n11, ff.x), ff.y);
float snowAlt = smoothstep(SnowLine - 10000.0, SnowLine + 10000.0, alt + (n - 0.5) * 30000.0);
float snowSlope = smoothstep(0.6, 0.95, slope + (n - 0.5) * 0.3);
float snow = snowAlt * snowSlope * land;
// La neige garde un peu du relief de la photo (ombres, rochers affleurants)
float3 snowCol = float3(0.86, 0.89, 0.93) * lerp(0.7, 1.05, saturate(lum * 2.5));
col = lerp(col, snowCol, snow * 0.92);
rough = lerp(rough, 0.55, snow);

return float4(col, rough);
