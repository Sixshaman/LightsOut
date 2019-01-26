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

		float2 cellCoord    = DTid.xy - cellNumber * (float2)gCellSize.xx - (float2)gCellSize.xx / 2;
		float  circleRadius = (gCellSize - 1) / 2;

		bool insideCircle      = (dot(cellCoord, cellCoord) < circleRadius * circleRadius);
		bool insideTopLeft     = !insideCircle && cellCoord.x <= 0 && cellCoord.y <= 0;
		bool insideTopRight    = !insideCircle && cellCoord.x >= 0 && cellCoord.y <= 0;
		bool insideBottomRight = !insideCircle && cellCoord.x >= 0 && cellCoord.y >= 0;
		bool insideBottomLeft  = !insideCircle && cellCoord.x <= 0 && cellCoord.y >= 0;

		uint leftPartValue        = (cellNumber.x > 0             )                                   * CellValue(cellNumber + int2(-1,  0));
		uint rightPartValue       = (cellNumber.x < gBoardSize - 1)                                   * CellValue(cellNumber + int2( 1,  0));
		uint topPartValue         = (cellNumber.y > 0             )                                   * CellValue(cellNumber + int2( 0, -1));
		uint bottomPartValue      = (cellNumber.y < gBoardSize - 1)                                   * CellValue(cellNumber + int2( 0,  1));
		uint leftTopPartValue     = (cellNumber.x > 0             ) * (cellNumber.y > 0             ) * CellValue(cellNumber + int2(-1, -1));
		uint rightTopPartValue    = (cellNumber.x < gBoardSize - 1) * (cellNumber.y > 0             ) * CellValue(cellNumber + int2( 1, -1));
		uint leftBottomPartValue  = (cellNumber.y < gBoardSize - 1) * (cellNumber.x > 0             ) * CellValue(cellNumber + int2(-1,  1));
		uint rightBottomPartValue = (cellNumber.x < gBoardSize - 1) * (cellNumber.y < gBoardSize - 1) * CellValue(cellNumber + int2( 1,  1));

		uint LTCandidate = (leftPartValue   == topPartValue   ) * leftPartValue;
		uint RTCandidate = (topPartValue    == rightPartValue ) * topPartValue;
		uint RBCandidate = (rightPartValue  == bottomPartValue) * rightPartValue;
		uint LBCandidate = (bottomPartValue == leftPartValue  ) * bottomPartValue;

		uint LTCCandidate = (cellValue == leftTopPartValue     || cellValue == topPartValue    || cellValue == leftPartValue)   * cellValue;
		uint RTCCandidate = (cellValue == rightTopPartValue    || cellValue == rightPartValue  || cellValue == topPartValue)    * cellValue;
		uint RBCCandidate = (cellValue == rightBottomPartValue || cellValue == bottomPartValue || cellValue == rightPartValue)  * cellValue;
		uint LBCCandidate = (cellValue == leftBottomPartValue  || cellValue == leftPartValue   || cellValue == bottomPartValue) * cellValue;

		uint resLT = max(LTCandidate, LTCCandidate);
		uint resRT = max(RTCandidate, RTCCandidate);
		uint resRB = max(RBCandidate, RBCCandidate);
		uint resLB = max(LBCandidate, LBCCandidate);

		float cellPower = (float)cellValue / (gDomainSize - 1.0f);
		float LTPower   = (float)resLT     / (gDomainSize - 1.0f);
		float RTPower   = (float)resRT     / (gDomainSize - 1.0f);
		float RBPower   = (float)resRB     / (gDomainSize - 1.0f);
		float LBPower   = (float)resLB     / (gDomainSize - 1.0f);

		float enablePower = cellPower * insideCircle + LTPower * insideTopLeft + RTPower * insideTopRight + RBPower * insideBottomRight + LBPower * insideBottomLeft;
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

			uint LTSolutionCandidate = (leftPartSolved   == topPartSolved   ) * leftPartSolved;
			uint RTSolutionCandidate = (topPartSolved    == rightPartSolved ) * topPartSolved;
			uint RBSolutionCandidate = (rightPartSolved  == bottomPartSolved) * rightPartSolved;
			uint LBSolutionCandidate = (bottomPartSolved == leftPartSolved  ) * bottomPartSolved;

			uint LTCSolutionCandidate = (solutionValue == leftTopPartSolved     || solutionValue == topPartSolved    || solutionValue == leftPartSolved)   * solutionValue;
			uint RTCSolutionCandidate = (solutionValue == rightTopPartSolved    || solutionValue == rightPartSolved  || solutionValue == topPartSolved)    * solutionValue;
			uint RBCSolutionCandidate = (solutionValue == rightBottomPartSolved || solutionValue == bottomPartSolved || solutionValue == rightPartSolved)  * solutionValue;
			uint LBCSolutionCandidate = (solutionValue == leftBottomPartSolved  || solutionValue == leftPartSolved   || solutionValue == bottomPartSolved) * solutionValue;

			uint resLTSolved = max(LTSolutionCandidate, LTCSolutionCandidate);
			uint resRTSolved = max(RTSolutionCandidate, RTCSolutionCandidate);
			uint resRBSolved = max(RBSolutionCandidate, RBCSolutionCandidate);
			uint resLBSolved = max(LBSolutionCandidate, LBCSolutionCandidate);

			float solutionPower   = (float)solutionValue / (gDomainSize - 1.0f);
			float LTSolutionPower = (float)resLTSolved   / (gDomainSize - 1.0f);
			float RTSolutionPower = (float)resRTSolved   / (gDomainSize - 1.0f);
			float RBSolutionPower = (float)resRBSolved   / (gDomainSize - 1.0f);
			float LBSolutionPower = (float)resLBSolved   / (gDomainSize - 1.0f);

			float solvedPower = solutionPower * insideCircle + LTSolutionPower * insideTopLeft + RTSolutionPower * insideTopRight + RBSolutionPower * insideBottomRight + LBSolutionPower * insideBottomLeft;
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

			uint LTStabilityCandidate = (leftPartStable   == topPartStable   ) * leftPartStable;
			uint RTStabilityCandidate = (topPartStable    == rightPartStable ) * topPartStable;
			uint RBStabilityCandidate = (rightPartStable  == bottomPartStable) * rightPartStable;
			uint LBStabilityCandidate = (bottomPartStable == leftPartStable  ) * bottomPartStable;

			uint LTCStabilityCandidate = (stableValue == leftTopPartStable     || stableValue == topPartStable    || stableValue == leftPartStable)   * stableValue;
			uint RTCStabilityCandidate = (stableValue == rightTopPartStable    || stableValue == rightPartStable  || stableValue == topPartStable)    * stableValue;
			uint RBCStabilityCandidate = (stableValue == rightBottomPartStable || stableValue == bottomPartStable || stableValue == rightPartStable)  * stableValue;
			uint LBCStabilityCandidate = (stableValue == leftBottomPartStable  || stableValue == leftPartStable   || stableValue == bottomPartStable) * stableValue;

			uint resLTStable = max(LTStabilityCandidate, LTCStabilityCandidate);
			uint resRTStable = max(RTStabilityCandidate, RTCStabilityCandidate);
			uint resRBStable = max(RBStabilityCandidate, RBCStabilityCandidate);
			uint resLBStable = max(LBStabilityCandidate, LBCStabilityCandidate);

			float stabilityPower   = (float)stableValue / (gDomainSize - 1.0f);
			float LTStabilityPower = (float)resLTStable / (gDomainSize - 1.0f);
			float RTStabilityPower = (float)resRTStable / (gDomainSize - 1.0f);
			float RBStabilityPower = (float)resRBStable / (gDomainSize - 1.0f);
			float LBStabilityPower = (float)resLBStable / (gDomainSize - 1.0f);

			float stablePower = stabilityPower * insideCircle + LTStabilityPower * insideTopLeft + RTStabilityPower * insideTopRight + RBStabilityPower * insideBottomRight + LBStabilityPower * insideBottomLeft;
			result            = lerp(result, colorStable, stablePower);
		}
	}
	else //Edge of the cell
	{
		result = gColorBetween;
	}

	Result[DTid.xy] = result;
}