#define FLAG_SHOW_SOLUTION  0x01
#define FLAG_SHOW_STABILITY 0x02

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

uint CellValue(uint2 cellNumber)
{
	uint cellNumberAll = cellNumber.y * gBoardSize + cellNumber.x; //Number of cell
	return Board[cellNumberAll];
}

uint CellValueSolution(uint2 cellNumber)
{
	uint cellNumberAll = cellNumber.y * gBoardSize + cellNumber.x; //Number of cell
	return Solution[cellNumberAll];
}

uint CellValueStability(uint2 cellNumber)
{
	uint cellNumberAll = cellNumber.y * gBoardSize + cellNumber.x; //Number of cell
	return Stability[cellNumberAll];
}

[numthreads(16, 16, 1)]
void main(uint3 DTid: SV_DispatchThreadID)
{
	float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);

	[flatten]
	if((DTid.x % gCellSize) && (DTid.y % gCellSize)) //Inside the cell
	{
		uint2 cellNumber = DTid.xy / gCellSize.xx;
		uint  cellValue  = CellValue(cellNumber);

		float2 cellCoord     = DTid.xy - cellNumber * (float2)gCellSize.xx - (float2)gCellSize.xx / 2;
		float  diamondRadius = (gCellSize - 1) / 2;

		float domainFactor = 1.0f / (gDomainSize - 1.0f);

		bool insideDiamond     = (abs(cellCoord.x) + abs(cellCoord.y) <= diamondRadius);
		bool insideTopLeft     = !insideDiamond && cellCoord.x <= 0 && cellCoord.y <= 0;
		bool insideTopRight    = !insideDiamond && cellCoord.x >= 0 && cellCoord.y <= 0;
		bool insideBottomRight = !insideDiamond && cellCoord.x >= 0 && cellCoord.y >= 0;
		bool insideBottomLeft  = !insideDiamond && cellCoord.x <= 0 && cellCoord.y >= 0;

		bool4 insideCorner = bool4(insideTopLeft, insideTopRight, insideBottomRight, insideBottomLeft);

		uint leftPartValue        = (cellNumber.x > 0             )                                   * CellValue(cellNumber + int2(-1,  0));
		uint rightPartValue       = (cellNumber.x < gBoardSize - 1)                                   * CellValue(cellNumber + int2( 1,  0));
		uint topPartValue         = (cellNumber.y > 0             )                                   * CellValue(cellNumber + int2( 0, -1));
		uint bottomPartValue      = (cellNumber.y < gBoardSize - 1)                                   * CellValue(cellNumber + int2( 0,  1));
		uint leftTopPartValue     = (cellNumber.x > 0             ) * (cellNumber.y > 0             ) * CellValue(cellNumber + int2(-1, -1));
		uint rightTopPartValue    = (cellNumber.x < gBoardSize - 1) * (cellNumber.y > 0             ) * CellValue(cellNumber + int2( 1, -1));
		uint leftBottomPartValue  = (cellNumber.y < gBoardSize - 1) * (cellNumber.x > 0             ) * CellValue(cellNumber + int2(-1,  1));
		uint rightBottomPartValue = (cellNumber.x < gBoardSize - 1) * (cellNumber.y < gBoardSize - 1) * CellValue(cellNumber + int2( 1,  1));

		uint4 edgeValue   = uint4(leftPartValue, topPartValue, rightPartValue, bottomPartValue);
		uint4 cornerValue = uint4(leftTopPartValue, rightTopPartValue, rightBottomPartValue, leftBottomPartValue);

		uint4 emptyCornerCandidate = (edgeValue.xyzw == edgeValue.yzwx)   * edgeValue;
		uint4 cornerCandidate      = (cellValue.xxxx == cornerValue.xyzw) * cellValue.xxxx;

		uint4 resCorner = max(emptyCornerCandidate, cornerCandidate);

		float  cellPower   = cellValue         * domainFactor;		
		float4 cornerPower = (float4)resCorner * domainFactor;

		float enablePower = cellPower * insideDiamond + dot(cornerPower, insideCorner);
		result            = lerp(gColorNone, gColorEnabled, enablePower);

		[flatten]
		if(gFlags & FLAG_SHOW_SOLUTION) //Show the solution
		{
			uint solutionValue = CellValueSolution(cellNumber);

			uint leftPartSolved        = (cellNumber.x > 0             )                                   * CellValueSolution(cellNumber + int2(-1,  0));
			uint rightPartSolved       = (cellNumber.x < gBoardSize - 1)                                   * CellValueSolution(cellNumber + int2( 1,  0));
			uint topPartSolved         = (cellNumber.y > 0             )                                   * CellValueSolution(cellNumber + int2( 0, -1));
			uint bottomPartSolved      = (cellNumber.y < gBoardSize - 1)                                   * CellValueSolution(cellNumber + int2( 0,  1));
			uint leftTopPartSolved     = (cellNumber.x > 0             ) * (cellNumber.y > 0             ) * CellValueSolution(cellNumber + int2(-1, -1));
			uint rightTopPartSolved    = (cellNumber.x < gBoardSize - 1) * (cellNumber.y > 0             ) * CellValueSolution(cellNumber + int2( 1, -1));
			uint leftBottomPartSolved  = (cellNumber.y < gBoardSize - 1) * (cellNumber.x > 0             ) * CellValueSolution(cellNumber + int2(-1,  1));
			uint rightBottomPartSolved = (cellNumber.x < gBoardSize - 1) * (cellNumber.y < gBoardSize - 1) * CellValueSolution(cellNumber + int2( 1,  1));

			uint4 edgeSolved   = uint4(leftPartSolved,    topPartSolved,      rightPartSolved,       bottomPartSolved);
			uint4 cornerSolved = uint4(leftTopPartSolved, rightTopPartSolved, rightBottomPartSolved, leftBottomPartSolved);

			uint4 emptyCornerSolutionCandidate = (edgeSolved.xyzw    == edgeSolved.yzwx)   * edgeSolved;
			uint4 cornerSolutionCandidate      = (solutionValue.xxxx == cornerSolved.xyzw) * solutionValue.xxxx;

			uint4 resCornerSolved = max(emptyCornerSolutionCandidate, cornerSolutionCandidate);

			float  solutionPower       = solutionValue           * domainFactor;		
			float4 cornerSolutionPower = (float4)resCornerSolved * domainFactor;

			float solvedPower = solutionPower * insideDiamond + dot(cornerSolutionPower, insideCorner);
			result            = lerp(result, gColorSolved, solvedPower);
		}
		else if(gFlags & FLAG_SHOW_STABILITY)
		{
			uint   stableValue = CellValueStability(cellNumber);
			float4 colorStable = float4(1.0f, 1.0f, 1.0f, 1.0f) - gColorEnabled;

			uint leftPartStable        = (cellNumber.x > 0             )                                   * CellValueStability(cellNumber + int2(-1,  0));
			uint rightPartStable       = (cellNumber.x < gBoardSize - 1)                                   * CellValueStability(cellNumber + int2( 1,  0));
			uint topPartStable         = (cellNumber.y > 0             )                                   * CellValueStability(cellNumber + int2( 0, -1));
			uint bottomPartStable      = (cellNumber.y < gBoardSize - 1)                                   * CellValueStability(cellNumber + int2( 0,  1));
			uint leftTopPartStable     = (cellNumber.x > 0             ) * (cellNumber.y > 0             ) * CellValueStability(cellNumber + int2(-1, -1));
			uint rightTopPartStable    = (cellNumber.x < gBoardSize - 1) * (cellNumber.y > 0             ) * CellValueStability(cellNumber + int2( 1, -1));
			uint leftBottomPartStable  = (cellNumber.y < gBoardSize - 1) * (cellNumber.x > 0             ) * CellValueStability(cellNumber + int2(-1,  1));
			uint rightBottomPartStable = (cellNumber.x < gBoardSize - 1) * (cellNumber.y < gBoardSize - 1) * CellValueStability(cellNumber + int2( 1,  1));

			uint4 edgeStable   = uint4(leftPartStable,    topPartStable,      rightPartStable,       bottomPartStable);
			uint4 cornerStable = uint4(leftTopPartStable, rightTopPartStable, rightBottomPartStable, leftBottomPartStable);

			uint4 emptyCornerStabilityCandidate = (edgeStable.xyzw  == edgeStable.yzwx)   * edgeStable;
			uint4 cornerStabilityCandidate      = (stableValue.xxxx == cornerStable.xyzw) * stableValue.xxxx;

			uint4 resCornerStable = max(emptyCornerStabilityCandidate, cornerStabilityCandidate);

			float  stabilityPower       = stableValue             * domainFactor;		
			float4 cornerStabilityPower = (float4)resCornerStable * domainFactor;

			float stablePower = stabilityPower * insideDiamond + dot(cornerStabilityPower, insideCorner);
			result            = lerp(result, colorStable, stablePower);
		}
	}
	else //Edge of the cell
	{
		result = gColorBetween;
	}

	Result[DTid.xy] = result;
}