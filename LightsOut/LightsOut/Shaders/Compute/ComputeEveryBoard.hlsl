#define FLAG_SHOW_SOLUTION  0x01
#define FLAG_SHOW_STABILITY 0x02
#define FLAG_TOROID_RENDER  0x04

cbuffer cbParams: register(b0)
{
	uint gBoardSize;
	uint gCellSize;
	uint gDomainSize;
	uint gFlags;

	float4 gColorNone;
	float4 gColorEnabled;
	float4 gColorSolved;
	float4 gColorBetween;
};

Buffer<uint> Board:     register(t0); //Board (uncompressed)
Buffer<uint> Solution:  register(t1); //Solution(unompressed)
Buffer<uint> Stability: register(t2); //Stability (uncompressed)

RWTexture2D<float4> Result: register(u0); //Drawn Board

[numthreads(16, 16, 1)]
void main(uint3 DTid: SV_DispatchThreadID)
{
	float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);

	[flatten]
	if((DTid.x % gCellSize) && (DTid.y % gCellSize)) //Inside the cell
	{
		uint2 cellNumber    = DTid.xy / gCellSize.xx;
		uint  cellNumberAll = cellNumber.y * gBoardSize + cellNumber.x; //Number of cell

		uint  cellValue = Board[cellNumberAll];
		float cellPower = (float)cellValue / (gDomainSize - 1.0f);

		result = lerp(gColorNone, gColorEnabled, cellPower);

		[flatten]
		if(gFlags & FLAG_SHOW_SOLUTION) //Show the solution
		{
			uint  solutionValue = Solution[cellNumberAll];
			float solutionPower = (float)solutionValue / (gDomainSize - 1.0f);

			result = lerp(result, gColorSolved, solutionPower);
		}
		else if(gFlags & FLAG_SHOW_STABILITY)
		{
			uint  stableValue = Stability[cellNumberAll];
			float stablePower = (float)stableValue / (gDomainSize - 1.0f);

			float4 colorStable = float4(1.0f, 1.0f, 1.0f, 1.0f) - gColorEnabled;
			result = lerp(result, colorStable, stablePower);
		}
	}
	else //Edge of the cell
	{
		result = gColorBetween;
	}

	Result[DTid.xy] = result;
}