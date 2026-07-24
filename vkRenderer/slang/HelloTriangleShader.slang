struct VertexInput{
	float3 vertPosition;
	float3 vertColor;
	float2 vectUV;
}; 

struct VertexOutput{
	float4 sv_position : SV_Position;	// system value Position
	float4 cv_vertexColor : VERTEX_COLOR0;	// customed output
	float2 cv_uvCoord : UV_COORD0;		// customed value uv coord
};

struct MVPMatrix{
	float4x4 modelMat;
	float4x4 viewMat;
	float4x4 projectiondMat;
};

ConstantBuffer<MVPMatrix> u_MVPMatix;


Sampler2D s_Texture;


[shader("vertex")]
VertexOutput VertMain(VertexInput vertIn){
	VertexOutput output;
	output.sv_position = 
		mul(u_MVPMatix.projectiondMat, 
			mul(u_MVPMatix.viewMat, 
				mul(u_MVPMatix.modelMat, float4(vertIn.vertPosition, 1.0))
				)
			);
	//output.sv_position = float4(vertIn.vertPosition, 0.0, 1.0);
	//output.cv_vertexColor =float4(u_MVPMatix.modelMat[0].xy,1.0, 1.0);
	output.cv_vertexColor = float4(vertIn.vertColor, 1.0);
	output.cv_uvCoord = vertIn.vectUV;
	return output;
}

[shader("fragment")]
float4 FragMain(VertexOutput sVertOut):SV_Target
{
	return s_Texture.Sample(sVertOut.cv_uvCoord);

	//return sVertOut.cv_vertexColor;
	//return vecVertColor;
}