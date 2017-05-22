uniform sampler2D tex0;

varying  float rotAngle;
varying float texsize;
varying float sprite_pos;


void main (void) {

	float angle		= rotAngle;
	vec2 v2 = gl_TexCoord[0].st;




    vec2 texcoord = vec2(sprite_pos,0) + vec2(gl_TexCoord[0].s * texsize, gl_TexCoord[0].t);    

	gl_FragColor = texture2D(tex0, texcoord) * gl_Color;

    //vec2 texcoord = sprite_coord + gl_TexCoord[0].st * texsize;
    //gl_FragColor = texture2D(tex0, texcoord) * gl_Color;

	//gl_FragColor = vec4(1,0,0,1);
	//gl_FragColor = gl_Color;
}
