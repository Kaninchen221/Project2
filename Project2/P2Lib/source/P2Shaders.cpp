#include "P2Shaders.hpp"

namespace P2
{
	std::string_view GetVertexShader()
	{
		return R"(
			//#version 450

			void main()
			{
				gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
			}
		)";
	}

	std::string_view GetFragmentShader()
	{
		return R"(
			//#version 450
			
			uniform sampler2D texture;
			
			in vec2 tex_coord;
			
			void main()
			{
			    // Read and apply a color from the texture
			    //gl_FragColor = texture2D(texture, tex_coord);
			    gl_FragColor = gl_Color;
			}
		)";
	}
}