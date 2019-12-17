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

		int2 leftCell   = cellNumber + int2(-1,  0);
		int2 rightCell  = cellNumber + int2( 1,  0);
		int2 topCell    = cellNumber + int2( 0, -1);
		int2 bottomCell = cellNumber + int2( 0,  1);

		bool insideCircle = (dot(cellCoord, cellCoord) < circleRadius * circleRadius);

		bool nonLeftEdge   = cellNumber.x > 0;
		bool nonRightEdge  = cellNumber.x < gBoardSize - 1;
		bool nonTopEdge    = cellNumber.y > 0;
		bool nonBottomEdge = cellNumber.y < gBoardSize - 1;

		if(gFlags & FLAG_TOROID_RENDER)
		{
			nonLeftEdge   = true;
			nonRightEdge  = true;
			nonTopEdge    = true;
			nonBottomEdge = true;

			const uint maxCheckDistance = 1; //Different for different render modes

			uint2 leftCellU   = (uint2)(leftCell   + gBoardSize.xx * maxCheckDistance);
			uint2 rightCellU  = (uint2)(rightCell  + gBoardSize.xx * maxCheckDistance);
			uint2 topCellU    = (uint2)(topCell    + gBoardSize.xx * maxCheckDistance);
			uint2 bottomCellU = (uint2)(bottomCell + gBoardSize.xx * maxCheckDistance);

			leftCell   = (int2)(leftCellU   % gBoardSize.xx);
			rightCell  = (int2)(rightCellU  % gBoardSize.xx);
			topCell    = (int2)(topCellU    % gBoardSize.xx);
			bottomCell = (int2)(bottomCellU % gBoardSize.xx);
		}

		uint leftPartValue   = nonLeftEdge   * CellValue(leftCell);
		uint rightPartValue  = nonRightEdge  * CellValue(rightCell);
		uint topPartValue    = nonTopEdge    * CellValue(topCell);
		uint bottomPartValue = nonBottomEdge * CellValue(bottomCell);

		bool circleRuleColored = insideCircle || ((leftPartValue == cellValue && cellCoord.x <= 0) || (topPartValue == cellValue && cellCoord.y <= 0) || (rightPartValue == cellValue && cellCoord.x >= 0) || (bottomPartValue == cellValue && cellCoord.y >= 0));
		
		cellPower = cellPower * circleRuleColored;
		result    = lerp(gColorNone, gColorEnabled, cellPower);

		[flatten]
		if(gFlags & FLAG_SHOW_SOLUTION) //Show the solution
		{
			uint  solutionValue = CellValueSolution(cellNumber);
			float solutionPower = (float)solutionValue / (gDomainSize - 1.0f);

			uint leftPartSolvedValue   = nonLeftEdge   * CellValueSolution(leftCell);
			uint rightPartSolvedValue  = nonRightEdge  * CellValueSolution(rightCell);
			uint topPartSolvedValue    = nonTopEdge    * CellValueSolution(topCell);
			uint bottomPartSolvedValue = nonBottomEdge * CellValueSolution(bottomCell);

			bool circleRuleSolved = insideCircle || ((leftPartSolvedValue == solutionValue && cellCoord.x <= 0) || (topPartSolvedValue == solutionValue && cellCoord.y <= 0) || (rightPartSolvedValue == solutionValue && cellCoord.x >= 0) || (bottomPartSolvedValue == solutionValue && cellCoord.y >= 0));

			solutionPower = solutionPower * circleRuleSolved;
			result        = lerp(result, gColorSolved, solutionPower);
		}
		else if(gFlags & FLAG_SHOW_STABILITY)
		{
			uint  stableValue = CellValueStability(cellNumber);
			float stablePower = (float)stableValue / (gDomainSize - 1.0f);

			float4 colorStable = float4(1.0f, 1.0f, 1.0f, 1.0f) - gColorEnabled;

			uint leftPartStableValue   = nonLeftEdge   * CellValueStability(leftCell);
			uint rightPartStableValue  = nonRightEdge  * CellValueStability(rightCell);
			uint topPartStableValue    = nonTopEdge    * CellValueStability(topCell);
			uint bottomPartStableValue = nonBottomEdge * CellValueStability(bottomCell);

			bool circleRuleStable = insideCircle || ((leftPartStableValue == stableValue && cellCoord.x <= 0) || (topPartStableValue == stableValue && cellCoord.y <= 0) || (rightPartStableValue == stableValue && cellCoord.x >= 0) || (bottomPartStableValue == stableValue && cellCoord.y >= 0));

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