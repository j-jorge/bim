#version 310 es
// SPDX-License-Identifier: AGPL-3.0-only
precision lowp float;
precision lowp int;

layout(location = COLOR0) in vec4 v_color;
layout(location = TEXCOORD0) in vec2 v_texCoord;
layout(binding = 0) uniform sampler2D u_tex0;
layout(std140, binding = 0) uniform fs_ub
{
  vec4 sprite_rect;
  vec2 texture_size;
  float time;
};

layout(location = SV_Target0) out vec4 FragColor;

vec2 rotate(vec2 p, float angle)
{
  mat2 rotation = mat2
    (
     vec2(cos(angle), -sin(angle)),
     vec2(sin(angle), cos(angle))
     );

  const vec2 center = vec2(0.5, 0.5);
  vec2 d = rotation * (p - center);
  return center + d;
}

vec4 run_effect(float t)
{
  vec2 sprite_pos = sprite_rect.xy;
  vec2 sprite_size = sprite_rect.zw;
  vec2 coord_in_sprite =
    (v_texCoord * texture_size - sprite_pos) / sprite_size;

  const float angle = 135.0 * (3.1415926/180.0);
  vec2 p = rotate(coord_in_sprite, angle);
  float total_distance = 2.0 * abs(rotate(vec2(0.0, 0.0), angle).y - 0.5);

  const float min_thickness = 0.07;
  const float max_thickness = 0.2;

  float distance_to_edge =
    2.0
    * (0.5 - max(abs(coord_in_sprite.x - 0.5), abs(coord_in_sprite.y - 0.5)));

  vec4 pixel = texture(u_tex0, v_texCoord);
  vec3 white = vec3(1.0);

  // The thickness-factor of the line at p.x.
  float thickness;

  // Apply a small rounding effect near the sprite's edges.
  if (distance_to_edge <= 0.4)
    {
      float x = (0.4 - distance_to_edge) / 0.4;
      thickness = x * x;
    }
  else
    thickness = 0.0;

  float line_width =
    min_thickness + thickness * (max_thickness - min_thickness);

  // This is the "cubic-out" easing, applied in and out on the ease_time.
  const float ease_time = 0.3;
  float ease_ratio = min(1.0, (0.5 - abs(0.5 - t)) / ease_time);
  float f = ease_ratio - 1.0;
  line_width *= f * f * f + 1.0;

  float distance_to_line =
    abs(p.y - (0.5 - total_distance / 2.0 + t * total_distance));
  float distance_to_line_edge = line_width - distance_to_line;

  // Mix factor: how much white we put on the texture. We apply an
  // antialiasing on the edge. If we are closer than 0.01 to the line,
  // we partially mix the white with the texture. If we are farther
  // than 0.01 we use full white.
  float m = max(0.0, min(distance_to_line_edge, 0.01) / 0.01);

  vec4 result;
  result.rgb = mix(pixel.rgb, white * pixel.a, m);
  result.a = pixel.a;

  return result;
}

void main()
{
  // How many seconds does it take to run through the sprite.
  const float effect_duration = 0.4;

  // How many seconds between two effects.
  const float effect_interval = 5.0;

  const float off_interval = effect_interval - effect_duration;

  float t = max(0.0, mod(time, effect_interval) - off_interval);
  vec4 fragment_color;

  if (t == 0.0)
    fragment_color = texture(u_tex0, v_texCoord);
  else
    fragment_color = run_effect(t / effect_duration);

  FragColor = fragment_color * v_color;
}
