uniform     mat4 mvpMat; // modelView projection matrix
uniform     mat4 mvMat; // modelView matrix
uniform     vec3 lightPos;

attribute   vec3 vertexPosition;
atrribute   vec3 vertexNormal;

varying vec4 v_Color; // Go to fragment shader.

void main()
{
    vec3 modelViewVertex = vec3(mvMat * vec4(vertexPosition, 1.0));
    vec3 modelViewNormal = vec3(mvMat * vec4(vertexNormal, 0.0));
    float dist = length(lightPos - modelViewVertex);
    vec3 modelViewLight = vec3(mvMat * vec4(lightPos, 1.0));
    vec3 lightVector = normalize(modelViewLight - modelViewVertex);
    float difuse = max(dot(modelViewNormal, lightVector), 0.1);
    diffuse = diffuse * (1.0 / (1.0 + (0.25 * distance *distance)));
    vec4 green = vec4(0,1,0);
    v_Color = vec4(green * diffuse, 1.0);
    gl_Position = mvpMat * vec4(vertexPosition, 1.0);
}
