
attribute float angle;
varying  float rotAngle;
varying float texsize;
varying float sprite_pos;


void main() {
	gl_TexCoord[0] = gl_MultiTexCoord0;
	vec4 eyeCoord = gl_ModelViewMatrix * gl_Vertex;
	gl_Position = gl_ProjectionMatrix * eyeCoord;
	float dist = sqrt(eyeCoord.x*eyeCoord.x + eyeCoord.y*eyeCoord.y + eyeCoord.z*eyeCoord.z);
	float att	 = 600.0 / dist;

	gl_PointSize = 30.0;//gl_Normal.x * att;
	gl_FrontColor = gl_Color;

    float sWidth = gl_Normal.y;

    //float x = mod(sPos,sWidth);
    //float y = floor(sPos/sWidth);
    float w = 1.0/sWidth;
    //sprite_coord = vec2(x * w, y * w);
    sprite_pos = gl_Normal.z * w;
	texsize = w;

	rotAngle = angle;
}





