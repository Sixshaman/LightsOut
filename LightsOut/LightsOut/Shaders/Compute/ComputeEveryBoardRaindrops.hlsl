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

		float2 cellCoord    = DTid.xy - cellNumber * (float2)gCellSize.xx - (float2)gCellSize.xx / 2;
		float  circleRadius = (gCellSize - 1) / 2;

		float domainFactor = 1.0f / (gDomainSize - 1.0f);

		bool insideCircle      = (dot(cellCoord, cellCoord) < circleRadius * circleRadius);
		bool insideTopLeft     = !insideCircle && cellCoord.x <= 0 && cellCoord.y <= 0;
		bool insideTopRight    = !insideCircle && cellCoord.x >= 0 && cellCoord.y <= 0;
		bool insideBottomRight = !insideCircle && cellCoord.x >= 0 && cellCoord.y >= 0;
		bool insideBottomLeft  = !insideCircle && cellCoord.x <= 0 && cellCoord.y >= 0;

		bool4 insideCorner = bool4(insideTopLeft, insideTopRight, insideBottomRight, insideBottomLeft);

		int2 leftCell        = cellNumber + int2(-1,  0);
		int2 rightCell       = cellNumber + int2( 1,  0);
		int2 topCell         = cellNumber + int2( 0, -1);
		int2 bottomCell      = cellNumber + int2( 0,  1);
		int2 leftTopCell     = cellNumber + int2(-1, -1);
		int2 rightTopCell    = cellNumber + int2( 1, -1);
		int2 leftBottomCell  = cellNumber + int2(-1,  1);
		int2 rightBottomCell = cellNumber + int2( 1,  1);

		bool nonLeftEdge        = cellNumber.x > 0;
		bool nonRightEdge       = cellNumber.x < gBoardSize - 1;
		bool nonTopEdge         =                                  cellNumber.y > 0;
		bool nonBottomEdge      =                                  cellNumber.y < gBoardSize - 1;
		bool nonLeftTopEdge     = cellNumber.x > 0              && cellNumber.y > 0;
		bool nonRightTopEdge    = cellNumber.x < gBoardSize - 1 && cellNumber.y > 0;
		bool nonLeftBottomEdge  = cellNumber.x > 0              && cellNumber.y < gBoardSize - 1;
		bool nonRightBottomEdge = cellNumber.x < gBoardSize - 1 && cellNumber.y < gBoardSize - 1;

		if(gFlags & FLAG_TOROID_RENDER)
		{
			nonLeftEdge        = true;
			nonRightEdge       = true;
			nonTopEdge         = true;
			nonBottomEdge      = true;
			nonLeftTopEdge     = true;
			nonRightTopEdge    = true;
			nonLeftBottomEdge  = true;
			nonRightBottomEdge = true;

			const uint maxCheckDistance = 1; //Different for different render modes

			uint2 leftCellU        = (uint2)(leftCell        + gBoardSize.xx * maxCheckDistance);
			uint2 rightCellU       = (uint2)(rightCell       + gBoardSize.xx * maxCheckDistance);
			uint2 topCellU         = (uint2)(topCell         + gBoardSize.xx * maxCheckDistance);
			uint2 bottomCellU      = (uint2)(bottomCell      + gBoardSize.xx * maxCheckDistance);
			uint2 leftTopCellU     = (uint2)(leftTopCell     + gBoardSize.xx * maxCheckDistance);
			uint2 rightTopCellU    = (uint2)(rightTopCell    + gBoardSize.xx * maxCheckDistance);
			uint2 leftBottomCellU  = (uint2)(leftBottomCell  + gBoardSize.xx * maxCheckDistance);
			uint2 rightBottomCellU = (uint2)(rightBottomCell + gBoardSize.xx * maxCheckDistance);

			leftCell        = (int2)(leftCellU        % gBoardSize.xx);
			rightCell       = (int2)(rightCellU       % gBoardSize.xx);
			topCell         = (int2)(topCellU         % gBoardSize.xx);
			bottomCell      = (int2)(bottomCellU      % gBoardSize.xx);
			leftTopCell     = (int2)(leftTopCellU     % gBoardSize.xx);
			rightTopCell    = (int2)(rightTopCellU    % gBoardSize.xx);
			leftBottomCell  = (int2)(leftBottomCellU  % gBoardSize.xx);
			rightBottomCell = (int2)(rightBottomCellU % gBoardSize.xx);
		}

		uint leftPartValue        = nonLeftEdge        * CellValue(leftCell);
		uint rightPartValue       = nonRightEdge       * CellValue(rightCell);
		uint topPartValue         = nonTopEdge         * CellValue(topCell);
		uint bottomPartValue      = nonBottomEdge      * CellValue(bottomCell);
		uint leftTopPartValue     = nonLeftTopEdge     * CellValue(leftTopCell);
		uint rightTopPartValue    = nonRightTopEdge    * CellValue(rightTopCell);
		uint leftBottomPartValue  = nonLeftBottomEdge  * CellValue(leftBottomCell);
		uint rightBottomPartValue = nonRightBottomEdge * CellValue(rightBottomCell);

		uint4 edgeValue   = uint4(leftPartValue,    topPartValue,      rightPartValue,       bottomPartValue);
		uint4 cornerValue = uint4(leftTopPartValue, rightTopPartValue, rightBottomPartValue, leftBottomPartValue);

		uint4 emptyCornerCandidate = (edgeValue.xyzw == edgeValue.yzwx                                                                          ) * edgeValue;
		uint4 cornerCandidate      = (cellValue.xxxx == cornerValue.xyzw || cellValue.xxxx == edgeValue.xyzw || cellValue.xxxx == edgeValue.yzwx) * cellValue.xxxx;

		uint4 resCorner = max(emptyCornerCandidate, cornerCandidate);

		float  cellPower   = cellValue         * domainFactor;		
		float4 cornerPower = (float4)resCorner * domainFactor;

		float enablePower = cellPower * insideCircle + dot(cornerPower, insideCorner);
		result            = lerp(gColorNone, gColorEnabled, enablePower);

		[flatten]
		if(gFlags & FLAG_SHOW_SOLUTION) //Show the solution
		{
			uint solutionValue = CellValueSolution(cellNumber);

			uint leftPartSolved        = nonLeftEdge        * CellValueSolution(leftCell);
			uint rightPartSolved       = nonRightEdge       * CellValueSolution(rightCell);
			uint topPartSolved         = nonTopEdge         * CellValueSolution(topCell);
			uint bottomPartSolved      = nonBottomEdge      * CellValueSolution(bottomCell);
			uint leftTopPartSolved     = nonLeftTopEdge     * CellValueSolution(leftTopCell);
			uint rightTopPartSolved    = nonRightTopEdge    * CellValueSolution(rightTopCell);
			uint leftBottomPartSolved  = nonLeftBottomEdge  * CellValueSolution(leftBottomCell);
			uint rightBottomPartSolved = nonRightBottomEdge * CellValueSolution(rightBottomCell);

			uint4 edgeSolved   = uint4(leftPartSolved,    topPartSolved,      rightPartSolved,       bottomPartSolved);
			uint4 cornerSolved = uint4(leftTopPartSolved, rightTopPartSolved, rightBottomPartSolved, leftBottomPartSolved);

			uint4 emptyCornerSolutionCandidate = (edgeSolved.xyzw    == edgeSolved.yzwx                                                                                    ) * edgeSolved;
			uint4 cornerSolutionCandidate      = (solutionValue.xxxx == cornerSolved.xyzw || solutionValue.xxxx == edgeSolved.xyzw || solutionValue.xxxx == edgeSolved.yzwx) * solutionValue.xxxx;

			uint4 resCornerSolved = max(emptyCornerSolutionCandidate, cornerSolutionCandidate);

			float  solutionPower       = solutionValue           * domainFactor;		
			float4 cornerSolutionPower = (float4)resCornerSolved * domainFactor;

			float solvedPower = solutionPower * insideCircle + dot(cornerSolutionPower, insideCorner);
			result            = lerp(result, gColorSolved, solvedPower);
		}
		else if(gFlags & FLAG_SHOW_STABILITY)
		{
			uint   stableValue = CellValueStability(cellNumber);
			float4 colorStable = float4(1.0f, 1.0f, 1.0f, 1.0f) - gColorEnabled;

			uint leftPartStable        = nonLeftEdge        * CellValueStability(leftCell);
			uint rightPartStable       = nonRightEdge       * CellValueStability(rightCell);
			uint topPartStable         = nonTopEdge         * CellValueStability(topCell);
			uint bottomPartStable      = nonBottomEdge      * CellValueStability(bottomCell);
			uint leftTopPartStable     = nonLeftTopEdge     * CellValueStability(leftTopCell);
			uint rightTopPartStable    = nonRightTopEdge    * CellValueStability(rightTopCell);
			uint leftBottomPartStable  = nonLeftBottomEdge  * CellValueStability(leftBottomCell);
			uint rightBottomPartStable = nonRightBottomEdge * CellValueStability(rightBottomCell);

			uint4 edgeStable   = uint4(leftPartStable,    topPartStable,      rightPartStable,       bottomPartStable);
			uint4 cornerStable = uint4(leftTopPartStable, rightTopPartStable, rightBottomPartStable, leftBottomPartStable);

			uint4 emptyCornerStabilityCandidate = (edgeStable.xyzw  == edgeStable.yzwx                                                                                ) * edgeStable;
			uint4 cornerStabilityCandidate      = (stableValue.xxxx == cornerStable.xyzw || stableValue.xxxx == edgeStable.xyzw || stableValue.xxxx == edgeStable.yzwx) * stableValue.xxxx;

			uint4 resCornerStable = max(emptyCornerStabilityCandidate, cornerStabilityCandidate);

			float  stabilityPower       = stableValue             * domainFactor;		
			float4 cornerStabilityPower = (float4)resCornerStable * domainFactor;

			float stablePower = stabilityPower * insideCircle + dot(cornerStabilityPower, insideCorner);
			result            = lerp(result, colorStable, stablePower);
		}
	}
	else //Edge of the cell
	{
		result = gColorBetween;
	}

	Result[DTid.xy] = result;
}