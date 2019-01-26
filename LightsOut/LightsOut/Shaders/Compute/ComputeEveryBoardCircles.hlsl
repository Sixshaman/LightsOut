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
		float cellPower  = (float)cellValue / (gDomainSize - 1.0f);

		float2 cellCoord    = DTid.xy - cellNumber * (float2)gCellSize.xx - (float2)gCellSize.xx / 2;
		float  circleRadius = (gCellSize - 1) / 2;

		bool insideCircle = (dot(cellCoord, cellCoord) < circleRadius * circleRadius);

		uint leftPartValue   = (cellNumber.x > 0             ) * CellValue(cellNumber + int2(-1,  0));
		uint rightPartValue  = (cellNumber.x < gBoardSize - 1) * CellValue(cellNumber + int2( 1,  0));
		uint topPartValue    = (cellNumber.y > 0             ) * CellValue(cellNumber + int2( 0, -1));
		uint bottomPartValue = (cellNumber.y < gBoardSize - 1) * CellValue(cellNumber + int2( 0,  1));

		bool circleRuleColored = insideCircle || ((leftPartValue == cellValue && cellCoord.x <= 0) || (topPartValue == cellValue && cellCoord.y <= 0) || (rightPartValue == cellValue && cellCoord.x >= 0) || (bottomPartValue == cellValue && cellCoord.y >= 0));
		
		cellPower = cellPower * circleRuleColored;
		result    = lerp(gColorNone, gColorEnabled, cellPower);

		[flatten]
		if(gFlags & FLAG_SHOW_SOLUTION) //Show the solution
		{
			uint  solutionValue = CellValueSolution(cellNumber);
			float solutionPower = (float)solutionValue / (gDomainSize - 1.0f);

			uint leftPartSolvedValue   = (cellNumber.x > 0             ) * CellValueSolution(cellNumber + int2(-1,  0));
			uint rightPartSolvedValue  = (cellNumber.x < gBoardSize - 1) * CellValueSolution(cellNumber + int2( 1,  0));
			uint topPartSolvedValue    = (cellNumber.y > 0             ) * CellValueSolution(cellNumber + int2( 0, -1));
			uint bottomPartSolvedValue = (cellNumber.y < gBoardSize - 1) * CellValueSolution(cellNumber + int2( 0,  1));

			bool circleRuleSolved = insideCircle || ((leftPartSolvedValue == cellValue && cellCoord.x <= 0) || (topPartSolvedValue == cellValue && cellCoord.y <= 0) || (rightPartSolvedValue == cellValue && cellCoord.x >= 0) || (bottomPartSolvedValue == cellValue && cellCoord.y >= 0));

			solutionPower = solutionPower * circleRuleSolved;
			result        = lerp(result, gColorSolved, solutionPower);
		}
		else if(gFlags & FLAG_SHOW_STABILITY)
		{
			uint  stableValue = CellValueStability(cellNumber);
			float stablePower = (float)stableValue / (gDomainSize - 1.0f);

			float4 colorStable = float4(1.0f, 1.0f, 1.0f, 1.0f) - gColorEnabled;

			uint leftPartStableValue   = (cellNumber.x > 0             ) * CellValueStability(cellNumber + int2(-1,  0));
			uint rightPartStableValue  = (cellNumber.x < gBoardSize - 1) * CellValueStability(cellNumber + int2( 1,  0));
			uint topPartStableValue    = (cellNumber.y > 0             ) * CellValueStability(cellNumber + int2( 0, -1));
			uint bottomPartStableValue = (cellNumber.y < gBoardSize - 1) * CellValueStability(cellNumber + int2( 0,  1));

			bool circleRuleStable = insideCircle || ((leftPartStableValue == cellValue && cellCoord.x <= 0) || (topPartStableValue == cellValue && cellCoord.y <= 0) || (rightPartStableValue == cellValue && cellCoord.x >= 0) || (bottomPartStableValue == cellValue && cellCoord.y >= 0));

			stablePower = stablePower * circleRuleStable;
			result      = lerp(result, colorStable, stablePower);
		}
	}
	else //Edge of the cell
	{
		result = gColorBetween;
	}

	Result[DTid.xy] = result;
}