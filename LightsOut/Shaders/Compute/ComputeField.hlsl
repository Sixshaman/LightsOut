cbuffer cbParams: register(b0)
{
	uint gFieldSize;
	uint gCellSize;
	bool gSolveVisible;
	uint gCompressedTurn; //Coordinates of hint

	float4 gColorNone;
	float4 gColorEnabled;
	float4 gColorSolved;
	float4 gColorBetween;
};

Texture1D<uint> Field: register(t0); //Field in compressed UINT-format
Texture1D<uint> Solve: register(t1); //Solution in compressed UINT-format

RWTexture2D<float4> Result: register(u0); //Drawn field

[numthreads(16, 16, 1)]
void main(uint3 DTid: SV_DispatchThreadID)
{
	float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);

	[flatten]
	if((DTid.x % gCellSize) && (DTid.y % gCellSize)) //Inside the cell
	{
		uint2 cellNumber = DTid.xy / gCellSize.xx;

		uint cellNumberAll = cellNumber.y * gFieldSize + cellNumber.x; //Number of cell

		uint compressedCellGroupNumber = cellNumberAll / 32; //Element of Field that contains that cell
		uint compressedCellNumber      = cellNumberAll % 32; //Number of bit of that cell

		bool cellEnabled = (Field[compressedCellGroupNumber] >> compressedCellNumber) & 1; //Getting the bit of cell

		[flatten]
		if(cellEnabled)
		{
			result = gColorEnabled;
		}
		else
		{
			result = gColorNone;
		}

		[flatten]
		if(gSolveVisible) //We are showing the solution
		{
			bool cellSolved = (Solve[compressedCellGroupNumber] >> compressedCellNumber) & 1; //Getting the bit of cell

			[flatten]
			if(cellSolved)
			{
				result = gColorSolved;
			}
		}
		else //We are not showing the solution
		{
			uint hintTurnX = gCompressedTurn >> 16;
			uint hintTurnY = gCompressedTurn & 0xffff;

			[flatten]
			if(cellNumber.x == hintTurnX && cellNumber.x == hintTurnY) //This cell is a hint cell
			{
				result = gColorSolved;
			}
		}
	}
	else //Edge of the cell
	{
		result = gColorBetween;
	}

	Result[DTid.xy] = result;
}