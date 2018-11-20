varying vec3 frag_pos;
varying vec3 frag_normal;
//
//uniform vec3 light_pos;
//uniform vec3 view_pos;
//uniform vec3 light_color;   //3中颜色分量的强度
//uniform vec3 object_color;
void main(){
    /*
    float ambient_strength = 0.1;
    vec3 ambient = ambient_strength * light_color;

    vec3 norm = normalize(frag_normal);
    vec3 light_dir = normalize(light_pos - frag_pos);   //light_dir从物体表面指向光源
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 disfuss = diff * light_color;

    float specular_strength = 0.5;
    vec3 view_dir = normalize(view_pos - frag_pos); //view_dir从物体表面指向观察者
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
    vec3 specular = specular_strength * spec * light_color;

    vec3 result = (ambient + disfuss + specular) * object_color;
    */
    gl_FragColor = vec4(frag_normal, 1.0);
}
