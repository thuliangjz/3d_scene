varying vec3 frag_pos;
varying vec3 frag_normal;
varying vec2 texture_coord;

uniform vec3 light_pos;
uniform vec3 view_pos;
uniform vec3 light_color;   //3种颜色分量的强度
uniform vec3 k_a;
uniform vec3 k_d;
uniform vec3 k_s;
uniform sampler2D texture_ambient;
uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;
uniform float use_ambient_sample;
uniform float use_diffuse_sample;
uniform float use_specular_sample;

void main(){
    vec4 tex_default = vec4(1,1,1,1);
    vec3 ambient_strength = vec3(1, 1, 1) * 1;
    vec3 ambient = k_a * light_color * ambient_strength
            * mix(texture2D(texture_ambient, texture_coord), tex_default,use_ambient_sample);

    vec3 diff_strength = vec3(1, 1, 1) * 3;
    vec3 norm = normalize(frag_normal);
    vec3 light_dir = normalize(light_pos - frag_pos);   //light_dir从物体表面指向光源
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 disfuss = diff * light_color * k_d * diff_strength
            * mix(texture2D(texture_diffuse, texture_coord), tex_default, use_diffuse_sample);
    float specular_strength = vec3(1, 1, 1) * 4;
    vec3 view_dir = normalize(view_pos - frag_pos); //view_dir从物体表面指向观察者
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 256);
    vec3 specular = specular_strength * spec * light_color * k_s
           * mix(texture2D(texture_specular, texture_coord), tex_default, use_specular_sample);
    gl_FragColor = vec4(ambient + disfuss + specular, 1.0);
}
