float2 uv = Idx < 0.5 ? T0 : (Idx < 1.5 ? T1 : (Idx < 2.5 ? T2 : T3));
uv = TS.xy + uv * TS.zw;
uv.y = 1.0 - uv.y;
float3 c = Texture2DSample(Tex, TexSampler, uv).rgb;
// Black Marble (~600 m par pixel) dit OU il y a des lumieres ; fond bleu nuit retire
float l = max(c.r, max(c.g, c.b));
float zone = saturate((l - 0.12) / 0.5);
zone *= zone;
// La photo du jour dit QUELS pixels peuvent s'allumer : toits et routes (clairs, peu satures)
float lum = dot(BC, float3(0.2126, 0.7152, 0.0722));
float sat = max(BC.r, max(BC.g, BC.b)) - min(BC.r, min(BC.g, BC.b));
float built = saturate((lum - 0.2) * 4.0) * saturate(1.0 - sat * 3.0);
// Points epars (fenetres, lampadaires) plutot que des toits entiers allumes
float2 q = floor(P.xy / 400.0);
float r = frac(sin(dot(q, float2(12.9898, 78.233))) * 43758.5453);
float sparkle = r * r * r * 3.0;
// Apparait au-dela de 300 m de la camera
float fade = saturate((length(P - Cam) - 30000.0) / 70000.0);
float3 warm = float3(1.0, 0.7, 0.4);
return warm * zone * built * sparkle * fade * saturate(Night) * Intensity * (1.0 - saturate(Water)) * 1.5;
